#pragma once

void *mainThreadBlink(void *arg0);
void vTaskBlink( void *pvParameters );
void vTaskPrintDebug( void *pvParameters );
void create_tasks_check_notify();
void prvTask1( void *pvParameters );
void prvTask2( void *pvParameters );

