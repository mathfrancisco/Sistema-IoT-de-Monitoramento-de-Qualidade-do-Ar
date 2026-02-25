#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

// ============================================================
//  mqtt_client.h — Comunicação Wi-Fi e MQTT
//  Responsabilidade: gerenciar a conexão com a rede e o broker,
//  montar o payload JSON e publicar os dados no dashboard.
// ============================================================

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

// ------------------------------------------------------------
//  Instâncias necessárias para Wi-Fi e MQTT
// ------------------------------------------------------------
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

// ============================================================
//  conectarWiFi()
//  Tenta conectar ao Wi-Fi definido em config.h.
//  Bloqueia o setup() até obter conexão (máx. TENTATIVAS_WIFI).
// ============================================================
void conectarWiFi() {
  Serial.print("[WIFI] Conectando a: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < TENTATIVAS_WIFI) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("[WIFI] Conectado! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    // Se não conectou, exibe aviso mas deixa o sistema continuar
    // (pode funcionar localmente mesmo sem internet)
    Serial.println();
    Serial.println("[WIFI] AVISO: Nao foi possivel conectar ao Wi-Fi.");
    Serial.println("[WIFI] O sistema continuara sem publicar no dashboard.");
  }
}

// ============================================================
//  conectarMQTT()
//  Tenta conectar ao broker MQTT com as credenciais de config.h.
//  Retorna TRUE se bem-sucedido, FALSE se falhou.
// ============================================================
bool conectarMQTT() {
  Serial.print("[MQTT] Conectando ao broker: ");
  Serial.println(MQTT_SERVER);

  // Tenta autenticar com client_id, usuário e senha/token
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("[MQTT] Conectado ao broker com sucesso!");
    return true;
  } else {
    Serial.print("[MQTT] Falha na conexao. Codigo de erro: ");
    Serial.println(mqttClient.state());
    // Códigos comuns: -2=sem servidor, -4=timeout, 5=credenciais inválidas
    return false;
  }
}

// ============================================================
//  inicializarMQTT()
//  Configura o cliente MQTT com servidor e porta.
//  Deve ser chamada UMA VEZ no setup(), após conectarWiFi().
// ============================================================
void inicializarMQTT() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  // Aumenta o buffer para suportar o payload JSON completo
  mqttClient.setBufferSize(512);
  Serial.println("[MQTT] Cliente configurado.");
}

// ============================================================
//  verificarConexaoMQTT()
//  Verifica se o MQTT ainda está conectado.
//  Se não estiver, tenta reconectar automaticamente.
//  Deve ser chamada NO INÍCIO de cada iteração do loop().
// ============================================================
void verificarConexaoMQTT() {
  // Primeiro verifica o Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] Conexao perdida. Reconectando...");
    conectarWiFi();
  }

  // Depois verifica o MQTT
  if (!mqttClient.connected()) {
    Serial.println("[MQTT] Conexao perdida. Reconectando...");
    conectarMQTT();
  }
}

// ============================================================
//  mqttLoop()
//  Wrapper para client.loop() — OBRIGATÓRIO em cada iteração.
//  Mantém a conexão MQTT viva e processa mensagens internas.
//  Sem isso, o broker desconecta o cliente por inatividade.
// ============================================================
void mqttLoop() {
  mqttClient.loop();
}

// ============================================================
//  montarPayloadJSON()
//  Recebe os dados lidos e monta uma String JSON formatada
//  pronta para ser publicada no tópico MQTT.
//
//  Formato de saída:
//  {"temperatura":27.5,"umidade":38.2,"co2":1120,"status":"ATENCAO","local":"escritorio_mf"}
// ============================================================
String montarPayloadJSON(float temperatura, float umidade, int co2, String status) {
  // StaticJsonDocument: alocação em stack (mais eficiente no ESP32)
  // 256 bytes é suficiente para este payload
  StaticJsonDocument<256> doc;

  doc["temperatura"] = serialized(String(temperatura, 1)); // 1 casa decimal
  doc["umidade"]     = serialized(String(umidade, 1));
  doc["co2"]         = co2;
  doc["status"]      = status;
  doc["local"]       = "escritorio_mf";
  doc["uptime_ms"]   = millis(); // Tempo desde o boot, útil para debug

  String payload;
  serializeJson(doc, payload);

  return payload;
}

// ============================================================
//  publicarDados()
//  Publica o payload JSON no tópico MQTT definido em config.h.
//  Retorna TRUE se a publicação foi bem-sucedida.
// ============================================================
bool publicarDados(String payload) {
  if (!mqttClient.connected()) {
    Serial.println("[MQTT] Nao publicado: sem conexao com o broker.");
    return false;
  }

  bool sucesso = mqttClient.publish(MQTT_TOPIC, payload.c_str());

  if (sucesso) {
    Serial.print("[MQTT] Publicado em '");
    Serial.print(MQTT_TOPIC);
    Serial.print("': ");
    Serial.println(payload);
  } else {
    Serial.println("[MQTT] ERRO: Falha ao publicar. Tentando no proximo ciclo.");
  }

  return sucesso;
}

#endif // MQTT_CLIENT_H
