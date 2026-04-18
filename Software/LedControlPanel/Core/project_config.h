#ifndef __PROJECT_CONFIG_H
#define __PROJECT_CONFIG_H

// Global settings for project

//#define PRODUCTION                 // Uncomment at production




#ifdef PRODUCTION

#else
//#define CHECK_FOR_STACK_OVERFLOW_TASK     // Включение проверки переполнения стека задач FreeRTOS
                                          // обработчик в мэйне void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)

#define USE_USART_LOGGING                 // Включает отправку данных по USART (логгирование, использование usart_printf)

//#define USE_USART_RECEIVE_TASK_WITH_DMA   // Включает создание задачи приема данных (и команд) по USART c DMA с кольцевым буфером и ProcessCommand


#endif




#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


#endif /* __PROJECT_CONFIG_H */
