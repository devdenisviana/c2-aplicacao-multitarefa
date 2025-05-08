# Sistema de Monitoramento Simples com FreeRTOS para Raspberry Pi Pico

Este projeto demonstra um sistema de monitoramento básico implementado na Raspberry Pi Pico utilizando o FreeRTOS. O sistema é composto por três tarefas distintas que operam em paralelo para ler o estado de um botão, processar esse estado e controlar um LED.

## Arquitetura do Sistema

O sistema é dividido em três tarefas principais que comunicam entre si através de filas (queues) do FreeRTOS:

1.  **Tarefa de Leitura do Botão (`vTaskLeBotao`):**
    * Responsável por ler o estado de um botão conectado a um pino GPIO específico.
    * Realiza a leitura a cada 100 milissegundos.
    * Envia o estado lido (0 ou 1) para a fila `xButtonStateQueue`.

2.  **Tarefa de Processamento do Botão (`vTaskProcessaBotao`):**
    * Aguarda o recebimento do estado do botão da fila `xButtonStateQueue`.
    * Processa o estado recebido: se o botão estiver pressionado (nível lógico baixo, 0), define um comando para ligar o LED; caso contrário, define um comando para desligar o LED.
    * Envia o comando de controle do LED (0 ou 1) para a fila `xLEDCommandQueue`.

3.  **Tarefa de Controle do LED (`vTaskControlaLED`):**
    * Aguarda o recebimento do comando de controle do LED da fila `xLEDCommandQueue`.
    * Acende ou apaga o LED conectado a um pino GPIO específico com base no comando recebido.
    * Imprime o estado do LED ("ON" ou "OFF") na saída serial.

## Componentes Utilizados

* **Raspberry Pi Pico:** A placa microcontroladora que executa o sistema FreeRTOS e o código do projeto.
* **FreeRTOS:** Um sistema operacional de tempo real (RTOS) de código aberto que permite a criação e o gerenciamento de múltiplas tarefas.
* **Botão:** Um botão conectado ao pino GPIO 5 da Raspberry Pi Pico. O botão é configurado como ativo baixo (pressionado -> nível lógico 0).
* **LED:** Um LED conectado ao pino GPIO 13 da Raspberry Pi Pico.
* **Filas (Queues) do FreeRTOS:** Mecanismos de comunicação FIFO (First-In, First-Out) utilizados para transferir dados entre as tarefas (`xButtonStateQueue` e `xLEDCommandQueue`).

## Pinagem

* **Botão:** Conectado ao **GPIO 5**.
* **LED:** Conectado ao **GPIO 13**.

## Pré-requisitos

* **Ambiente de Desenvolvimento Raspberry Pi Pico:** O SDK da Raspberry Pi Pico deve estar instalado e configurado no seu sistema.
* **FreeRTOS para Raspberry Pi Pico:** O FreeRTOS deve estar integrado ao seu ambiente de desenvolvimento para a Raspberry Pi Pico.
* **Conexão Serial:** Uma conexão serial entre a Raspberry Pi Pico e o seu computador é necessária para visualizar a saída do `printf`.

## Como Compilar e Executar

1.  **Salve o Código:** Salve o código C fornecido em um arquivo (por exemplo, `main.c`).
2.  **Configure o Projeto:** Utilize o sistema de build da Raspberry Pi Pico (geralmente CMake e Makefiles) para configurar o projeto, garantindo que as bibliotecas do FreeRTOS estejam incluídas.
3.  **Construa o Projeto:** Compile o código C para gerar o arquivo `.uf2`.
4.  **Conecte a Raspberry Pi Pico:** Conecte a Raspberry Pi Pico ao seu computador via USB enquanto mantém pressionado o botão BOOTSEL.
5.  **Carregue o Arquivo `.uf2`:** Arraste e solte o arquivo `.uf2` para o dispositivo de armazenamento da Raspberry Pi Pico. A placa irá reiniciar e executar o programa.
6.  **Abra o Monitor Serial:** Utilize um programa de terminal serial (como `minicom`, `screen`, `PuTTY`) para conectar-se à porta serial da Raspberry Pi Pico na velocidade de 115200 bps. Você deverá ver as mensagens "LED ON" e "LED OFF" sendo impressas conforme o botão é pressionado e solto.

## Explicação do Código

* **Inclusão de Headers:** Inclui os headers necessários para o FreeRTOS (`FreeRTOS.h`, `task.h`, `queue.h`) e para o SDK da Raspberry Pi Pico (`pico/stdlib.h`, `stdio.h`).
* **Definição de Pinos:** Define constantes para os pinos GPIO conectados ao botão (`BOTAO_GPIO`) e ao LED (`LED_GPIO`).
* **Criação de Filas:** Declara e cria duas filas do FreeRTOS:
    * `xButtonStateQueue`: Para enviar o estado do botão da tarefa de leitura para a tarefa de processamento.
    * `xLEDCommandQueue`: Para enviar o comando de controle do LED da tarefa de processamento para a tarefa de controle.
    Ambas as filas têm capacidade para armazenar um único elemento do tipo `uint8_t`.
* **`vTaskLeBotao`:** A tarefa responsável por ler o estado do botão usando `gpio_get()` e enviar esse estado para a `xButtonStateQueue` usando `xQueueSend()`. A tarefa entra em um estado de espera por 100ms usando `vTaskDelay()`.
* **`vTaskProcessaBotao`:** Esta tarefa recebe o estado do botão da `xButtonStateQueue` usando `xQueueReceive()`. Com base no estado do botão (ativo baixo), ela determina o comando para o LED (1 para ligar, 0 para desligar) e envia esse comando para a `xLEDCommandQueue` usando `xQueueSend()`.
* **`vTaskControlaLED`:** A tarefa final recebe o comando de controle do LED da `xLEDCommandQueue` usando `xQueueReceive()`. Ela então utiliza `gpio_put()` para controlar o estado físico do LED e imprime uma mensagem indicando o estado do LED na saída serial.
* **`main()`:** A função principal do programa.
    * Inicializa a comunicação serial via USB com `stdio_init_all()`.
    * Configura os pinos GPIO para o botão (entrada, pull-up) e para o LED (saída).
    * Cria as filas `xButtonStateQueue` e `xLEDCommandQueue`.
    * Cria as três tarefas FreeRTOS (`vTaskLeBotao`, `vTaskProcessaBotao`, `vTaskControlaLED`) usando `xTaskCreate()`, especificando a função da tarefa, um nome descritivo, o tamanho da pilha alocada para a tarefa, os parâmetros passados para a tarefa (neste caso, `NULL`), a prioridade da tarefa e um handle para a tarefa (não utilizado neste exemplo).
    * Inicia o scheduler do FreeRTOS usando `vTaskStartScheduler()`, que começa a executar as tarefas criadas.
    * O `while (1);` no final impede que a função `main()` retorne, pois o FreeRTOS agora está gerenciando a execução das tarefas.

Este exemplo ilustra os conceitos básicos de multitarefa com FreeRTOS, incluindo a criação de tarefas e a comunicação entre elas usando filas. Ele demonstra como dividir um problema simples em tarefas menores e independentes para melhor organização e concorrência.