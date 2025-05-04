# Sistema de Monitoramento Simples com FreeRTOS para Raspberry Pi Pico

Este projeto demonstra um sistema de monitoramento básico utilizando o FreeRTOS no Raspberry Pi Pico. Ele consiste em três tarefas distintas que interagem para ler o estado de um botão e controlar um LED.

## Funcionalidades

* **Leitura do Botão:** Uma tarefa dedicada lê o estado de um botão conectado a um pino GPIO específico a cada 100 milissegundos.
* **Processamento do Botão:** Outra tarefa recebe o estado do botão e decide se o LED deve ser aceso ou apagado. A lógica implementada assume que o botão é ativo baixo (pressionado quando o nível lógico é 0).
* **Controle do LED:** Uma terceira tarefa recebe o comando (acender ou apagar) e atualiza o estado do LED conectado a outro pino GPIO. O estado do LED (ON ou OFF) também é impresso via serial USB.

## Hardware Necessário

* Raspberry Pi Pico
* Um botão (com resistor de pull-up externo ou utilizando o pull-up interno do Pico)
* Um LED (com resistor limitador de corrente)
* Cabos para conexão

## Conexões

* **Botão:** Conecte um terminal do botão ao pino GPIO 5 (conforme definido em `BOTAO_GPIO`). Se não estiver utilizando um resistor de pull-up externo, o pull-up interno é habilitado no código. O outro terminal do botão deve ser conectado ao GND.
* **LED:** Conecte o terminal positivo do LED (ânodo, geralmente a perna mais longa) a um resistor limitador de corrente e, em seguida, ao pino GPIO 13 (conforme definido em `LED_GPIO`). Conecte o terminal negativo do LED (cátodo, geralmente a perna mais curta) ao GND.

## Como Compilar e Executar

1.  **Pré-requisitos:** Certifique-se de ter o ambiente de desenvolvimento do Raspberry Pi Pico configurado, incluindo o Pico SDK e as ferramentas de compilação. Você também precisará ter o FreeRTOS incluído no seu projeto (conforme indicado no `CMakeLists.txt`).
2.  **CMakeLists.txt:** O arquivo `CMakeLists.txt` no seu projeto deve estar configurado para incluir o Pico SDK e o FreeRTOS. O exemplo fornecido no início da conversa (`CMakeLists.txt`) contém as configurações necessárias.
3.  **Compilação:** Navegue até o diretório raiz do seu projeto no terminal e execute os seguintes comandos:
    ```bash
    mkdir build
    cd build
    cmake ..
    make -j4
    ```
    Isso irá compilar o projeto e gerar o arquivo `.uf2`.
4.  **Execução:**
    * Conecte o Raspberry Pi Pico ao seu computador via cabo USB enquanto pressiona o botão BOOTSEL no Pico. Isso fará com que ele apareça como um dispositivo de armazenamento.
    * Copie o arquivo `.uf2` (geralmente encontrado na pasta `build`) para o dispositivo de armazenamento do Pico.
    * O Pico irá reiniciar e começar a executar o programa.
5.  **Monitoramento Serial:** Você pode usar um programa de terminal serial (como `minicom` no Linux ou PuTTY no Windows) para conectar-se à porta serial USB do Pico (geralmente na taxa de 115200 bps). O estado do LED (ON ou OFF) será impresso no terminal quando o botão for pressionado.

## Código Fonte

O código fonte principal (`multitarefa.c` ou o nome do seu arquivo principal) contém a lógica para as três tarefas e a inicialização do hardware e do FreeRTOS.

```c
/*
 * Sistema de Monitoramento Simples com FreeRTOS - Raspberry Pi Pico
 *
 * Tarefa 1: Lê estado do botão (a cada 100ms)
 * Tarefa 2: Processa o estado do botão e decide
 * Tarefa 3: Controla o LED (acende ou apaga)
 *
 * Autor: [Denis Jeronimo]
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Definição de pinos
#define BOTAO_GPIO 5   // Exemplo: botão no GPIO5
#define LED_GPIO 13    // LED onboard (GPIO13)

// Fila para comunicação entre Tarefa 1 -> Tarefa 2 -> Tarefa 3
QueueHandle_t xButtonQueue;

// Tarefa 1: Leitura do botão
void vTaskLeBotao(void *pvParameters) {
    while (1) {
        uint8_t buttonState = gpio_get(BOTAO_GPIO);   // Lê pino do botão (0 ou 1)

        // Envia o estado lido para a fila
        xQueueSend(xButtonQueue, &buttonState, portMAX_DELAY);

        // Aguarda 100ms antes de próxima leitura
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefa 2: Processamento do botão
void vTaskProcessaBotao(void *pvParameters) {
    uint8_t receivedState;

    while (1) {
        // Espera receber o estado do botão (bloqueia até receber)
        if (xQueueReceive(xButtonQueue, &receivedState, portMAX_DELAY) == pdTRUE) {

            // Se botão pressionado (nível lógico 0 — ativo baixo)
            if (receivedState == 0) {
                uint8_t acionaLED = 1;   // Liga LED
                xQueueSend(xButtonQueue, &acionaLED, portMAX_DELAY);
            } else {
                uint8_t apagaLED = 0;    // Desliga LED
                xQueueSend(xButtonQueue, &apagaLED, portMAX_DELAY);
            }
        }
    }
}

// Tarefa 3: Controle do LED
void vTaskControlaLED(void *pvParameters) {
    uint8_t ledCommand;

    while (1) {
        // Espera comando (1 = acende, 0 = apaga)
        if (xQueueReceive(xButtonQueue, &ledCommand, portMAX_DELAY) == pdTRUE) {

            gpio_put(LED_GPIO, ledCommand);   // Atualiza o LED físico

            printf("LED %s\n", ledCommand ? "ON" : "OFF");
        }
    }
}

int main() {
    stdio_init_all();   // Inicia USB serial (para printf)

    // Configuração dos pinos
    gpio_init(BOTAO_GPIO);
    gpio_set_dir(BOTAO_GPIO, GPIO_IN);
    gpio_pull_up(BOTAO_GPIO);   // Ativo baixo

    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    // Cria uma fila com capacidade para 1 elemento (uint8_t)
    xButtonQueue = xQueueCreate(1, sizeof(uint8_t));

    if (xButtonQueue != NULL) {
        // Cria tarefas
        xTaskCreate(vTaskLeBotao, "LeBotao", 256, NULL, 1, NULL);
        xTaskCreate(vTaskProcessaBotao, "ProcessaBotao", 256, NULL, 2, NULL);
        xTaskCreate(vTaskControlaLED, "ControlaLED", 256, NULL, 3, NULL);

        // Inicia o scheduler
        vTaskStartScheduler();
    }

    while (1);
    return 0;
}