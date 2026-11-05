#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"   //rtos librares
#include "queue.h"
#include "task.h"

#define LOG_MAX_MESSAGE_LEN   128         //limit for msg len
#define LOG_QUEUE_DEPTH       32          //depth
#define LOGGER_TASK_STACK     512         //stacklimit
#define LOGGER_TASK_PRIO      (configMAX_PRIORITIES - 4)       //priority upto 5 tasks

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} log_level_t;

typedef struct {
    uint32_t timestamp_ms;
    uint8_t task_id;
    log_level_t level;
    char message[LOG_MAX_MESSAGE_LEN];
} log_entry_t;

/* Public API */
bool logger_init(void);
void log_msg(log_level_t level, const char *fmt, ...);
void log_msg_from_isr(log_level_t level, const char *fmt, ...);

#endif