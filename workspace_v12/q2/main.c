#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "drivers/pinout.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "driverlib/inc/hw_memmap.h"
#include "driverlib/inc/hw_ints.h"
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
#define TOTAL_TIME 300
#define FREQ 1000; // frequency in Hz

long int ul_per;
SemaphoreHandle_t task1_sem, task2_sem;
TickType_t start_tick ;

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

// Task 1
void task_1(void *pvParameters)
{
    TickType_t last_tick;
    last_tick = xTaskGetTickCount();

    while ((last_tick - start_tick) < TOTAL_TIME)
    {
        if (xSemaphoreTake(task1_sem, portMAX_DELAY) == pdTRUE)
        {
            TickType_t current = xTaskGetTickCount();
            load_fib(10);
            TickType_t execution_time = xTaskGetTickCount();
            UARTprintf("Task 1 Start Time: %d Fibonacci Execution Time:  %d \n", current, (execution_time - current));
            last_tick = current;
            xSemaphoreGive(task2_sem);
        }
    }
}

// Task 2
void task_2(void *pvParameters)
{
    TickType_t last_tick;
    last_tick = xTaskGetTickCount();

    while ((last_tick - start_tick) < TOTAL_TIME)
    {
        if (xSemaphoreTake(task2_sem, portMAX_DELAY) == pdTRUE)
        {
            TickType_t current = xTaskGetTickCount();
            load_fib(40);
            TickType_t execution_time = xTaskGetTickCount();
            UARTprintf("Task 2 Start Time: %d Fibonacci Execution Time:  %d \n", current, (execution_time - current));
            last_tick = current;
            xSemaphoreGive(task1_sem);
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

// Main function
int main(void)
{
    periphInit();
    vSemaphoreCreateBinary(task1_sem);
    vSemaphoreCreateBinary(task2_sem);
    xTaskCreate(task_1, "task 1", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(task_2, "task 2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xSemaphoreGive(task1_sem);
    start_tick = xTaskGetTickCount();

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
