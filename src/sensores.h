#ifndef SENSORES_H
#define SENSORES_H

// ============================================================
//  sensores.h — Leitura e Validação dos Sensores
//  Responsabilidade: abstrair a leitura física dos sensores,
//  tratando erros antes de devolver os dados ao orquestrador.
// ============================================================

#include <DHT.h>
#include "config.h"

// ------------------------------------------------------------
//  Instância global do sensor DHT22
// ------------------------------------------------------------
DHT dht(PINO_DHT22, TIPO_DHT);

// ------------------------------------------------------------
//  Variável de controle do warm-up do MQ-135
//  Registra o momento em que o sistema iniciou.
// ------------------------------------------------------------
unsigned long tempoInicioSistema = 0;

// ============================================================
//  inicializarSensores()
//  Deve ser chamada UMA VEZ no setup().
//  Inicia o DHT22 e registra o tempo de início para o warm-up.
// ============================================================
void inicializarSensores() {
  dht.begin();
  tempoInicioSistema = millis();
  Serial.println("[SENSOR] DHT22 inicializado.");
  Serial.println("[SENSOR] Aguardando aquecimento do MQ-135 (30s)...");
}

// ============================================================
//  sensorAquecido()
//  Retorna TRUE se o MQ-135 já passou pelo tempo de warm-up.
//  O sensor precisa de ~30s para estabilizar as leituras.
// ============================================================
bool sensorAquecido() {
  return (millis() - tempoInicioSistema) >= WARMUP_MQ135;
}

// ============================================================
//  lerDHT22()
//  Lê temperatura e umidade do sensor DHT22.
//  Parâmetros por referência: os valores são preenchidos aqui.
//  Retorna TRUE se a leitura foi válida, FALSE se houve erro.
// ============================================================
bool lerDHT22(float &temperatura, float &umidade) {
  // Realiza a leitura dos dois valores
  float t = dht.readTemperature();
  float u = dht.readHumidity();

  // Verifica se algum valor retornou NaN (leitura inválida)
  if (isnan(t) || isnan(u)) {
    Serial.println("[ERRO] Falha na leitura do DHT22! Verifique a conexão.");
    return false;
  }

  // Leitura válida — preenche as variáveis do chamador
  temperatura = t;
  umidade     = u;

  Serial.print("[DHT22] Temperatura: ");
  Serial.print(temperatura, 1);
  Serial.print(" °C | Umidade: ");
  Serial.print(umidade, 1);
  Serial.println(" %");

  return true;
}

// ============================================================
//  lerMQ135()
//  Lê o valor analógico do pino do MQ-135 (ou potenciômetro
//  no Wokwi) e converte para uma estimativa em ppm de CO2.
//  O ESP32 tem ADC de 12 bits: leitura vai de 0 a 4095.
// ============================================================
int lerMQ135() {
  // Leitura bruta do ADC (0–4095)
  int leituraADC = analogRead(PINO_MQ135);

  // Mapeia o valor ADC para a faixa de ppm definida em config.h
  // map(valor, de_min, de_max, para_min, para_max)
  int ppm = map(leituraADC, ADC_MIN, ADC_MAX, MQ135_PPM_MIN, MQ135_PPM_MAX);

  Serial.print("[MQ135] Leitura ADC: ");
  Serial.print(leituraADC);
  Serial.print(" | CO2 estimado: ");
  Serial.print(ppm);
  Serial.println(" ppm");

  return ppm;
}

// ============================================================
//  analisarQualidade()
//  Recebe os três valores lidos e retorna uma String com o
//  estado atual do ambiente. Segue a prioridade:
//  CRITICO > ATENCAO > CALOR > SECO > NORMAL
// ============================================================
String analisarQualidade(float temperatura, float umidade, int co2) {

  // --- Nível 1: CO2 Crítico (prioridade máxima) ---
  if (co2 > CO2_CRITICO) {
    Serial.println("[ANALISE] Estado: CRITICO — CO2 acima de 1500 ppm!");
    return "CRITICO";
  }

  // --- Nível 2: CO2 em Atenção ---
  if (co2 > CO2_ATENCAO) {
    Serial.println("[ANALISE] Estado: ATENCAO — CO2 entre 1000 e 1500 ppm.");
    return "ATENCAO";
  }

  // --- Nível 3: Temperatura Alta ---
  if (temperatura > TEMP_LIMITE) {
    Serial.println("[ANALISE] Estado: CALOR — Temperatura acima de 28°C.");
    return "CALOR";
  }

  // --- Nível 4: Ar Muito Seco ---
  if (umidade < UMIDADE_MINIMA) {
    Serial.println("[ANALISE] Estado: SECO — Umidade abaixo de 40%.");
    return "SECO";
  }

  // --- Padrão: Tudo dentro dos limites ---
  Serial.println("[ANALISE] Estado: NORMAL — Qualidade do ar saudável.");
  return "NORMAL";
}

#endif // SENSORES_H
