#include "fsl_device_registers.h"
#include "board.h"
#include "logger.h"

//prod 
void vWorkerTask(void *pvParameters) {
    uint32_t id = (uint32_t)pvParameters;
    uint32_t counter = 0;

    for (;;) {
        log_msg(LOG_LEVEL_INFO, "Worker %d running iteration %u", id, counter++);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    // init log sys
    if (!logger_init()) {
        while (1); 
    }

    // limited to 12 tasks
    for (uint32_t i = 0; i < 12; i++) {
        char task_name[16];
        snprintf(task_name, sizeof(task_name), "Worker_%lu", i);
        xTaskCreate(vWorkerTask, task_name, configMINIMAL_STACK_SIZE + 128, (void *)i, tskIDLE_PRIORITY + 2, NULL);
    }

    vTaskStartScheduler();

    while (1);
}