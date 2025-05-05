/*
 * Sistema de Monitoramento Simples com FreeRTOS - Raspberry Pi Pico
 *
 * Tarefa 1: Lê estado do botão (a cada 100ms)
 * Tarefa 2: Processa o estado do botão e decide
 * Tarefa 3: Controla o LED (acende ou apaga)
 *
 * Autor: Denis Jeronimo 
 */

 #include "FreeRTOS.h"
 #include "task.h"
 #include "queue.h"
 #include "pico/stdlib.h"
 #include <stdio.h>
 
 // Definição de pinos 
 #define BOTAO_GPIO 5  // botão no GPIO5
 #define LED_GPIO 13   // LED no GPIO13
 
 // Filas para comunicação
 QueueHandle_t xButtonStateQueue;  // Leitura -> Processamento
 QueueHandle_t xLEDCommandQueue;   // Processamento -> Controle
 
 // Tarefa 1: Leitura do botão
 void vTaskLeBotao(void *pvParameters) {
     while (1) {
         uint8_t buttonState = gpio_get(BOTAO_GPIO);  // Lê pino do botão (0 ou 1)
 
         // Envia o estado lido para a fila
         xQueueSend(xButtonStateQueue, &buttonState, portMAX_DELAY);
 
         // Aguarda 100ms antes de próxima leitura
         vTaskDelay(pdMS_TO_TICKS(100));
     }
 }
 
 // Tarefa 2: Processamento do botão
 void vTaskProcessaBotao(void *pvParameters) {
     uint8_t receivedState;
 
     while (1) {
         // Espera receber o estado do botão
         if (xQueueReceive(xButtonStateQueue, &receivedState, portMAX_DELAY) == pdTRUE) {
 
             uint8_t ledCommand;
 
             // Se botão pressionado (nível lógico 0 — ativo baixo)
             if (receivedState == 0) {
                 ledCommand = 1;  // Liga LED
             } else {
                 ledCommand = 0;  // Desliga LED
             }
 
             // Envia o comando de controle do LED para próxima tarefa
             xQueueSend(xLEDCommandQueue, &ledCommand, portMAX_DELAY);
         }
     }
 }
 
 // Tarefa 3: Controle do LED
 void vTaskControlaLED(void *pvParameters) {
     uint8_t ledCommand;
 
     while (1) {
         // Espera comando (1 = acende, 0 = apaga)
         if (xQueueReceive(xLEDCommandQueue, &ledCommand, portMAX_DELAY) == pdTRUE) {
 
             gpio_put(LED_GPIO, ledCommand);  // Atualiza o LED físico
 
             printf("LED %s\n", ledCommand ? "ON" : "OFF");
         }
     }
 }
 
 int main() {
     stdio_init_all();  // Inicia USB serial (para printf)
 
     // Configuração dos pinos
     gpio_init(BOTAO_GPIO);
     gpio_set_dir(BOTAO_GPIO, GPIO_IN);
     gpio_pull_up(BOTAO_GPIO);  // Ativo baixo
 
     gpio_init(LED_GPIO);
     gpio_set_dir(LED_GPIO, GPIO_OUT);
 
     // Cria as filas (1 elemento cada, tipo uint8_t)
     xButtonStateQueue = xQueueCreate(1, sizeof(uint8_t));
     xLEDCommandQueue  = xQueueCreate(1, sizeof(uint8_t));
 
     if (xButtonStateQueue != NULL && xLEDCommandQueue != NULL) {
         // Cria tarefas (256 - tamanho da pilha)
         xTaskCreate(vTaskLeBotao, "LeBotao", 256, NULL, 1, NULL);
         xTaskCreate(vTaskProcessaBotao, "ProcessaBotao", 256, NULL, 2, NULL);
         xTaskCreate(vTaskControlaLED, "ControlaLED", 256, NULL, 3, NULL);
 
         // Inicia o scheduler
         vTaskStartScheduler();
     }
 
     while (1);
     return 0;
 }
 