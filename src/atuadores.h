#ifndef ATUADORES_H
#define ATUADORES_H

// ============================================================
//  atuadores.h — Controle de Relés, LED e Buzzer
//  Responsabilidade: acionar as saídas físicas do sistema
//  com base no estado recebido do analisador.
// ============================================================

#include "config.h"

// ============================================================
//  inicializarAtuadores()
//  Deve ser chamada UMA VEZ no setup(), ANTES de tudo.
//  Define todos os pinos de saída e garante estado inicial
//  seguro (tudo desligado).
// ============================================================
void inicializarAtuadores() {
  pinMode(PINO_RELE1,  OUTPUT);
  pinMode(PINO_RELE2,  OUTPUT);
  pinMode(PINO_LED,    OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);

  // Estado inicial seguro: tudo desligado
  // Relés com lógica invertida: HIGH = desligado, LOW = ligado
  // (módulos de relé mais comuns são Active LOW)
  digitalWrite(PINO_RELE1,  HIGH);
  digitalWrite(PINO_RELE2,  HIGH);
  digitalWrite(PINO_LED,    LOW);
  digitalWrite(PINO_BUZZER, LOW);

  Serial.println("[ATUADOR] Pinos inicializados. Todos os atuadores: OFF.");
}

// ============================================================
//  setRele()
//  Liga ou desliga um relé específico.
//  Abstrai a lógica invertida dos módulos de relé Active LOW.
//  ligar=true → pino LOW (relé fecha o circuito)
//  ligar=false → pino HIGH (relé abre o circuito)
// ============================================================
void setRele(int pino, bool ligar) {
  // Módulos de relé Active LOW: LOW liga, HIGH desliga
  digitalWrite(pino, ligar ? LOW : HIGH);
}

// ============================================================
//  setBuzzer()
//  Liga ou desliga o buzzer de alerta.
// ============================================================
void setBuzzer(bool ligar) {
  digitalWrite(PINO_BUZZER, ligar ? HIGH : LOW);
}

// ============================================================
//  piscarLED()
//  Faz o LED piscar N vezes com um intervalo em ms.
//  Usado para o estado CRITICO.
//  ATENÇÃO: usa delay() intencionalmente aqui pois é chamado
//  apenas uma vez por ciclo, sem impacto na conexão MQTT.
// ============================================================
void piscarLED(int vezes, int intervaloMs) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(PINO_LED, HIGH);
    delay(intervaloMs);
    digitalWrite(PINO_LED, LOW);
    delay(intervaloMs);
  }
}

// ============================================================
//  acionarAtuadores()
//  Função principal deste módulo.
//  Recebe o estado (String) e aciona as saídas corretas.
//
//  MAPA DE ESTADOS:
//  CRITICO  → Relé1 ON + Relé2 ON + LED pisca vermelho + Buzzer ON
//  ATENCAO  → Relé1 ON + Relé2 OFF + LED ON + Buzzer OFF
//  CALOR    → Relé1 ON + Relé2 OFF + LED ON + Buzzer OFF
//  SECO     → Relé1 OFF + Relé2 ON + LED ON + Buzzer OFF
//  NORMAL   → Todos OFF + LED Verde (pisca 1x para confirmar)
// ============================================================
void acionarAtuadores(String estado) {

  if (estado == "CRITICO") {
    // Máxima intervenção: liga exaustor + umidificador + alerta sonoro
    setRele(PINO_RELE1, true);
    setRele(PINO_RELE2, true);
    setBuzzer(true);
    piscarLED(3, 200); // Pisca 3 vezes rápido — sinal visual de alerta
    Serial.println("[ATUADOR] CRITICO — Exaustor ON | Umidificador ON | Buzzer ON");
  }
  else if (estado == "ATENCAO") {
    // Ventilação moderada, sem buzzer
    setRele(PINO_RELE1, true);
    setRele(PINO_RELE2, false);
    setBuzzer(false);
    digitalWrite(PINO_LED, HIGH);
    Serial.println("[ATUADOR] ATENCAO — Exaustor ON | Umidificador OFF");
  }
  else if (estado == "CALOR") {
    // Ventilação forçada por temperatura
    setRele(PINO_RELE1, true);
    setRele(PINO_RELE2, false);
    setBuzzer(false);
    digitalWrite(PINO_LED, HIGH);
    Serial.println("[ATUADOR] CALOR — Exaustor ON | Umidificador OFF");
  }
  else if (estado == "SECO") {
    // Somente umidificador, sem ventilação
    setRele(PINO_RELE1, false);
    setRele(PINO_RELE2, true);
    setBuzzer(false);
    digitalWrite(PINO_LED, HIGH);
    Serial.println("[ATUADOR] SECO — Exaustor OFF | Umidificador ON");
  }
  else {
    // NORMAL: tudo desligado, LED apagado
    setRele(PINO_RELE1, false);
    setRele(PINO_RELE2, false);
    setBuzzer(false);
    digitalWrite(PINO_LED, LOW);
    Serial.println("[ATUADOR] NORMAL — Todos os atuadores OFF");
  }
}

#endif // ATUADORES_H
