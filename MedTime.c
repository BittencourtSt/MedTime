#include <stdio.h>                    // Inclui a biblioteca padrão para entrada e saída, permitindo usar funções como printf.
#include "pico/stdlib.h"               // Inclui a biblioteca para funcionalidades básicas do Raspberry Pi Pico, como controle de pinos e temporização.
#include "hardware/i2c.h"              // Inclui a biblioteca para comunicação I2C com dispositivos periféricos.
#include "ssd1306.h"                   // Inclui a biblioteca para controle de displays OLED baseados no driver SSD1306.
#include "hardware/adc.h"              // Inclui a biblioteca para ler valores analógicos (ADC).
#include <stdlib.h>                    // Inclui a biblioteca padrão para funções como alocação de memória e geração de números aleatórios.
#include <time.h>                      // Inclui a biblioteca para funções relacionadas ao tempo e data.
#include <string.h>                    // Inclui a biblioteca para manipulação de strings, como funções de comparação e cópia.
#include "pico/cyw43_arch.h"           // Inclui a biblioteca para uso de Wi-Fi no Raspberry Pi Pico W.
#include "lwip/pbuf.h"                 // Inclui a biblioteca para buffers de pacotes de rede (LWIP).
#include "lwip/tcp.h"                  // Inclui a biblioteca para protocolos TCP.
#include "lwip/dns.h"                  // Inclui a biblioteca para resolução de DNS.
#include "lwip/init.h"                 // Inclui a biblioteca para inicializar a pilha de rede LWIP.


// Protótipos das funções
void Medicamento1();                   // Declaração da função Medicamento1.
void Medicamento2();                   // Declaração da função Medicamento2.
void Medicamento3();                   // Declaração da função Medicamento3.
void rotina_medicamento(void (*callback)(), _Bool *flag);  // Declaração da função rotina_medicamento, que recebe um ponteiro para função (callback) e um ponteiro para flag (controle de estado).

#define I2C_PORT i2c1                   // Define o I2C utilizado, neste caso, o i2c1.
#define PINO_SDA 14                     // Define o pino 14 como o pino SDA para a comunicação I2C.
#define PINO_SCL 15                     // Define o pino 15 como o pino SCL para a comunicação I2C.
#define OLED_ADDR 0x3C                  // Define o endereço do display OLED como 0x3C (endereço padrão para SSD1306).
#define BTN_B 6                         // Define o pino 6 como o pino do Botão B (usado para interação).
#define LED_R_PIN 13                    // Define o pino 13 como o pino do LED vermelho (parte do LED RGB).
#define LED_G_PIN 11                    // Define o pino 11 como o pino do LED verde (parte do LED RGB).
#define LED_B_PIN 12                    // Define o pino 12 como o pino do LED azul (parte do LED RGB).
#define BUZZER_PIN 21                   // Define o pino 21 como o pino do buzzer (para alertas sonoros).
const int SW = 22;                      // Define o pino 22 como o pino para o Botão SW.

// Configurações do Wi-Fi e ThingSpeak
#define WIFI_SSID "SANBELE_2G"           // Define o SSID da rede Wi-Fi para conectar.
#define WIFI_PASS "28021997"             // Define a senha da rede Wi-Fi.
#define THINGSPEAK_HOST "api.thingspeak.com"  // Define o endereço do servidor ThingSpeak.
#define THINGSPEAK_PORT 80               // Define a porta do servidor ThingSpeak (porta HTTP padrão).
#define API_KEY "67QZT6B5M5KBXZ7E"       // Define a chave de escrita do ThingSpeak para enviar dados.

struct tcp_pcb *tcp_client_pcb = NULL;  // Cria um ponteiro para a estrutura do controle do protocolo TCP, inicializado como NULL.
ip_addr_t server_ip;                    // Declara a variável para armazenar o endereço IP do servidor.
bool tcp_connected = false;             // Variável booleana para verificar se a conexão TCP foi estabelecida.

ssd1306_t disp;                          // Declara a estrutura para armazenar o display SSD1306.
bool running = false;                    // Variável booleana para controlar o estado de execução do programa (se está em execução ou não).

#define ADC_READINGS 10                  // Define a quantidade de leituras ADC a serem feitas para média ou outro processamento.
uint16_t adc_buffer[ADC_READINGS];      // Cria um buffer de leituras do ADC, com o tamanho definido acima.
uint16_t adc_index = 0;                 // Índice que controla a posição atual no buffer de leituras do ADC.
const uint DEAD_ZONE = 128;              // Define uma zona morta (valor de limiar) para operações de leitura ou de controle, como ADC ou botões.

bool sw_estado_anterior = true;         // Variável para armazenar o estado anterior do botão SW (usado para detectar mudanças de estado).
bool btn_b_estado_anterior = true;      // Variável para armazenar o estado anterior do Botão B.

int hora_atual = 0;                     // Variável para armazenar a hora atual (horas).
int minuto_atual = 0;                   // Variável para armazenar o minuto atual.
bool definindo_horario = true;          // Variável para indicar se o horário está sendo definido.
bool definindo_hora = true;             // Variável para indicar se a hora está sendo definida (usada para ajustes de hora/minuto).

bool medicamento_consumido = false;     // Flag para verificar se o medicamento foi consumido.
bool medicamento1_consumido = false;    // Flag para verificar se o primeiro medicamento foi consumido.
bool medicamento2_consumido = false;    // Flag para verificar se o segundo medicamento foi consumido.
bool medicamento3_consumido = false;    // Flag para verificar se o terceiro medicamento foi consumido.

// Função para enviar dados ao ThingSpeak
void enviar_dados_thingspeak(const char *horario) {  // Define a função que envia dados ao ThingSpeak. Ela recebe uma string 'horario' como parâmetro.
    if (!tcp_client_pcb || !tcp_connected) {          // Verifica se o ponteiro do controle TCP (tcp_client_pcb) ou a conexão TCP não estão inicializados.
        printf("Erro: Conexão TCP não estabelecida.\n");  // Se a condição for verdadeira, exibe uma mensagem de erro e retorna da função.
        return;
    }

    char request[256];                                // Cria um buffer de 256 caracteres para armazenar a requisição HTTP.
    snprintf(request, sizeof(request),               // Formata a string da requisição HTTP para enviar ao ThingSpeak.
        "GET /update?api_key=%s&field1=%s HTTP/1.1\r\n"  // Comando GET com a chave de API e o campo de dados que será enviado (horário).
        "Host: %s\r\n"                                  // Cabeçalho HTTP com o host do ThingSpeak.
        "Connection: close\r\n"                         // Indica que a conexão será fechada após a transmissão.
        "\r\n",                                         // Linha em branco para finalizar o cabeçalho.
        API_KEY, horario, THINGSPEAK_HOST);             // Substitui as variáveis (API_KEY, horario e THINGSPEAK_HOST) na string da requisição.

    err_t err = tcp_write(tcp_client_pcb, request, strlen(request), TCP_WRITE_FLAG_COPY);  // Envia a requisição HTTP para o servidor ThingSpeak.
    if (err != ERR_OK) {                             // Verifica se ocorreu algum erro ao escrever no TCP.
        printf("Erro ao escrever no TCP: %d\n", err);  // Caso haja erro, exibe uma mensagem de erro e retorna.
        return;
    }

    err = tcp_output(tcp_client_pcb);                 // Solicita a saída dos dados TCP (envio dos dados para o servidor).
    if (err != ERR_OK) {                             // Verifica se houve erro ao tentar enviar os dados.
        printf("Erro ao enviar dados TCP: %d\n", err);  // Caso haja erro, exibe uma mensagem de erro e retorna.
        return;
    }

    printf("Dados enviados ao ThingSpeak: %s\n", horario);  // Se os dados foram enviados com sucesso, imprime uma mensagem de confirmação.
}

// Callback quando recebe resposta do ThingSpeak
static err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {  // Define a função de callback que é chamada quando uma resposta é recebida via TCP.
    if (p == NULL) {                               // Verifica se o buffer de dados recebidos (p) é NULL, o que indica que a conexão foi fechada.
        tcp_close(tpcb);                          // Fecha a conexão TCP.
        tcp_client_pcb = NULL;                    // Limpa o ponteiro para o PCB (controle de protocolo de controle de transmissão) após fechar a conexão.
        tcp_connected = false;                    // Marca a conexão como fechada (não conectada).
        return ERR_OK;                             // Retorna o código de erro "ERR_OK" indicando que a operação foi concluída corretamente.
    }

    printf("Resposta do ThingSpeak: %.*s\n", p->len, (char *)p->payload);  // Imprime a resposta recebida do ThingSpeak. O `p->len` especifica o comprimento da resposta, e `p->payload` é o ponteiro para os dados recebidos.
    pbuf_free(p);                               // Libera o buffer (pbuf) de dados recebidos, pois ele não é mais necessário.
    return ERR_OK;                               // Retorna "ERR_OK", indicando que o callback foi processado corretamente.
}

// Callback quando a conexão TCP é estabelecida
static err_t http_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {  // Define a função de callback chamada quando a conexão TCP é estabelecida.
    if (err != ERR_OK) {                            // Verifica se ocorreu algum erro na tentativa de conexão (erro diferente de "ERR_OK").
        printf("Erro na conexão TCP\n");            // Se houve erro, imprime uma mensagem de erro.
        return err;                                 // Retorna o erro encontrado, indicando que a operação de conexão falhou.
    }

    printf("Conectado ao ThingSpeak!\n");            // Se a conexão for bem-sucedida, imprime uma mensagem de sucesso.
    tcp_connected = true;                            // Marca a conexão como estabelecida, alterando a variável `tcp_connected` para true.
    return ERR_OK;                                   // Retorna "ERR_OK" indicando que a conexão foi estabelecida com sucesso.
}

// Resolver DNS e conectar ao servidor
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {  // Define a função de callback para resolver o DNS e obter o endereço IP do servidor.
    if (ipaddr) {                                 // Verifica se o endereço IP foi resolvido com sucesso (ipaddr não é NULL).
        printf("Endereço IP do ThingSpeak: %s\n", ipaddr_ntoa(ipaddr));  // Se o endereço IP foi resolvido, imprime o endereço IP do ThingSpeak.
        tcp_client_pcb = tcp_new();               // Cria um novo controle de protocolo TCP (PCB) para a conexão.
        if (!tcp_client_pcb) {                    // Verifica se a criação do PCB falhou.
            printf("Erro ao criar PCB.\n");       // Se falhou, imprime uma mensagem de erro.
            return;                               // Retorna da função sem tentar conectar.
        }
        tcp_connect(tcp_client_pcb, ipaddr, THINGSPEAK_PORT, http_connected_callback);  // Tenta estabelecer uma conexão TCP com o servidor ThingSpeak, passando o endereço IP resolvido e a porta.
    } else {
        printf("Falha na resolução de DNS\n");     // Se a resolução de DNS falhar (ipaddr é NULL), imprime uma mensagem de erro.
    }
}

void inicializa() {  // Define a função de inicialização do sistema, que configura todos os periféricos e a conexão Wi-Fi.

    stdio_init_all();                        // Inicializa a comunicação padrão (serial, para usar printf e outras funções de E/S).
    
    adc_init();                              // Inicializa o ADC (Conversor Analógico-Digital) para ler sinais analógicos.
    adc_gpio_init(26);                       // Inicializa o pino 26 como entrada analógica para o ADC.
    adc_gpio_init(27);                       // Inicializa o pino 27 como entrada analógica para o ADC.
    
    i2c_init(I2C_PORT, 400 * 1000);          // Inicializa o barramento I2C na velocidade de 400kHz.
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);  // Configura o pino de clock I2C (SCL) para função I2C.
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);  // Configura o pino de dados I2C (SDA) para função I2C.
    gpio_pull_up(PINO_SCL);                  // Ativa o resistor de pull-up no pino SCL.
    gpio_pull_up(PINO_SDA);                  // Ativa o resistor de pull-up no pino SDA.

    disp.external_vcc = false;               // Define que o display OLED não usará uma fonte de alimentação externa (externa ou interna).
    ssd1306_init(&disp, 128, 64, OLED_ADDR, I2C_PORT);  // Inicializa o display OLED SSD1306 com a resolução 128x64, endereço I2C e porta I2C.

    gpio_init(BTN_B);                        // Inicializa o pino do botão B.
    gpio_set_dir(BTN_B, GPIO_IN);            // Configura o pino do botão B como entrada.
    gpio_pull_up(BTN_B);                     // Ativa o resistor de pull-up para o botão B.

    gpio_init(SW);                           // Inicializa o pino do botão SW.
    gpio_set_dir(SW, GPIO_IN);               // Configura o pino do botão SW como entrada.
    gpio_pull_up(SW);                        // Ativa o resistor de pull-up para o botão SW.

    gpio_init(LED_R_PIN);                    // Inicializa o pino do LED vermelho.
    gpio_set_dir(LED_R_PIN, GPIO_OUT);       // Configura o pino do LED vermelho como saída.
    gpio_put(LED_R_PIN, 0);                  // Desliga o LED vermelho (define o pino como 0).

    gpio_init(LED_G_PIN);                    // Inicializa o pino do LED verde.
    gpio_set_dir(LED_G_PIN, GPIO_OUT);       // Configura o pino do LED verde como saída.
    gpio_put(LED_G_PIN, 0);                  // Desliga o LED verde (define o pino como 0).

    gpio_init(LED_B_PIN);                    // Inicializa o pino do LED azul.
    gpio_set_dir(LED_B_PIN, GPIO_OUT);       // Configura o pino do LED azul como saída.
    gpio_put(LED_B_PIN, 0);                  // Desliga o LED azul (define o pino como 0).

    gpio_init(BUZZER_PIN);                   // Inicializa o pino do buzzer.
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);      // Configura o pino do buzzer como saída.
    gpio_put(BUZZER_PIN, 0);                 // Desliga o buzzer (define o pino como 0).

    // Inicializa Wi-Fi
    if (cyw43_arch_init()) {                  // Tenta inicializar o módulo Wi-Fi.
        printf("Falha ao iniciar Wi-Fi\n");   // Se falhar, imprime mensagem de erro e retorna da função.
        return;
    }

    cyw43_arch_enable_sta_mode();             // Coloca o módulo Wi-Fi no modo "Station" (conectar-se a uma rede existente).
    printf("Conectando ao Wi-Fi...\n");       // Imprime mensagem informando que está tentando conectar ao Wi-Fi.

    if (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK)) {  // Conecta-se à rede Wi-Fi de forma bloqueante (aguarda a conexão).
        printf("Falha ao conectar ao Wi-Fi\n"); // Se falhar, imprime mensagem de erro e retorna da função.
        return;
    }

    printf("Wi-Fi conectado!\n");              // Se a conexão for bem-sucedida, imprime mensagem de sucesso.
}

bool detectar_borda_baixa(uint gpio, bool *estado_anterior) {  // Define a função que detecta uma borda baixa em um pino GPIO.
    bool estado_atual = gpio_get(gpio);  // Lê o estado atual do pino GPIO e armazena na variável 'estado_atual'.
    
    if (*estado_anterior && !estado_atual) {  // Verifica se o estado anterior era alto (1) e o estado atual é baixo (0) — condição de borda de descida (borda baixa).
        *estado_anterior = estado_atual;  // Atualiza o estado anterior para o estado atual (baixo).
        return true;  // Retorna true, indicando que a borda baixa foi detectada.
    }
    
    *estado_anterior = estado_atual;  // Atualiza o estado anterior para o estado atual, independentemente da detecção de borda.
    return false;  // Retorna false, indicando que a borda baixa não foi detectada.
}

void mostrar_relogio() {  // Define a função para exibir o relógio no display OLED.

    ssd1306_clear(&disp);  // Limpa a tela do display OLED antes de exibir novas informações.

    char horario_str[10];  // Declara um array de caracteres para armazenar o horário como string.

    snprintf(horario_str, sizeof(horario_str), "%02d:%02d", hora_atual, minuto_atual);  // Formata o horário no formato "HH:MM" e armazena na variável 'horario_str'.

    ssd1306_draw_string(&disp, 30, 24, 2, horario_str);  // Desenha a string do horário na posição (30, 24) no display OLED, com tamanho de fonte 2.

    ssd1306_show(&disp);  // Atualiza o display OLED para mostrar o conteúdo desenhado.
}

void definir_horario() {  // Define a função para permitir que o usuário defina a hora e os minutos.

    while (definindo_horario) {  // Continua a execução enquanto a variável 'definindo_horario' for verdadeira.

        ssd1306_clear(&disp);  // Limpa a tela do display OLED.

        if (definindo_hora) {  // Verifica se está definindo a hora.
            ssd1306_draw_string(&disp, 6, 18, 1.5, "Definir Hora:");  // Exibe a mensagem "Definir Hora:" na tela.
            char hora_str[3];  // Declara um array para armazenar a hora como string.
            snprintf(hora_str, sizeof(hora_str), "%02d", hora_atual);  // Formata a hora atual no formato "HH".
            ssd1306_draw_string(&disp, 50, 34, 1.5, hora_str);  // Exibe a hora no display.
        } else {  // Se não estiver definindo a hora, significa que está definindo os minutos.
            ssd1306_draw_string(&disp, 6, 18, 1.5, "Definir Minuto:");  // Exibe a mensagem "Definir Minuto:" na tela.
            char minuto_str[3];  // Declara um array para armazenar os minutos como string.
            snprintf(minuto_str, sizeof(minuto_str), "%02d", minuto_atual);  // Formata os minutos no formato "MM".
            ssd1306_draw_string(&disp, 50, 34, 1.5, minuto_str);  // Exibe os minutos no display.
        }

        ssd1306_show(&disp);  // Atualiza o display OLED para mostrar as informações.

        adc_select_input(0);  // Seleciona a entrada do ADC (pino analógico 0).
        uint adc_y_raw = adc_read();  // Lê o valor bruto do ADC.

        if (adc_y_raw < 1500) {  // Se o valor do ADC for menor que 1500, aumenta a hora ou minuto.
            if (definindo_hora) {  // Se estiver definindo a hora, incrementa a hora.
                hora_atual = (hora_atual + 1) % 24;  // Aumenta a hora e garante que fique dentro do intervalo de 0 a 23.
            } else {  // Se estiver definindo os minutos, incrementa os minutos.
                minuto_atual = (minuto_atual + 1) % 60;  // Aumenta os minutos e garante que fique dentro do intervalo de 0 a 59.
            }
            busy_wait_us_32(250000);  // Aguarda 250 milissegundos antes de continuar, para evitar mudanças rápidas.
        } else if (adc_y_raw > 2500) {  // Se o valor do ADC for maior que 2500, diminui a hora ou minuto.
            if (definindo_hora) {  // Se estiver definindo a hora, decrementa a hora.
                hora_atual = (hora_atual - 1 + 24) % 24;  // Diminui a hora e garante que fique dentro do intervalo de 0 a 23.
            } else {  // Se estiver definindo os minutos, decrementa os minutos.
                minuto_atual = (minuto_atual - 1 + 60) % 60;  // Diminui os minutos e garante que fique dentro do intervalo de 0 a 59.
            }
            busy_wait_us_32(250000);  // Aguarda 250 milissegundos antes de continuar.
        }

        if (detectar_borda_baixa(SW, &sw_estado_anterior)) {  // Verifica se foi pressionado o botão SW (detecção de borda baixa).
            if (definindo_hora) {  // Se estiver definindo a hora, passa para a definição dos minutos.
                definindo_hora = false;  // Altera para definir minutos.
            } else {  // Caso contrário, termina o processo de definição do horário.
                definindo_horario = false;  // Marca o processo de definição do horário como terminado.
                break;  // Sai do loop.
            }
        }
    }

    mostrar_relogio();  // Após definir o horário, chama a função para mostrar o relógio na tela.
}

void atualizar_relogio() {  // Define a função para atualizar o relógio.

    minuto_atual++;  // Incrementa o valor dos minutos.

    if (minuto_atual >= 60) {  // Se os minutos atingirem ou ultrapassarem 60.
        minuto_atual = 0;  // Reseta os minutos para 0.
        hora_atual = (hora_atual + 1) % 24;  // Incrementa a hora, e garante que ela fique dentro do intervalo de 0 a 23 (circular).
    }

    mostrar_relogio();  // Atualiza a exibição do relógio no display OLED com o novo horário.
}

void verificar_medicamentos() {  // Define a função que verifica se é hora de tomar os medicamentos.

    if (hora_atual == 8 && !medicamento1_consumido) {  // Se for 8 horas e o medicamento 1 não foi consumido ainda.
        rotina_medicamento(Medicamento1, &medicamento1_consumido);  // Chama a função rotina_medicamento para o Medicamento1 e marca como consumido.
    }

    if (hora_atual == 10 && !medicamento2_consumido) {  // Se for 10 horas e o medicamento 2 não foi consumido ainda.
        rotina_medicamento(Medicamento2, &medicamento2_consumido);  // Chama a função rotina_medicamento para o Medicamento2 e marca como consumido.
    }

    if (hora_atual == 21 && !medicamento3_consumido) {  // Se for 21 horas e o medicamento 3 não foi consumido ainda.
        rotina_medicamento(Medicamento3, &medicamento3_consumido);  // Chama a função rotina_medicamento para o Medicamento3 e marca como consumido.
    }
}

void mostrar_relogio_continuamente() {  // Define a função para mostrar o relógio continuamente.

    while (!definindo_horario) {  // Executa enquanto a variável 'definindo_horario' for falsa (ou seja, enquanto não estiver no modo de definição de horário).
        
        mostrar_relogio();  // Exibe o relógio atual no display OLED.
        verificar_medicamentos();  // Verifica se é hora de tomar algum medicamento.

        sleep_ms(60000);  // Aguarda por 60.000 milissegundos (ou 1 minuto), fazendo uma pausa na execução.

        atualizar_relogio();  // Atualiza o relógio após 1 minuto, incrementando os minutos.

        if (detectar_borda_baixa(SW, &sw_estado_anterior)) {  // Verifica se foi pressionado o botão SW (detecção de borda baixa).
            definindo_horario = true;  // Altera para o modo de definição de horário.
            definindo_hora = true;  // Inicia a definição da hora (não minutos ainda).
            definir_horario();  // Chama a função para definir o horário.
        }
    }
}

void buzzer_melodia1() {  // Define a função que toca a melodia 1 no buzzer.

    for (int i = 0; i < 5; i++) {  // Um loop que repete 5 vezes para tocar a melodia.
        gpio_put(BUZZER_PIN, 1);  // Aciona o buzzer (setando o pino para 1, ativando o sinal).
        sleep_ms(200);  // Aguarda por 200 milissegundos (nota).
        gpio_put(BUZZER_PIN, 0);  // Desativa o buzzer (setando o pino para 0, desativando o sinal).
        sleep_ms(200);  // Aguarda por 200 milissegundos antes de tocar a próxima nota.
    }
}

void buzzer_melodia2() {  // Define a função que toca a melodia 2 no buzzer.

    for (int i = 0; i < 5; i++) {  // Um loop que repete 5 vezes para tocar a melodia.
        gpio_put(BUZZER_PIN, 1);  // Aciona o buzzer (setando o pino para 1).
        sleep_ms(100);  // Aguarda por 100 milissegundos (nota).
        gpio_put(BUZZER_PIN, 0);  // Desativa o buzzer (setando o pino para 0).
        sleep_ms(300);  // Aguarda por 300 milissegundos antes de tocar a próxima nota.
    }
}

void buzzer_melodia3() {  // Define a função que toca a melodia 3 no buzzer.

    for (int i = 0; i < 5; i++) {  // Um loop que repete 5 vezes para tocar a melodia.
        gpio_put(BUZZER_PIN, 1);  // Aciona o buzzer (setando o pino para 1).
        sleep_ms(300);  // Aguarda por 300 milissegundos (nota).
        gpio_put(BUZZER_PIN, 0);  // Desativa o buzzer (setando o pino para 0).
        sleep_ms(100);  // Aguarda por 100 milissegundos antes de tocar a próxima nota.
    }
}

void Medicamento1() {  // Função que sinaliza a hora de tomar o primeiro medicamento.

    gpio_put(LED_R_PIN, 1);  // Liga o LED vermelho.
    gpio_put(LED_G_PIN, 0);  // Desliga o LED verde.
    gpio_put(LED_B_PIN, 0);  // Desliga o LED azul.

    buzzer_melodia1();  // Toca a primeira melodia no buzzer para alertar o usuário.

    ssd1306_clear(&disp);  // Limpa o display OLED.
    ssd1306_draw_string(&disp, 20, 24, 2, "REMEDIO 1");  // Exibe a mensagem "REMEDIO 1" no display.
    ssd1306_show(&disp);  // Atualiza o display para mostrar a mensagem.
}

void Medicamento2() {  // Função que sinaliza a hora de tomar o segundo medicamento.

    gpio_put(LED_R_PIN, 0);  // Desliga o LED vermelho.
    gpio_put(LED_G_PIN, 1);  // Liga o LED verde.
    gpio_put(LED_B_PIN, 0);  // Desliga o LED azul.

    buzzer_melodia2();  // Toca a segunda melodia no buzzer para alertar o usuário.

    ssd1306_clear(&disp);  // Limpa o display OLED.
    ssd1306_draw_string(&disp, 20, 24, 2, "REMEDIO 2");  // Exibe a mensagem "REMEDIO 2" no display.
    ssd1306_show(&disp);  // Atualiza o display para mostrar a mensagem.
}

void Medicamento3() {  // Função que sinaliza a hora de tomar o terceiro medicamento.

    gpio_put(LED_R_PIN, 0);  // Desliga o LED vermelho.
    gpio_put(LED_G_PIN, 0);  // Desliga o LED verde.
    gpio_put(LED_B_PIN, 1);  // Liga o LED azul.

    buzzer_melodia3();  // Toca a terceira melodia no buzzer para alertar o usuário.

    ssd1306_clear(&disp);  // Limpa o display OLED.
    ssd1306_draw_string(&disp, 20, 24, 2, "REMEDIO 3");  // Exibe a mensagem "REMEDIO 3" no display.
    ssd1306_show(&disp);  // Atualiza o display para mostrar a mensagem.
}

void rotina_medicamento(void (*callback)(), _Bool *flag) {  
    medicamento_consumido = false;  // Inicializa a variável de controle como "não consumido"

    // Executa a rotina do medicamento (LEDs e buzzer)
    callback();  // Chama a função correspondente ao medicamento (ex: Medicamento1, Medicamento2, etc.)

    // Aguarda o botão ser pressionado para confirmar o consumo do medicamento
    while (!medicamento_consumido) {  
        if (detectar_borda_baixa(BTN_B, &btn_b_estado_anterior)) {  // Verifica se o botão foi pressionado
            medicamento_consumido = true;  // Atualiza a flag para indicar que o medicamento foi consumido
            *flag = true;  // Atualiza a flag específica do medicamento para evitar alertas repetidos

            // Captura o horário atual
            char horario_consumo[6];  // Formato HH:MM + terminador nulo
            snprintf(horario_consumo, sizeof(horario_consumo), "%02d:%02d", hora_atual, minuto_atual);

            // Envia o horário de consumo ao ThingSpeak
            printf("Enviando horário de consumo ao ThingSpeak: %s\n", horario_consumo);
            dns_gethostbyname(THINGSPEAK_HOST, &server_ip, dns_callback, NULL);  // Resolve o IP do ThingSpeak

            // Aguarda a conexão TCP ser estabelecida antes de enviar os dados
            while (!tcp_connected) {  
                sleep_ms(100);  // Aguarda 100ms antes de verificar novamente se a conexão foi estabelecida
            }

            enviar_dados_thingspeak(horario_consumo);  // Envia o horário de consumo para a plataforma ThingSpeak

            // Sai do loop após o botão ser pressionado
            break;
        }
        sleep_ms(100);  // Pequeno delay para evitar leituras excessivas do botão
    }

    // Apaga LEDs e buzzer ao finalizar a rotina
    gpio_put(LED_R_PIN, 0);  
    gpio_put(LED_G_PIN, 0);  
    gpio_put(LED_B_PIN, 0);  
    gpio_put(BUZZER_PIN, 0);  

    // Volta a exibir o relógio na tela
    printf("Retornando ao relógio...\n");
    mostrar_relogio();  // Exibe novamente o relógio no display OLED
}

int main() {
    inicializa();  // Configura hardware, Wi-Fi e display OLED

    while (true) {  // Loop infinito para manter o programa rodando continuamente
        if (definindo_horario) {  
            definir_horario();  // Entra no modo de ajuste de horário
        } else {
            mostrar_relogio_continuamente();  // Exibe o relógio e verifica alarmes
        }
    }
}