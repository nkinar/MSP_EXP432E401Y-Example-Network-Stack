#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "constants.h"
#include "webserver.h"

static char *HTTP_NOT_FOUND = "HTTP/1.1 404 Not Found" CRLF_STR
                              "Connection: Closed\r\n" CRLF_STR;

static void prvServerConnectionInstanceWeb(void *pvParameters);
void vCreateTCPServerSocketWeb(  void *pvParameters  );


// Process connections for ident
void prvServerConnectionInstanceWeb(void *pvParameters)
{
    char cRxedData[MAX_WEB_BYTES];
    memcpy( cRxedData, 0, MAX_WEB_BYTES);
    Socket_t xSocket;
    BaseType_t xBytesReceived;

    xSocket = ( Socket_t ) pvParameters;
    for(;;)
    {
        xBytesReceived = FreeRTOS_recv( xSocket, &cRxedData, MAX_WEB_BYTES-1, 0 );
        if(xBytesReceived > 0)
        {
            // For now, this indicates to all clients that a webpage was not found.
            // This prevents web browsers from aggressively checking if the port is open.
            // TODO: Add in a simple web server.
            cRxedData[xBytesReceived-1] = 0;
            FreeRTOS_send(xSocket, HTTP_NOT_FOUND, strlen(HTTP_NOT_FOUND), 0);  // http not found
            break;  // since the connection is closed
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
void vCreateTCPServerSocketWeb(  void *pvParameters  )
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

    xBindAddress.sin_port = ( uint16_t ) WEB_PORT;  // web socket port bind
    xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
    FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );
    FreeRTOS_listen( xListeningSocket, xBacklog );

    for( ;; )
    {
        xConnectedSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize );
        if ((xConnectedSocket != FREERTOS_INVALID_SOCKET) && (xConnectedSocket != NULL))
        {
            xTaskCreate( prvServerConnectionInstanceWeb,
                                "WebSoc",
                                THREAD_STACK_SIZE_WEB,
                                ( void * ) xConnectedSocket,
                                tskIDLE_PRIORITY,
                                NULL );
        }
    } // end for
} // end task



// Setup task to deal with the web server
void setup_web()
{
    xTaskCreate( vCreateTCPServerSocketWeb,
                               "WebSrv",
                               configMINIMAL_STACK_SIZE,
                               NULL,
                               tskIDLE_PRIORITY,
                               NULL );
} // end


