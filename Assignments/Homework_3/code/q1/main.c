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


#define FIB_LIMIT_FOR_32_BIT 47
#define TIME_TO_RUN 240 //ms

SemaphoreHandle_t task1SyncSemaphore;
TaskHandle_t Task1_handle;
double Hz = 100;
uint32_t ulPeriod;



void Timer0Isr(void)
{
    TickType_t xCurrentTick = xTaskGetTickCount();
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  // Clear the timer interrupt

        xTaskNotifyFromISR(Task1_handle, xCurrentTick, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

}



// Process 1
void xTask1(void * pvParameters)
{


    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 5000 );
    BaseType_t xResult;
    uint32_t ulNotifiedValue;

    while(1){

        xResult = xTaskNotifyWait( pdFALSE,
                        /* Don't clear bits on entry. */
                        ULONG_MAX,
                        /* Clear all bits on exit. */
                        &ulNotifiedValue, /* Stores the notified value. */
                        xMaxBlockTime );

        if( xResult == pdPASS )
        {

            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("Task 1 completed at %d ms and Timer interrupt data: %d\n", xCurrentTick, ulNotifiedValue);

        }
    }
}



// Main function
int main(void)
{
    // Initialize system clock to 120 MHz
    uint32_t output_clock_rate_hz;
    output_clock_rate_hz = ROM_SysCtlClockFreqSet(
                               (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                               SYSTEM_CLOCK);
    ASSERT(output_clock_rate_hz == SYSTEM_CLOCK);


    // Initialize the GPIO pins for the Launchpad
    PinoutSet(false, false);
    UARTStdioConfig(0, 230400, SYSTEM_CLOCK);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);   // 32 bits Timer
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0Isr);    // Registering  isr


    ulPeriod = (SYSTEM_CLOCK / Hz);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod -1);

    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    task1SyncSemaphore = xSemaphoreCreateBinary();


    xTaskCreate(xTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, 2, &Task1_handle);

    vTaskStartScheduler();

    return (0);
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

