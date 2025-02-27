
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

#define TIME_TO_RUN 220 //ms

#define UART_PORT 0
#define UART_BAUD 115200
#define UART_CLK SYSTEM_CLOCK
#define TOTAL_TIME 300
#define FREQ 100 // frequency in Hz

long int ul_per;
TickType_t start_tick ;
uint32_t counter = 0;
TaskHandle_t task1_handle,task2_handle;

void load_fib(int ms)
{
    TickType_t start = xTaskGetTickCount();
    TickType_t last_tick = xTaskGetTickCount();
    uint32_t fib = 1, fib_a = 1, fib_b = 1;
    while ((last_tick - start) < pdMS_TO_TICKS(ms))
    {
        fib_a = fib_b;
        fib_b = fib;
        fib = fib_a + fib_b;
        last_tick = xTaskGetTickCount();
    }
}
void TIM0_Int(void)
{
    TickType_t xCurrentTick = xTaskGetTickCount();
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  // Clear the timer interrupt
    counter ++;
    if (counter % 3 == 0){
        xTaskNotifyFromISR(task1_handle, xCurrentTick, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
    else if(counter % 8 == 0){
        xTaskNotifyFromISR(task2_handle, xCurrentTick, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        counter = 0;
    }
}



void task_1(void * pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 5000 );
    BaseType_t xResult;
    uint32_t ulNotifiedValue;

    while((xLastWakeTime - start_tick) < TIME_TO_RUN){
        xResult = xTaskNotifyWait( pdFALSE, ULONG_MAX, &ulNotifiedValue, xMaxBlockTime );
        if( xResult == pdPASS )
        {
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("Task 1 Start Time\t:%d ms\tTime from Timer Interrupt\t:%d ms\n", xCurrentTick, ulNotifiedValue);
            load_fib(10);
            TickType_t xFibTime = xTaskGetTickCount();
            UARTprintf("Task 1 Completion Time\t:%d ms\tExecution Time\t\t\t:%d ms\n", xFibTime, (xFibTime - xCurrentTick));
            xLastWakeTime = xCurrentTick;
        }
    }
}



void task_2(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 5000 );
    BaseType_t xResult;
    uint32_t ulNotifiedValue;

    while ((xLastWakeTime - start_tick) < TIME_TO_RUN)
    {
        xResult = xTaskNotifyWait( pdFALSE, ULONG_MAX, &ulNotifiedValue, xMaxBlockTime );
        if( xResult == pdPASS )
        {
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("Task 2 Start Time\t:%d m\tTime from Timer Interrupt\t:%d ms\n", xCurrentTick, ulNotifiedValue);
            load_fib(40);
            TickType_t xFibTime = xTaskGetTickCount();
            UARTprintf("Task 2 Completion Time\t:%d ms\tExecution Time\t\t\t:%d ms\n", xFibTime, (xFibTime - xCurrentTick));
            xLastWakeTime = xCurrentTick;
        }
    }
}



void periphInit()
{
    PinoutSet(false, false);
    UARTStdioConfig(UART_PORT, UART_BAUD, UART_CLK);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // 32 bits Timer
}
void timerInit(){
    TimerIntRegister(TIMER0_BASE, TIMER_A, TIM0_Int);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, (SYSTEM_CLOCK / FREQ) -1);
}
// Main function
int main(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    periphInit();
    timerInit();
    xTaskCreate(task_1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 2,&task1_handle);
    xTaskCreate(task_2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 1,&task2_handle);
    start_tick = xTaskGetTickCount();
//    xTaskNotifyFromISR(task1_handle, start_tick, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
//    xTaskNotifyFromISR(task2_handle, start_tick, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
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
