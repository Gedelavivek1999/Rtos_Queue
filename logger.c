#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#include "fsl_lpuart.h"      //nxp drivers
#include "fsl_clock.h"

#define BOARD_LOGGER_LPUART       LPUART0
#define BOARD_LOGGER_UART_CLK_FREQ CLOCK_GetLpuartClkFreq(0)

static QueueHandle_t xLogQueue = NULL;
static TaskHandle_t xLoggerTaskHandle = NULL;


static void logger_hw_init(void) {
    lpuart_config_t config;
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200U;
    config.enableTx     = true;
    config.enableRx     = false;

    LPUART_Init(BOARD_LOGGER_LPUART, &config, BOARD_LOGGER_UART_CLK_FREQ);
}

// string tx
static void logger_hw_write(const char *str) {
    while (*str) {
        LPUART_WriteBlocking(BOARD_LOGGER_LPUART, (const uint8_t *)str, 1);
        str++;
    }
}

/* Consumer Task: Handles I/O hardware output asynchronously */
static void prvLoggerTask(void *pvParameters) {
    (void)pvParameters;
    log_entry_t entry;

    const char *level_str[] = {"DBG", "INF", "WRN", "ERR"};

    for (;;) {
        /* Block indefinitely until a log entry arrives */
        if (xQueueReceive(xLogQueue, &entry, portMAX_DELAY) == pdTRUE) {
            char formatted_buf[LOG_MAX_MESSAGE_LEN + 64];
            
            snprintf(formatted_buf, sizeof(formatted_buf),
                     "[%08lu][Task:%d][%s]: %s\r\n",
                     (unsigned long)entry.timestamp_ms,
                     entry.task_id,
                     level_str[entry.level],
                     entry.message);

            logger_hw_write(formatted_buf);
        }
    }
}

// queue init
bool logger_init(void) {
    logger_hw_init();

    xLogQueue = xQueueCreate(LOG_QUEUE_DEPTH, sizeof(log_entry_t));
    if (xLogQueue == NULL) {
        return false;
    }

    BaseType_t xStatus = xTaskCreate(prvLoggerTask,
                                     "LoggerTask",
                                     LOGGER_TASK_STACK,
                                     NULL,
                                     LOGGER_TASK_PRIO,
                                     &xLoggerTaskHandle);

    return (xStatus == pdPASS);
}

// task log
void log_msg(log_level_t level, const char *fmt, ...) {
    if (xLogQueue == NULL) return;

    log_entry_t entry;
    entry.timestamp_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    entry.level = level;
    entry.task_id = (uint8_t)(uintptr_t)xTaskGetCurrentTaskHandle();

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, sizeof(entry.message), fmt, args);
    va_end(args);

    
    (void)xQueueSendToBack(xLogQueue, &entry, pdMS_TO_TICKS(2)); //2ms
}

//ISR routine
void log_msg_from_isr(log_level_t level, const char *fmt, ...) {
    if (xLogQueue == NULL) return;

    log_entry_t entry;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    entry.timestamp_ms = (uint32_t)(xTaskGetTickCountFromISR() * portTICK_PERIOD_MS);
    entry.level = level;
    entry.task_id = 0xFF;  // adress for isr routine one 

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, sizeof(entry.message), fmt, args);
    va_end(args);

    xQueueSendToBackFromISR(xLogQueue, &entry, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}