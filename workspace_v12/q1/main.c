#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "drivers/pinout.h"
#include "utils/uartstdio.h"


// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "driverlib/inc/hw_memmap.h"
#include "driverlib/inc/hw_ints.h"

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include <timers.h>
#include <semphr.h>
#include "task.h"
#include "queue.h"
#include "limits.h"

#define UART_PORT 0
#define UART_BAUD 115200
#define UART_CLK SYSTEM_CLOCK

#define FIB_LIMIT_FOR_32_BIT 47
#define TIME_TO_RUN 240 //ms

SemaphoreHandle_t task1SyncSemaphore;
TaskHandle_t loggingTaskHandle;
double FREQ = 100;
uint32_t ulPeriod;


void periphInit();
void timerInit();


void TIM0_Int(void)
{
    TickType_t xCurrTick = xTaskGetTickCount();
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    xTaskNotifyFromISR(loggingTaskHandle, xCurrTick, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Logging Task
void xLoggingTask(void * pvParameters)
{
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 1000 );
    BaseType_t xResult;
    uint32_t ulNotifiedValue;
    while(1){
        xResult = xTaskNotifyWait( pdFALSE, ULONG_MAX, &ulNotifiedValue, xMaxBlockTime );
        if( xResult == pdPASS )
        {
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("Logging Task : %d ms Interrupt Time: %d ms\n", xCurrentTick, ulNotifiedValue);
        }
    }
}

int main(void)
{
    periphInit();
    timerInit();
    vSemaphoreCreateBinary(task1SyncSemaphore);
    xTaskCreate(xLoggingTask, "Logging Task", configMINIMAL_STACK_SIZE, NULL, 2, &loggingTaskHandle);
    vTaskStartScheduler();
    return (0);
}
void timerInit(){
    TimerIntRegister(TIMER0_BASE, TIMER_A, TIM0_Int);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, (SYSTEM_CLOCK / FREQ) -1);
}
void periphInit()
{
    PinoutSet(false, false);
    UARTStdioConfig(UART_PORT, UART_BAUD, UART_CLK);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);   // 32 bits Timer
}
/*  ASSERT() Error function
 *
 *  failed ASSERTS() from driverlib/debug.h are executed in this function
 */
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
