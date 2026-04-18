#include "project_config.h"
#include "usart.hpp"
#include "stm32g0xx.h"


#ifdef USE_USART_RECEIVE_TASK_WITH_DMA
extern "C" void USART1_IRQHandler(void);
void DMA_Init(void);
void vUsartReceiveTask(void *pvParameters);
extern void ProcessCommand(const char *cmd);

#define DMA_BUF_SIZE 256
#define RING_BUF_SIZE 512

char dma_rx_buf[DMA_BUF_SIZE];      // Буфер DMA
char app_rx_buf[RING_BUF_SIZE];     // Кольцевой буфер
volatile uint16_t app_head = 0;
volatile uint16_t app_tail = 0;
TaskHandle_t xUsartReceiveTaskHandle;
#endif

void USART2_Init(void){

	//PA2 - TX, PA3 - RX, speed - 115200, Full duplex

	RCC->APBENR1 |= RCC_APBENR1_USART2EN;
	RCC->APBENR1 |= RCC_IOPENR_GPIOAEN;

	GPIOA->MODER &= ~GPIO_MODER_MODE2;              // Clear MODE for PA2
	GPIOA->MODER |= GPIO_MODER_MODE2_1;             // Alternate function mode
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;              // Output push-pull
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED2;        // Clear OSPEEDR for PA2
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_0;       // Medium speed
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2);            // Clear PUPDR
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2);           // Clear Alternate function for PA2
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_0;            // AF1 (USART2_TX)

	GPIOA->MODER &= ~GPIO_MODER_MODE3;              // Clear MODE for PA3
	GPIOA->MODER |= GPIO_MODER_MODE3_1;             // Alternate function mode
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED3;        // Clear OSPEEDR for PA3
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED3_0;       // Medium speed
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3;              // Clear PUPDR
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;             // pull-up
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL3);           // Clear Alternate function for PA3
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_0;            // AF1 (USART2_RX)

	USART2->BRR = (16000000U << 4) / 115200U;       // 0x08AE for FCLK = 16 MHz and baudrate = 115200

#ifdef USE_USART_RECEIVE_TASK_WITH_DMA
	//SART1->CR1 |= USART_CR1_RXNEIE;      // RXNE interrupt enable (when data received)
	USART1->CR1 |= USART_CR1_IDLEIE;      //  IDLE interrupt enable (IDLE line detected)

	USART1->CR3 = USART_CR3_DMAR;         // Включаем DMA для приема

	DMA_Init();

    xTaskCreate(
    		vUsartReceiveTask,            // Функция задачи
	        "vUsartReceiveTask",          // Имя задачи (для отладки)
	        1024,                         // Размер стека в словах (не байтах!)
	        NULL,                         // Параметр задачи
	        tskIDLE_PRIORITY + 2,         // Приоритет задачи
			&xUsartReceiveTaskHandle      // Указатель на дескриптор задачи
	);

	NVIC_EnableIRQ(USART1_IRQn);
	NVIC->IP[USART1_IRQn] = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	USART1->CR1 |= USART_CR1_RE;          // Receiver Enable
#endif

	USART2->CR1 |= USART_CR1_TE;          // Transmitter Enable
	USART2->CR1 |= USART_CR1_UE;          // Enable USART

}

void USART2_Send_Char(char chr){

	while(!(USART2->ISR & USART_ISR_TXFE));
	USART2->TDR = chr;
}

void USART2_Send_Str(const char* str){

	uint8_t i = 0;
	while(str[i]) USART2_Send_Char(str[i++]);
}

#ifdef USE_USART_RECEIVE_TASK_WITH_DMA
void DMA_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    /* Отключаем канал перед настройкой */
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    while (DMA1_Channel5->CCR & DMA_CCR_EN);

    /* Настройка канала */
    DMA1_Channel5->CPAR = (uint32_t)&USART1->DR;           // Источник: USART DR
    DMA1_Channel5->CMAR = (uint32_t)dma_rx_buf;            // Приемник: буфер
    DMA1_Channel5->CNDTR = DMA_BUF_SIZE;                   // Количество байт

    DMA1_Channel5->CCR &= ~DMA_CCR_PSIZE;                  // Peripheral size 8-bits
    DMA1_Channel5->CCR &= ~DMA_CCR_MSIZE;                  // Memory size 8-bits
    DMA1_Channel5->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;     // PL = 11 (Very High)
    DMA1_Channel5->CCR |= DMA_CCR_MINC;                    // MINC = 1 (инкремент памяти)
    DMA1_Channel5->CCR &= ~DMA_CCR_PINC;                   // PINC = 0 (инкремент переферии выключен)
    DMA1_Channel5->CCR |= DMA_CCR_CIRC;                    // циклический режим
    DMA1_Channel5->CCR &= ~DMA_CCR_DIR;                    // Read from peripheral

    /* Включаем канал */
    DMA1_Channel5->CCR |= DMA_CCR_EN;

}

extern "C" void USART2_IRQHandler(void){  //Receive data with interrupt

    if (USART1->SR & USART_SR_IDLE) {
        // 1. Сброс флага IDLE
        volatile uint32_t tmp = USART1->SR;
        tmp = USART1->DR;
        (void)tmp;

        // 2. Вычисляем размер пакета
        uint16_t dma_remaining = DMA1_Channel5->CNDTR;
		uint16_t dma_current = DMA_BUF_SIZE - dma_remaining;  // Куда запишется следующий байт

		// Статическая переменная для отслеживания позиции
		static uint16_t last_dma_pos = 0;

		// Вычисляем, сколько новых байт пришло (с учётом wrap-around)
		uint16_t received;
		if (dma_current >= last_dma_pos) {
			received = dma_current - last_dma_pos;
		} else {
			received = (DMA_BUF_SIZE - last_dma_pos) + dma_current;    // DMA обернулся: [last..end] + [0..current]
		}

		uint16_t dma_start = last_dma_pos;  // Данные начинаются с last_dma_pos

        // 3. Копирование блока из DMA в Кольцевой буфер с учётом wrap-around DMA-буфера

        for (uint16_t i = 0; i < received; i++) {
            uint16_t src_idx = (dma_start + i) % DMA_BUF_SIZE;
            uint16_t next_head = (app_head + 1) % RING_BUF_SIZE;

            if (next_head != app_tail) {
                app_rx_buf[app_head] = dma_rx_buf[src_idx];
                app_head = next_head;
            } else {
                break;  // Буфер полон
            }
        }

        // Обновляем позицию для следующего прерывания
        last_dma_pos = dma_current;

        // 4. Сброс и перезапуск DMA
        DMA1_Channel5->CNDTR = DMA_BUF_SIZE;

        // 5. Уведомление задачи (минимальный оверхед)
        BaseType_t xWake = pdFALSE;
        vTaskNotifyGiveFromISR(xUsartReceiveTaskHandle, &xWake);
        portYIELD_FROM_ISR(xWake);
    }

    // Обработка ошибок
    if (USART1->SR & (USART_SR_ORE | USART_SR_PE | USART_SR_FE)) {
        volatile uint32_t tmp_sr = USART1->SR;
        volatile uint32_t tmp_dr = USART1->DR;  // Сбрасывает ORE
        (void)tmp_sr; (void)tmp_dr;
    }
}

void vUsartReceiveTask(void *arg){
    char cmd_buf[64] = {0};       // Локальный буфер команды
    uint16_t cmd_idx = 0;         // Индекс в буфере

    for (;;) {
    	/* =============================================================
    	 * 1. Если есть неполная команда — ждём продолжения (50 мс)
    	 * ============================================================= */
        if (cmd_idx > 0) {
        	// Если до этого команда пришла без разделителя, то ждем уведомления 50 мс, вдруг есть задержка
        	// и остальная часть команды (включая разделитель) еще не пришла
        	// Если в течении 50 мс придет продолжение с разделителем, то полученное суммируется и обрабатывается (
        	// Если по истечении 50 мс, так ничего не придет, то обрабатываем как есть
        	if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)) == 0) { // Если данные пришли до истечения таймаута, то вовращаемое значение 1
        		                                                    // Переходим к началу цикла for
                cmd_buf[cmd_idx] = '\0';   // Null-terminate для безопасности
                ProcessCommand(cmd_buf);   // Функция обработки
                cmd_idx = 0;               // Сброс для следующего пакета
        	}
        }
        /* =============================================================
         * 2. Если до этого команды нет — ждём новые данные (бесконечно)
         * ============================================================= */
        else ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Ждём уведомление от ISR (когда новый пакет будет принят, IDLE сработает)

        /* =============================================================
         * 3. Вычитываем ВСЕ доступные байты, которые пришли
         * ============================================================= */
        while (app_head != app_tail) {
            char ch = app_rx_buf[app_tail];
            app_tail = (app_tail + 1) % RING_BUF_SIZE;

            // Разделитель завершает команду
            if (ch == '\n' || ch == '\r') {
                if (cmd_idx > 0) {
                    cmd_buf[cmd_idx] = '\0';
                    ProcessCommand(cmd_buf);
                    cmd_idx = 0;  // Сброс для следующей команды
                }
            }
            // Накопление
            else if (cmd_idx < sizeof(cmd_buf) - 1) {
                cmd_buf[cmd_idx++] = ch;
            }
        }
    }
}
#endif
