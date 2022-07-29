#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "ident.h"
#include "string_helper.h"
#include "constants.h"

// Static functions
static void prvServerConnectionInstanceIdent(void *pvParameters);
static void vCreateTCPServerSocketIdent(void *pvParameters  );

// Holds the name of the ident string
static char ident_str[MAX_DEVICE_NAME_SIZ];

// Process connections for ident
static void prvServerConnectionInstanceIdent(void *pvParameters)
{
    char cRxedData[MAX_IDENT_BYTES];
    memcpy( cRxedData, 0, MAX_IDENT_BYTES);
    Socket_t xSocket;
    BaseType_t xBytesReceived, xBytesSent;

    xSocket = ( Socket_t ) pvParameters;
    for(;;)
    {
        xBytesReceived = FreeRTOS_recv( xSocket, &cRxedData, MAX_IDENT_BYTES-1, 0 );
        if(xBytesReceived > 0)
        {
            cRxedData[xBytesReceived-1] = 0;
            if(string_comp(trimwhitespace(cRxedData), IDENT_STR))
            {
                xBytesSent = FreeRTOS_send(xSocket, ident_str, strlen(ident_str), 0);
                if(xBytesSent < 0) break;
            }
        }
        else
        {
            break;
        }
    }
    FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );
    FreeRTOS_closesocket( xSocket );
    vTaskDelete( NULL );
} // end

// Create a socket used for identification on the network
void vCreateTCPServerSocketIdent(  void *pvParameters  )
{
    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    TickType_t xReceiveTimeOut = pdMS_TO_TICKS(PORT_RX_TIMEOUT);
    TickType_t xSendTimeOut = pdMS_TO_TICKS(PORT_TX_TIMEOUT);
    BaseType_t xBacklog = PORT_BACKLOG;

    xListeningSocket = FreeRTOS_socket(  FREERTOS_AF_INET,
                                            FREERTOS_SOCK_STREAM,  // TCP
                                            FREERTOS_IPPROTO_TCP );
    FreeRTOS_setsockopt( xListeningSocket,
                            0,
                            FREERTOS_SO_RCVTIMEO,
                            &xReceiveTimeOut,
                            sizeof( xReceiveTimeOut ) );
    FreeRTOS_setsockopt( xListeningSocket,
                            0,
                            FREERTOS_SO_SNDTIMEO,
                            &xSendTimeOut,
                            sizeof( xSendTimeOut ) );

    xBindAddress.sin_port = ( uint16_t ) IDENT_PORT;
    xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
    FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );
    FreeRTOS_listen( xListeningSocket, xBacklog );

    for( ;; )
    {
        xConnectedSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize );
        if ((xConnectedSocket != FREERTOS_INVALID_SOCKET) && (xConnectedSocket != NULL))
        {
            xTaskCreate( prvServerConnectionInstanceIdent,
                                "IdentSoc",
                                configMINIMAL_STACK_SIZE,
                                ( void * ) xConnectedSocket,
                                tskIDLE_PRIORITY,
                                NULL );
        }
    } // end for
} // end task


// Setup ident task to start the ident server
void setup_ident()
{
    memcpy(ident_str, 0, MAX_DEVICE_NAME_SIZ);
    xTaskCreate( vCreateTCPServerSocketIdent,
                               "IdentSrv",
                               configMINIMAL_STACK_SIZE,
                               NULL,
                               tskIDLE_PRIORITY,
                               NULL );
} // end


// Call this function to set the identification name sent back to the system
void set_ident_name(char *name)
{
    snprintf(ident_str, MAX_DEVICE_NAME_SIZ-1, "%s"CRLF_STR, name);
} // end







