/*
  Arquivo com as definições
*/

// REDE WIFI              - Mude para credienciais válidas se sua rede for diferente.
#define WIFI_SSID "motog04s"
#define WIFI_PASS "qwnse5dbf4phszi"

// SENSOR DE TEMPERATURA
#define TEMP_SENSOR_PIN 26

// Thingspeak
#define THINGSPEAK_HOST "api.thingspeak.com"
#define THINGSPEAK_PORT 80
#define API_KEY "2MNYL2MZ3NHEAR6P" // Chave de escrita do ThingSpeak

// Pinos dos LEDs RGB
#define LED_R_PIN 13
#define LED_G_PIN 12
#define LED_B_PIN 11

// Pino do Botão          - Botão A para enviar dados manualmente
#define BUTTON_PIN 5

// Pinos do I2C para o OLED
#define I2C_PORT i2c1
#define I2C_SDA 15
#define I2C_SCL 14
