
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
#define ITERATION 12000
#define MULTIPLIER 100
#define Hz (30 * MULTIPLIER)        // Hz
#define SEQUENCER_COUNT (900 * MULTIPLIER)
#define UART_BAUD_RATE 1000000

SemaphoreHandle_t task_1_SyncSemaphore, task_2_SyncSemaphore, task_3_SyncSemaphore, task_4_SyncSemaphore, task_5_SyncSemaphore, task_6_SyncSemaphore, task_7_SyncSemaphore;
TickType_t startTimeTick;
TaskHandle_t Task1_handle, Task2_handle, Task3_handle, Task4_handle, Task5_handle, Task6_handle, Task7_handle;
volatile uint32_t counter_isr = 0;
uint32_t ulPeriod;
volatile bool abort_test = false;
uint32_t wcet[7];
uint32_t execution_time[7];
uint32_t execution_cycle[7];

void init_Timer();
void init_Uart();
void init_Clock();

void fibonacci()
{
    uint32_t i,j;
    uint32_t fib = 1, fib_a = 1, fib_b = 1;
    for ( i=0; i<ITERATION; i++)
    {
        for(j=0; j<FIB_LIMIT_FOR_32_BIT; j++){
            fib_a = fib_b;
            fib_b = fib;
            fib = fib_a + fib_b;
        }

    }
}


void print_data(){
    uint32_t i = 0;
    for (i = 0; i < 7; i++){
        UARTprintf("***** Task %d wcet %d total_exectution_time %d execution unit %d *****\n\r", i+1, wcet[i], execution_time[i], execution_cycle[i]);
    }
}

void Timer0Isr_Sequencer(void)
{
    TickType_t xCurrentTick = xTaskGetTickCount();
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt
    counter_isr++;

//    UARTprintf("Sequencer Thread ran at %d ms and Cycle of sequencer %d \n\r", xCurrentTick, counter_isr);

    if ((counter_isr % 10) == 0)
    {
        // Service_1 = RT_MAX-1 @ 300 Hz
        xSemaphoreGive(task_1_SyncSemaphore); // Frame Sampler thread
    }

    if ((counter_isr % 30) == 0)
    {

        // Service_2 = RT_MAX-2 @ 100 Hz
        xSemaphoreGive(task_2_SyncSemaphore); // Time-stamp with Image Analysis thread
        // Service_4 = RT_MAX-2 @ 100 Hz
        xSemaphoreGive(task_4_SyncSemaphore); // Time-stamp Image Save to File thread
        // Service_6 = RT_MAX-2 @ 100 Hz
        xSemaphoreGive(task_6_SyncSemaphore); // Send Time-stamped Image to Remote thread

    }

    if ((counter_isr % 60) == 0)
    {
        // Service_3 = RT_MAX-3 @ 50 Hz
        xSemaphoreGive(task_3_SyncSemaphore); // Difference Image Proc thread
        // Service_5 = RT_MAX-3 @ 50 Hz
        xSemaphoreGive(task_5_SyncSemaphore); // Processed Image Save to File thread
    }


    if ((counter_isr % 300) == 0)
    {
        // Service_7 = RT_MIN   10 Hz
        xSemaphoreGive(task_7_SyncSemaphore); // 10 sec Tick Debug thread
    }

    if (counter_isr > SEQUENCER_COUNT)
    {
        xSemaphoreGive(task_1_SyncSemaphore); // Frame Sampler thread
        xSemaphoreGive(task_2_SyncSemaphore); // Time-stamp with Image Analysis thread
        xSemaphoreGive(task_3_SyncSemaphore); // Difference Image Proc thread
        xSemaphoreGive(task_4_SyncSemaphore); // Time-stamp Image Save to File thread
        xSemaphoreGive(task_5_SyncSemaphore); // Processed Image Save to File thread
        xSemaphoreGive(task_6_SyncSemaphore); // Send Time-stamped Image to Remote thread
        xSemaphoreGive(task_7_SyncSemaphore); // 10 sec Tick Debug thread
        abort_test = true;
        ROM_TimerDisable(TIMER0_BASE, TIMER_A);
        print_data();
    }
}

// Process 1
void xTask1(void *pvParameters)
{
    BaseType_t xResult;

    while (!abort_test)
    {

        xResult = xSemaphoreTake(task_1_SyncSemaphore, portMAX_DELAY);

        if (xResult == pdPASS)
        {
            execution_cycle[0]++;
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("T1 S:%d, R %d\n\r", xCurrentTick, execution_cycle[0]);
            fibonacci();
            TickType_t xFibTime = xTaskGetTickCount();
            TickType_t total_time = (xFibTime - xCurrentTick);
            execution_time[0] += total_time;
            if(wcet[0] < total_time) wcet[0] = total_time;

            UARTprintf("T1 C:%d, E:%d\n\r", xFibTime, total_time);
        }
    }
    vTaskDelete( NULL );
}

void xTask2(void *pvParameters)
{
    BaseType_t xResult;

    while (!abort_test)
    {
        xResult = xSemaphoreTake(task_2_SyncSemaphore, portMAX_DELAY);


        if (xResult == pdPASS)
        {
            execution_cycle[1]++;
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("T2 S:%d, R %d\n\r", xCurrentTick, execution_cycle[1]);
            fibonacci();
            TickType_t xFibTime = xTaskGetTickCount();
            TickType_t total_time = (xFibTime - xCurrentTick);
            execution_time[1] += total_time;
            if(wcet[1] < total_time) wcet[1] = total_time;
            UARTprintf("T2 C:%d, E:%d\n\r", xFibTime, (xFibTime - xCurrentTick));
        }
    }
    vTaskSuspend( NULL );
}
void xTask3(void *pvParameters)
{
    BaseType_t xResult;;

    while (!abort_test)
    {

        xResult = xSemaphoreTake(task_3_SyncSemaphore, portMAX_DELAY);

        if (xResult == pdPASS)
        {
            execution_cycle[2]++;
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("T3 S:%d, R %d\n\r", xCurrentTick, execution_cycle[2]);
            fibonacci();
            TickType_t xFibTime = xTaskGetTickCount();
            TickType_t total_time = (xFibTime - xCurrentTick);
            execution_time[2] += total_time;
            if(wcet[2] < total_time) wcet[2] = total_time;
            UARTprintf("T3 C:%d, E:%d\n\r", xFibTime, (xFibTime - xCurrentTick));
        }
    }
    vTaskDelete( NULL );
}
void xTask4(void *pvParameters)
{
    BaseType_t xResult;

    while (!abort_test)
    {

        xResult = xSemaphoreTake(task_4_SyncSemaphore, portMAX_DELAY);

        if (xResult == pdPASS)
        {
            execution_cycle[3]++;
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("T4 S:%d, R %d\n\r", xCurrentTick, execution_cycle[3]);
            fibonacci();
            TickType_t xFibTime = xTaskGetTickCount();
            TickType_t total_time = (xFibTime - xCurrentTick);
            execution_time[3] += total_time;
            if(wcet[3] < total_time) wcet[3] = total_time;
            UARTprintf("T4 C:%d, E:%d\n\r", xFibTime, (xFibTime - xCurrentTick));
        }
    }
    vTaskDelete( NULL );
}
void xTask5(void *pvParameters)
{
    BaseType_t xResult;

    while (!abort_test)
    {

        xResult = xSemaphoreTake(task_5_SyncSemaphore, portMAX_DELAY);

        if (xResult == pdPASS)
        {
            execution_cycle[4]++;
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("T5 S:%d, R %d\n\r", xCurrentTick, execution_cycle[4]);
            fibonacci();
            TickType_t xFibTime = xTaskGetTickCount();
            TickType_t total_time = (xFibTime - xCurrentTick);
            execution_time[4] += total_time;
            if(wcet[4] < total_time) wcet[4] = total_time;
            UARTprintf("T5 C:%d, E:%d\n\r", xFibTime, (xFibTime - xCurrentTick));
        }
    }
    vTaskDelete( NULL );
}
void xTask6(void *pvParameters)
{
    BaseType_t xResult;

    while (!abort_test)
    {

        xResult = xSemaphoreTake(task_6_SyncSemaphore, portMAX_DELAY);

        if (xResult == pdPASS)
        {
            execution_cycle[5]++;
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("T6 S:%d, R %d\n\r", xCurrentTick, execution_cycle[5]);
            fibonacci();
            TickType_t xFibTime = xTaskGetTickCount();
            TickType_t total_time = (xFibTime - xCurrentTick);
            execution_time[5] += total_time;
            if(wcet[5] < total_time) wcet[5] = total_time;
            UARTprintf("T6 C:%d, E:%d\n\r", xFibTime, (xFibTime - xCurrentTick));
        }
    }
    vTaskDelete( NULL );
}
void xTask7(void *pvParameters)
{
    BaseType_t xResult;

    while (!abort_test)
    {

        xResult = xSemaphoreTake(task_7_SyncSemaphore, portMAX_DELAY);

        if (xResult == pdPASS)
        {
            execution_cycle[6]++;
            TickType_t xCurrentTick = xTaskGetTickCount();
            UARTprintf("T7 S:%d , R %d\n\r", xCurrentTick, execution_cycle[6]);
            fibonacci();
            TickType_t xFibTime = xTaskGetTickCount();
            TickType_t total_time = (xFibTime - xCurrentTick);
            execution_time[6] += total_time;
            if(wcet[6] < total_time) wcet[6] = total_time;
            UARTprintf("T7 C:%d, E:%d\n\r", xFibTime, (xFibTime - xCurrentTick));
        }
    }
    vTaskDelete( NULL );
}

// Main function
int main(void)
{
    init_Clock();
    init_Uart();
    init_Timer();

    task_1_SyncSemaphore = xSemaphoreCreateBinary();
    task_2_SyncSemaphore = xSemaphoreCreateBinary();
    task_3_SyncSemaphore = xSemaphoreCreateBinary();
    task_4_SyncSemaphore = xSemaphoreCreateBinary();
    task_5_SyncSemaphore = xSemaphoreCreateBinary();
    task_6_SyncSemaphore = xSemaphoreCreateBinary();
    task_7_SyncSemaphore = xSemaphoreCreateBinary();

    UARTprintf("Cyclic executer : %d Hz\n\r", Hz);
    xTaskCreate(xTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, 4, &Task1_handle);
    xTaskCreate(xTask2, "Task2", configMINIMAL_STACK_SIZE, NULL, 3, &Task2_handle);
    xTaskCreate(xTask3, "Task3", configMINIMAL_STACK_SIZE, NULL, 2, &Task3_handle);
    xTaskCreate(xTask4, "Task4", configMINIMAL_STACK_SIZE, NULL, 3, &Task4_handle);
    xTaskCreate(xTask5, "Task5", configMINIMAL_STACK_SIZE, NULL, 2, &Task5_handle);
    xTaskCreate(xTask6, "Task6", configMINIMAL_STACK_SIZE, NULL, 3, &Task6_handle);
    xTaskCreate(xTask7, "Task7", configMINIMAL_STACK_SIZE, NULL, 1, &Task7_handle);

    startTimeTick = xTaskGetTickCount();

    vTaskStartScheduler();
    UARTprintf("\nTEST COMPLETE\n");
    return (0);
}

void init_Timer()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);         // 32 bits Timer
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0Isr_Sequencer); // Registering  isr

    ulPeriod = (SYSTEM_CLOCK / Hz);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod - 1);

    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void init_Clock()
{
    // Initialize system clock to 120 MHz
    uint32_t output_clock_rate_hz;
    output_clock_rate_hz = ROM_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), SYSTEM_CLOCK);
    ASSERT(output_clock_rate_hz == SYSTEM_CLOCK);
}

void init_Uart()
{
    // Initialize the GPIO pins for the Launchpad
    PinoutSet(false, false);
    UARTStdioConfig(0, UART_BAUD_RATE, SYSTEM_CLOCK);
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
