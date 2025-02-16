/*
Projeto: Desenvolvimento de uma Solução IoT para Monitoramento de Temperatura Utilizando a Placa BitDogLab
Autor: Diego Gomes
Data: 16/02/2025
Gráfico dos dados:  https://thingspeak.mathworks.com/channels/2841648

Descrição: Utilizado para monitorar a temperatura ambiente e controlar 
    a iluminação em um sistema de automação residencial. Os dados de temperatura
    são coletados por um sensor conectado a um microcontrolador BitDogLab e 
    enviados para o ThingSpeak a cada 15 segundos.

    O sistema também possui um botão para enviar os dados manualmente e um
    display OLED para exibir a temperatura e o status da conexão Wi-Fi.

    O LED RGB muda de cor com base na temperatura ambiente:
    - Vermelho: Temperatura acima de 30°C
    - Verde: Temperatura entre 20°C e 30°C
    - Azul: Temperatura abaixo de 20°C

    O código foi desenvolvido utilizando a biblioteca lwIP para comunicação
    com o ThingSpeak e a biblioteca ssd1306 para o display OLED.

Oberservações: Antes de compilar o código, é necessário mudar as credenciais
    da rede Wi-Fi e a chave de escrita(Caso contrário, mantenha as informações 
    que estão no arquivo) do ThingSpeak no arquivo parametros.h.
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/init.h"
#include "ssd1306.h"
#include "parametros.h"

struct tcp_pcb *tcp_client_pcb;
ip_addr_t server_ip;
ssd1306_t disp;
volatile bool timer_fired = false;

// Função para inicializar o display OLED
void init_oled() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);
    ssd1306_clear(&disp);
}

// Função para calcular a posição X e Y para centralizar o texto
void calculate_position(const char *msg, int display_width, int display_height, int font_width, int font_height, int *x, int *y) {
    int msg_length = strlen(msg);
    int text_width = msg_length * font_width;
    int text_height = font_height;
    *x = (display_width - text_width) / 2;
    *y = (display_height - text_height) / 2;
    if (*x < 0) *x = 0;  // Garante que a posição X não seja negativa
    if (*y < 0) *y = 0;  // Garante que a posição Y não seja negativa
}

// Função para exibir mensagens no display OLED
void display_message(const char *msg) {
    ssd1306_clear(&disp);
    int x_position, y_position;
    calculate_position(msg, 128, 64, 6, 8, &x_position, &y_position);  // 128x64 é a resolução do display, 6x8 é o tamanho do caractere
    ssd1306_draw_string(&disp, x_position, y_position, 1, msg);  // Ajuste do tamanho do texto para 1
    ssd1306_show(&disp);
}

// Função para ler a temperatura do sensor interno (ADC4)
float read_temperature() {
    adc_select_input(4);  // Seleciona o canal ADC4
    uint16_t raw = adc_read();  // Lê o valor bruto do ADC
    float voltage = raw * 3.3f / (1 << 12);  // Converte o valor bruto para tensão (3.3V, 12 bits)
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;  // Converte a tensão para temperatura em Celsius
    temperature = 0.527f * ( temperature - 32.0f ); // Ajuste Conversão para °C
    return temperature;
}

// Função para atualizar os LEDs RGB com base na temperatura
void update_leds(float temperature) {
    if (temperature > 30.0) {
        gpio_put(LED_R_PIN, 1);
        gpio_put(LED_G_PIN, 0);
        gpio_put(LED_B_PIN, 0);
    } else if (temperature > 20.0) {
        gpio_put(LED_R_PIN, 0);
        gpio_put(LED_G_PIN, 1);
        gpio_put(LED_B_PIN, 0);
    } else {
        gpio_put(LED_R_PIN, 0);
        gpio_put(LED_G_PIN, 0);
        gpio_put(LED_B_PIN, 1);
    }
}

// Função para conectar ao Wi-Fi com efeito de "loading"
void connect_to_wifi() {
    int retries = 5;
    int loading_step = 0;
    char loading_msg[32];

    while (retries > 0) {
        snprintf(loading_msg, sizeof(loading_msg), "Conectando ao Wi-Fi");
        for (int i = 0; i < loading_step; i++) {
            strcat(loading_msg, "*");
        }
        display_message(loading_msg);

        if (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK) == 0) {
            display_message("Wi-Fi conectado!");
            return;
        }

        retries--;
        loading_step = (loading_step + 1) % 4;  // Alterna entre 0, 1, 2, 3
        sleep_ms(500);  // Espera 500 ms antes de tentar novamente
    }

    display_message("Falha ao conectar ao Wi-Fi");
}

// Callback quando recebe resposta do ThingSpeak
static err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }
    display_message("Dados enviados!");
    pbuf_free(p);
    return ERR_OK;
}

// Callback quando a conexão TCP é estabelecida
static err_t http_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        display_message("Erro na conexão TCP");
        return err;
    }

    float temperature = read_temperature();  // Lê a temperatura
    char request[256];
    snprintf(request, sizeof(request),
        "GET /update?api_key=%s&field1=%.2f HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        API_KEY, temperature, THINGSPEAK_HOST);

    tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    tcp_recv(tpcb, http_recv_callback);

    return ERR_OK;
}

// Resolver DNS e conectar ao servidor
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr) {
        display_message("DNS resolvido");
        tcp_client_pcb = tcp_new();
        tcp_connect(tcp_client_pcb, ipaddr, THINGSPEAK_PORT, http_connected_callback);
    } else {
        display_message("DNS não resolvido");
    }
}

// Callback do temporizador
bool repeating_timer_callback(struct repeating_timer *t) {
    timer_fired = true;
    return true;
}

bool modo_automatico = true;

int main() {
    stdio_init_all();
    adc_init();  // Inicializa o ADC para ler o sensor de temperatura
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    init_oled();  // Inicializa o display OLED

    if (cyw43_arch_init()) {
        display_message("Falha ao iniciar Wi-Fi");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    connect_to_wifi();  // Tenta conectar ao Wi-Fi

    // Configura o temporizador para disparar a cada 15 segundos
    struct repeating_timer timer;
    add_repeating_timer_ms(15000, repeating_timer_callback, NULL, &timer);

    while (true) {
        if (gpio_get(BUTTON_PIN) == 0) {  // Verifica se o botão foi pressionado
            sleep_ms(200);  // Debounce
            float temperature = read_temperature();
            char temp_str[32];
            snprintf(temp_str, sizeof(temp_str), "Temp: %.2f C", temperature);
            display_message(temp_str);  // Exibe a temperatura no OLED
            dns_gethostbyname(THINGSPEAK_HOST, &server_ip, dns_callback, NULL);  // Envia dados manualmente
        }

        if (timer_fired) {
            timer_fired = false;
            float temperature = read_temperature();
            update_leds(temperature);
            char temp_str[32];
            snprintf(temp_str, sizeof(temp_str), "Temp: %.2f C", temperature);
            display_message(temp_str);  // Exibe a temperatura no OLED
            dns_gethostbyname(THINGSPEAK_HOST, &server_ip, dns_callback, NULL);  // Envia dados automaticamente
        }
    }

    return 0;
}