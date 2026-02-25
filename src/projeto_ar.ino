// ============================================================
//
//   ███╗   ███╗███████╗    ████████╗███████╗ ██████╗
//   ████╗ ████║██╔════╝    ╚══██╔══╝██╔════╝██╔════╝
//   ██╔████╔██║█████╗         ██║   █████╗  ██║
//   ██║╚██╔╝██║██╔══╝         ██║   ██╔══╝  ██║
//   ██║ ╚═╝ ██║██║            ██║   ███████╗╚██████╗
//   ╚═╝     ╚═╝╚═╝            ╚═╝   ╚══════╝ ╚═════╝
//
//   Sistema IoT de Monitoramento de Qualidade do Ar
//   MF Tecnologia e Sistemas — Projeto Integrador 2025
//   Tópico 2: "Como é o ar que você respira?"
//   Alinhado ao ODS 3 — Saúde e Bem-Estar (ONU)
//
//   Hardware: ESP32 DevKit V1
//   Sensores: DHT22 (Temp/Umidade) + MQ-135 (CO2)
//   Saídas:   2x Relé + LED + Buzzer
//   Protocolo: MQTT → TagoIO Dashboard
//
//   Arquitetura modular:
//   - config.h      → Constantes e credenciais
//   - sensores.h    → Leitura do DHT22 e MQ-135
//   - atuadores.h   → Controle de relés, LED e buzzer
//   - mqtt_client.h → Wi-Fi, MQTT e publicação JSON
//
// ============================================================

// --- Bibliotecas externas (instalar no Arduino IDE / Wokwi) ---
#include <DHT.h>            // Adafruit DHT sensor library
#include <PubSubClient.h>   // knolleary/pubsubclient
#include <ArduinoJson.h>    // bblanchon/ArduinoJson
#include <WiFi.h>           // Nativa do ESP32

// --- Módulos do projeto (ordem importa: config primeiro) ---
#include "config.h"
#include "sensores.h"
#include "atuadores.h"
#include "mqtt_client.h"

// ============================================================
//  Variáveis Globais do Orquestrador
//  Armazenam os dados entre as fases do ciclo principal.
// ============================================================
float        temperatura   = 0.0;  // Último valor de temperatura (°C)
float        umidade       = 0.0;  // Último valor de umidade (%)
int          co2           = 0;    // Último valor de CO2 (ppm)
String       estadoAtual   = "INICIALIZANDO"; // Estado atual do sistema
unsigned long ultimaLeitura = 0;   // Timestamp da última leitura (millis)

// ============================================================
//  setup()
//  Executado UMA VEZ ao ligar ou resetar o ESP32.
//  Inicializa todos os módulos na ordem correta.
// ============================================================
void setup() {
  // 1. Inicia a comunicação Serial para logs de debug
  Serial.begin(115200);
  delay(500); // Aguarda o Serial estabilizar
  Serial.println();
  Serial.println("============================================");
  Serial.println("  MF Tecnologia — Monitor de Qualidade do Ar");
  Serial.println("  Iniciando sistema...");
  Serial.println("============================================");

  // 2. Inicializa os pinos de saída e garante estado seguro
  //    (tudo desligado antes de qualquer leitura)
  inicializarAtuadores();

  // 3. Inicializa os sensores DHT22 e registra tempo para warm-up
  inicializarSensores();

  // 4. Conecta ao Wi-Fi (bloqueia até conectar ou esgotar tentativas)
  conectarWiFi();

  // 5. Configura o cliente MQTT com servidor e porta
  inicializarMQTT();

  // 6. Tenta conectar ao broker MQTT
  conectarMQTT();

  // 7. Sistema pronto — acende o LED para confirmação visual
  digitalWrite(PINO_LED, HIGH);
  delay(1000);
  digitalWrite(PINO_LED, LOW);

  Serial.println("============================================");
  Serial.println("  Sistema pronto! Iniciando monitoramento...");
  Serial.println("  Aguardando warm-up do MQ-135 (30 segundos)");
  Serial.println("============================================");
}

// ============================================================
//  loop()
//  Executado CONTINUAMENTE após o setup().
//  Segue exatamente as 5 fases do fluxograma.
//
//  IMPORTANTE — Por que não usamos delay(5000)?
//  Usar delay() travaria o processador e derrubaria a conexão
//  MQTT. Em vez disso, usamos millis() para verificar se o
//  intervalo de 5s passou, mantendo o loop() não-bloqueante.
// ============================================================
void loop() {

  // ── FASE 5 (verificação contínua) ─────────────────────────
  // Mantém a conexão MQTT viva e reconecta se necessário.
  // Deve ser chamado em TODA iteração do loop, não só a cada 5s.
  verificarConexaoMQTT();
  mqttLoop();

  // ── Controle de tempo não-bloqueante ──────────────────────
  // Verifica se já passaram INTERVALO_LEITURA ms desde a última leitura.
  unsigned long agora = millis();
  if (agora - ultimaLeitura < INTERVALO_LEITURA) {
    return; // Ainda não é hora — retorna e continua o loop
  }

  // Atualiza o timestamp para o próximo ciclo
  ultimaLeitura = agora;

  Serial.println();
  Serial.println("---- Novo Ciclo de Monitoramento ----");

  // ── FASE 2 — Verificar warm-up do MQ-135 ──────────────────
  if (!sensorAquecido()) {
    unsigned long tempoRestante = (WARMUP_MQ135 - (agora - tempoInicioSistema)) / 1000;
    Serial.print("[SENSOR] MQ-135 aquecendo... faltam ~");
    Serial.print(tempoRestante);
    Serial.println(" segundos.");
    digitalWrite(PINO_LED, !digitalRead(PINO_LED)); // Pisca LED durante warm-up
    return; // Aguarda warm-up antes de ler
  }

  // ── FASE 2 — Leitura do DHT22 ─────────────────────────────
  bool leituraOK = lerDHT22(temperatura, umidade);

  if (!leituraOK) {
    // Leitura inválida: não aborta o sistema, tenta no próximo ciclo
    Serial.println("[AVISO] Ciclo interrompido por leitura invalida do DHT22.");
    return;
  }

  // ── FASE 2 — Leitura do MQ-135 ────────────────────────────
  co2 = lerMQ135();

  // ── FASE 3 — Análise e Decisão ────────────────────────────
  estadoAtual = analisarQualidade(temperatura, umidade, co2);

  // ── FASE 4 — Atuação: relés, LED e buzzer ─────────────────
  acionarAtuadores(estadoAtual);

  // ── FASE 5 — Montar e Publicar payload JSON via MQTT ──────
  String payload = montarPayloadJSON(temperatura, umidade, co2, estadoAtual);
  publicarDados(payload);

  Serial.println("---- Fim do Ciclo ----");
}
