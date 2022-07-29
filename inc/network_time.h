#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

struct __attribute__((packed))
ntp_packet
{

  uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                           // li.   Two bits.   Leap indicator.
                           // vn.   Three bits. Version number of the protocol.
                           // mode. Three bits. Client will pick mode 3 for client.

  uint8_t stratum;         // 8 bits. Stratum level of the local clock.
  uint8_t poll;            // 8 bits. Maximum interval between successive messages.
  uint8_t precision;       // 8 bits. Precision of the local clock.

  uint32_t rootDelay;      // 32 bits. Total round trip delay time.
  uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
  uint32_t refId;          // 32 bits. Reference clock identifier.

  uint64_t refTm_s;        // 64 bits. Reference time-stamp seconds. (time when the system clock was set)

  uint64_t origTm_s;       // 64 bits. Originate time-stamp seconds. (time at the client when the request departed for the server)

  uint64_t rxTm_s;         // 64 bits. Received time-stamp seconds. (time at the server when the request arrived)

  uint64_t txTm_s;         // Transmit time-stamp seconds. (Time at the server when the response left for the client or vice versa)

}; // end (48 bytes)

bool get_network_time(struct ntp_packet *packet, char *addr);
void ntp_packet_to_litte_endian(struct ntp_packet *packet);
void ntp_timestamp_to_unix_timestamp(uint64_t *seconds, uint64_t *f_seconds, const uint64_t epoch, const uint64_t ts);
bool get_unix_time_from_ntp(uint64_t *unix_ts, char *addr, uint64_t epoch);
bool get_ctime_from_ntp(struct tm *t, char *addr, uint64_t epoch);
bool check_ntp(const struct ntp_packet *packet);
void get_delay_dispersion(int16_t *number, uint16_t *fract, const uint32_t d);
bool check_delay_dispersion(const uint32_t d);


