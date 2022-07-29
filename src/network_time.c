#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>

#include "FreeRTOS.h"
#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "network_time.h"
#include "helper_math.h"
#include "timeconvert.h"
#include "constants.h"

/*
NTP Client
REFERENCE:
[1] https://lettier.github.io/posts/2016-04-26-lets-make-a-ntp-client-in-c.html
[2] https://tools.ietf.org/rfc/rfc5905.txt
[3] http://thompsonng.blogspot.com/2010/04/ntp-timestamp_21.html
[4] https://github.com/sidsingh78/EPOCH-to-time-date-converter/blob/master/epoch_conv.c

List of public time servers:
[5] https://gist.github.com/mutin-sa/eea1c396b1e610a2da1e5550d94b0453

The autokey protocol is not used since it is not widely supported.
Some checks are provided in the code when the NTP packet is returned.

Standards:
https://tools.ietf.org/html/rfc5905
https://tools.ietf.org/html/rfc4330
*/

// Seconds between 1 January 1900 and 1 January 1970
// The NTP epoch starts on 1 January 1900
// The UNIX epoch starts on 1 January 1970
const uint64_t NTP_TIMESTAMP_DELTA = 2208988800;

// max amount of time for server to be out of sync
const uint32_t MAX_SEC_SYNC = 1;

// Function to obtain the C time from the NTP server
bool get_ctime_from_ntp(struct tm *t, char *addr, uint64_t epoch)
{
    uint64_t unix_ts;
    bool rv = get_unix_time_from_ntp(&unix_ts, addr, epoch);
    if(!rv) return false;

    epoch_to_time(t, unix_ts);
    return true;
} // end


void get_delay_dispersion(int16_t *number, uint16_t *fract, const uint32_t d)
{
    uint16_t high = (d >> 16) & 0xFFFF;
    uint16_t low =  (uint16_t)(d & 0xFFFF);

    *number = (int16_t)high;
    *fract = low;
} // end


bool check_delay_dispersion(const uint32_t d)
{
    int16_t number;
    uint16_t fract;
    get_delay_dispersion(&number, &fract, d);
    if(number < 0) return false;
    if(number >= MAX_SEC_SYNC) return false;
    return true;
} // end


// Function to check the NTP
bool check_ntp(const struct ntp_packet *packet)
{
    uint8_t li_vn_mode = packet->li_vn_mode;
    uint8_t li = (li_vn_mode >> 6) & 0x03;
    uint8_t vn = (li_vn_mode >> 3) & 0x07;
    uint8_t mode = li_vn_mode & 0x07;

    if(li > 3) return false;    // li 0-3 as leap second indicator
    if(vn != 4) return false;   // vn = 4 as NTPv4
    if(mode != 4) return false; // mode = 4 as server
    if((packet->stratum) == 0) return false;  // kiss-o'-death message
    if((packet->stratum) > 15) return false;  // reserved number?

    uint32_t rootDelay = FreeRTOS_htonl(packet->rootDelay);
    uint32_t rootDispersion = FreeRTOS_htonl(packet->rootDispersion);

    bool rv = check_delay_dispersion(rootDelay);
    if(rv == false) return false;

    rv = check_delay_dispersion(rootDispersion);
    if(rv == false) return false;

    // TODO: check the timestamps on the packet that is returned

    // everything passes!
    return true;
} // end


// Function to obtain the UNIX timestamp from the NTP server
// unix_ts  = UNIX timestamp in seconds (time since 1 January 1970)
// addr     = IP address of network time server
// era      = the NTP epoch (currently 0, but will roll over to 1 on February 7, 2036)
bool get_unix_time_from_ntp(uint64_t *unix_ts, char *addr, uint64_t epoch)
{
    *unix_ts = 0;
    struct ntp_packet packet;
    bool rv = get_network_time(&packet, addr);
    if(rv == false) return false;

    rv = check_ntp(&packet);
    if(rv == false) return false;

    uint64_t ts = ntohll(packet.txTm_s);
    uint64_t seconds, f_seconds;
    ntp_timestamp_to_unix_timestamp(&seconds, &f_seconds, epoch, ts);

    // TODO: calculate the offset and then use this offset to correct the time in seconds if required.
    // Return a false value if the offset is too large.

    // return the seconds since 1 Jan 1970
    *unix_ts = seconds;

    return true;
} // end


// Convert the timestamp into fractional parts.  Note that we assume here that the timestamp is in little endian format
// and has been converted from the big endian network format.
// seconds      = seconds since 1 January 1970
// f_seconds    = fractional seconds
// epoch        = the NTP epoch
// ts           = the timestamp from the NTP packet
void ntp_timestamp_to_unix_timestamp(uint64_t *seconds, uint64_t *f_seconds, const uint64_t epoch, const uint64_t ts)
{
    const uint64_t nump32 = ((uint64_t)1<<32);
    uint64_t low = ts & 0xFFFFFFFF;
    uint64_t high = (ts >> 32) & 0xFFFFFFFF;
    *seconds = (epoch * nump32) + high - NTP_TIMESTAMP_DELTA;
    *f_seconds = low;
} // end0


// Convert the NTP packet to little endian
void ntp_packet_to_litte_endian(struct ntp_packet *packet)
{
    packet->rootDelay = ntohll( packet->rootDelay);
    packet->rootDispersion = ntohll(packet->rootDispersion);
    packet->refId = ntohll(packet->refId);

    packet->refTm_s = ntohll(packet->refTm_s);
    packet->origTm_s = ntohll(packet->origTm_s);
    packet->rxTm_s = ntohll(packet->rxTm_s);
    packet->txTm_s = ntohll(packet->txTm_s);
} // end


// Obtains the network time by opening a socket and obtaining the network time from a server
bool get_network_time(struct ntp_packet *packet, char *addr)
{
    BaseType_t xHow = 0;
    int32_t rv = 0;
    const TickType_t xSendTimeOut = pdMS_TO_TICKS(2500);
    const TickType_t xReceiveTimeOut = pdMS_TO_TICKS(2500);
    const TickType_t xWaitARP = pdMS_TO_TICKS(10);

    memset(packet, 0, sizeof(packet));
    uint8_t mode = 0xe3; // vn = 4 (version 4), mode = 3 (client)
    packet->li_vn_mode = mode;

    struct freertos_sockaddr xDestinationAddress;
    struct freertos_sockaddr xSourceAddress;
    socklen_t xAddressLength = 0;

    xDestinationAddress.sin_addr = FreeRTOS_gethostbyname(addr);
    xDestinationAddress.sin_port = FreeRTOS_htons(  NTP_PORT  );

    Socket_t xSocket;
    xSocket = FreeRTOS_socket( FREERTOS_AF_INET,
                                FREERTOS_SOCK_DGRAM,
                                FREERTOS_IPPROTO_UDP );
    if (xSocket == FREERTOS_INVALID_SOCKET ) return false;

    FreeRTOS_setsockopt( xSocket,
                         0,
                         FREERTOS_SO_SNDTIMEO,
                         &xSendTimeOut,
                         sizeof( xSendTimeOut ) );
    FreeRTOS_setsockopt( xSocket,
                         0,
                         FREERTOS_SO_RCVTIMEO,
                         &xReceiveTimeOut,
                         sizeof( xReceiveTimeOut ) );

    struct freertos_sockaddr xBindAddress;
    xBindAddress.sin_port = FreeRTOS_htons(NTP_PORT_CLIENT);
    rv = FreeRTOS_bind(  xSocket, &xBindAddress, sizeof( xBindAddress ) );
    if(rv < 0)
    {
        goto cleanup;
    }

    // Try to ensure that the first UDP packet does not get turned into an ARP request.
    // If it does, the packet send will fail, so an ARP request is required.
    FreeRTOS_OutputARPRequest( xDestinationAddress.sin_addr);
    vTaskDelay(pdMS_TO_TICKS(xWaitARP));

    // TODO: Obtain the current system clock time and add it to the packet.

    // send the packet to the time server
    rv = FreeRTOS_sendto( xSocket,
                            packet,
                            sizeof(struct ntp_packet),
                            0,
                            &xDestinationAddress,
                            sizeof( xDestinationAddress ) );
    if(rv <= 0) goto cleanup;


    // receive the response from the time server
    rv = FreeRTOS_recvfrom( xSocket,
                                packet,
                                sizeof(struct ntp_packet),
                                0,
                                &xSourceAddress,
                                &xAddressLength);

cleanup:
    FreeRTOS_shutdown( xSocket, xHow);
    FreeRTOS_closesocket( xSocket);

    if(rv <= 0) return false;
    // check the source and destination ports and addresses
    if(xSourceAddress.sin_addr != xDestinationAddress.sin_addr) return false;
    if(xSourceAddress.sin_port != xDestinationAddress.sin_port) return false;

    // TODO: check that the current clock time is sent back to the server by the client

    // success
    return true;
} // end









