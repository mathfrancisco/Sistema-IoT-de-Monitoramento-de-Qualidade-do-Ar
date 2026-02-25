#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
//  config.h — Configurações Centrais do Sistema
//  MF Tecnologia e Sistemas · Projeto Integrador 2025
//  Altere SOMENTE este arquivo para adaptar o projeto.
// ============================================================

// ------------------------------------------------------------
//  Wi-Fi
// ------------------------------------------------------------
#define WIFI_SSID     "Wokwi-GUEST"   // SSID da rede (no Wokwi use "Wokwi-GUEST")
#define WIFI_PASSWORD ""              // Senha (Wokwi-GUEST não tem senha)

// ------------------------------------------------------------
//  MQTT — TagoIO
//  Crie um Device em app.tago.io e cole o token abaixo.
// ------------------------------------------------------------
#define MQTT_SERVER   "broker.hivemq.com"
#define MQTT_PORT     1883
#define MQTT_USER     ""
#define MQTT_PASSWORD ""
#define MQTT_TOPIC    "mftecnologia/escritorio/ar"
#define MQTT_CLIENT_ID "mf-dashboard-monitor"

// ------------------------------------------------------------
//  Pinos do ESP32
// ------------------------------------------------------------
#define PINO_DHT22     4    // GPIO 4  — Sensor DHT22 (Data)
#define PINO_MQ135     34   // GPIO 34 — Sensor MQ-135 (Analógico, somente leitura)
#define PINO_RELE1     26   // GPIO 26 — Relé 1: Exaustor
#define PINO_RELE2     27   // GPIO 27 — Relé 2: Umidificador
#define PINO_LED       2    // GPIO 2  — LED de Status (onboard)
#define PINO_BUZZER    25   // GPIO 25 — Buzzer de Alerta

// ------------------------------------------------------------
//  Sensor DHT
// ------------------------------------------------------------
#define TIPO_DHT  DHT22     // Modelo do sensor (DHT11 ou DHT22)

// ------------------------------------------------------------
//  Limites de Alerta
//  Baseados nas normas ASHRAE 62.1 e NR-17
// ------------------------------------------------------------
#define CO2_ATENCAO       1000   // ppm — Nível de atenção
#define CO2_CRITICO       1500   // ppm — Nível crítico
#define TEMP_LIMITE       28.0   // °C  — Temperatura máxima tolerada
#define UMIDADE_MINIMA    40.0   // %   — Umidade mínima saudável

// ------------------------------------------------------------
//  Temporização
// ------------------------------------------------------------
#define INTERVALO_LEITURA   5000   // ms — Ciclo de leitura (5 segundos)
#define WARMUP_MQ135       30000   // ms — Tempo de aquecimento do MQ-135
#define TENTATIVAS_WIFI       20   // Número máximo de tentativas de conexão Wi-Fi

// ------------------------------------------------------------
//  Mapeamento do MQ-135 (potenciômetro no Wokwi)
//  O valor analógico do ESP32 vai de 0 a 4095 (12 bits).
//  Mapeamos para a faixa realista de CO2: 400 a 2000 ppm.
// ------------------------------------------------------------
#define MQ135_PPM_MIN    400
#define MQ135_PPM_MAX   2000
#define ADC_MIN            0
#define ADC_MAX         4095

#endif // CONFIG_H
