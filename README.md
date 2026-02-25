# 🌬️ Sistema IoT de Monitoramento de Qualidade do Ar
### MF Tecnologia e Sistemas — Projeto Integrador · Tópico 2: *"Como é o ar que você respira?"*

<div align="center">

![Status](https://img.shields.io/badge/Status-Em%20Desenvolvimento-yellow)
![Plataforma](https://img.shields.io/badge/Plataforma-ESP32-blue)
![Protocolo](https://img.shields.io/badge/Protocolo-MQTT-green)
![ODS](https://img.shields.io/badge/ODS%203-Saúde%20e%20Bem--Estar-brightgreen)
![Simulador](https://img.shields.io/badge/Simulador-Wokwi-orange)

</div>

---

## 📋 Sobre o Projeto

Programadores da **MF Tecnologia** passam 8+ horas por dia em ambientes fechados. Níveis elevados de CO₂ e baixa umidade causam **fadiga mental, dor de cabeça e queda direta na produtividade** — um problema real, silencioso e mensurável.

Este projeto utiliza um microcontrolador **ESP32** conectado a sensores de temperatura, umidade e qualidade do ar para **monitorar o ambiente em tempo real**, acionar automaticamente ventiladores e umidificadores quando os limites são ultrapassados, e enviar alertas para o time via **dashboard e Slack**.

**Conexão com o ODS 3 — Saúde e Bem-Estar (ONU):** Ambientes de trabalho com ar saudável reduzem o absenteísmo, aumentam a produtividade e protegem a saúde dos colaboradores — contribuindo diretamente com a meta 3.d da Agenda 2030.

---

## 🏗️ Arquitetura de Hardware

| Componente | Modelo | Função no Sistema |
|---|---|---|
| Microcontrolador | ESP32 DevKit V1 | Processamento central, Wi-Fi e MQTT |
| Sensor de Temp/Umidade | DHT22 | Mede temperatura (°C) e umidade relativa (%) |
| Sensor de Qualidade do Ar | MQ-135 | Mede concentração de CO₂ e gases (ppm) |
| Módulo Relé x2 | Relé 5V | Aciona exaustor e umidificador automaticamente |
| LED de Status | LED RGB / Onboard | Indicador visual do estado do sistema |
| Buzzer | Buzzer Passivo | Alerta sonoro em estado crítico |

### Diagrama de Pinagem (ESP32)

```
ESP32          Componente
─────────────────────────────
GPIO 4    →   DHT22 (Data)
GPIO 34   →   MQ-135 (Analog OUT)
GPIO 26   →   Relé 1 — Exaustor
GPIO 27   →   Relé 2 — Umidificador
GPIO 2    →   LED de Status (onboard)
GPIO 25   →   Buzzer
3.3V/GND  →   Alimentação dos sensores
```

---

## ⚙️ Lógica de Automação

O sistema opera em um ciclo contínuo de **5 segundos** com 5 fases bem definidas:

```
[Fase 1] Inicialização → Wi-Fi + MQTT
[Fase 2] Leitura       → DHT22 + MQ-135 (com validação e warm-up)
[Fase 3] Análise       → Comparação com limites de segurança
[Fase 4] Atuação       → Relés + LED + Buzzer conforme o estado
[Fase 5] Publicação    → Payload JSON via MQTT para o dashboard
```

### Tabela de Limites e Respostas

| Parâmetro | ✅ Normal | ⚠️ Atenção | 🚨 Crítico | Ação Automática |
|---|---|---|---|---|
| CO₂ (ppm) | < 1000 | 1000 – 1500 | > 1500 | Ligar exaustor + alerta Slack |
| Temperatura (°C) | < 26°C | 26 – 28°C | > 28°C | Ventilação forçada |
| Umidade Relativa (%) | 40 – 60% | 30 – 40% | < 30% | Ligar umidificador |

> Limites baseados nas normas **ASHRAE 62.1** (qualidade do ar interior) e **NR-17** (ergonomia em escritórios).

---

## 🔗 Links do Projeto

| Recurso | Link |
|---|---|
| 🖥️ Simulador Wokwi | `[INSERIR LINK DO WOKWI APÓS CRIAR]` |
| 📊 Dashboard (TagoIO/Adafruit) | `[INSERIR LINK DO DASHBOARD]` |
| 📐 Fluxograma (LucidChart) | `[(https://lucid.app/lucidchart/eee9f12f-1672-403d-800d-662465c7eae6/edit?viewport_loc=-5939%2C-8493%2C16535%2C8555%2C0_0&invitationId=inv_8c78659e-f4a6-4b0b-914e-472c49d85950)]` |

---

## 🚀 Como Executar a Simulação (passo a passo)

### Pré-requisitos
- Conta gratuita no [Wokwi](https://wokwi.com) *(não requer instalação)*
- Conta gratuita no [TagoIO](https://tago.io) ou [Adafruit IO](https://io.adafruit.com) para o dashboard

### Passo a Passo

**1. Abrir o simulador**
```
Acesse o link do Wokwi na tabela acima → clique em "Play" (▶)
```

**2. Configurar as credenciais MQTT**

No arquivo `config.h`, altere as variáveis com suas credenciais:
```cpp
// Wi-Fi
#define WIFI_SSID     "SUA_REDE"
#define WIFI_PASSWORD "SUA_SENHA"

// MQTT (TagoIO ou Adafruit IO)
#define MQTT_SERVER   "mqtt.tago.io"
#define MQTT_PORT     1883
#define MQTT_USER     "SEU_DEVICE_TOKEN"
#define MQTT_TOKEN    "SEU_TOKEN"
#define MQTT_TOPIC    "/mftecnologia/escritorio/ar"
```

**3. Observar o funcionamento**
- Abra o **Serial Monitor** no Wokwi para ver os logs em tempo real
- Gire os potenciômetros dos sensores para simular CO₂ alto ou temperatura elevada
- Os relés devem acionar automaticamente quando os limites forem ultrapassados
- Acesse o dashboard para ver os dados chegando via MQTT

**4. Verificar os estados possíveis**

| Estado | LED | Buzzer | Relé 1 | Relé 2 |
|---|---|---|---|---|
| NORMAL | Verde fixo | OFF | OFF | OFF |
| ATENÇÃO | Amarelo | OFF | ON | OFF |
| CALOR | Laranja | OFF | ON | OFF |
| SECO | Azul | OFF | OFF | ON |
| CRÍTICO | Vermelho piscando | ON | ON | ON |

---

## 📁 Estrutura do Repositório

```
📦 sistema-monitoramento-ar/
├── 📄 README.md                  ← Este arquivo
├── 📄 diagram.json               ← Diagrama de circuito do Wokwi
├── 📂 src/
│   ├── 🔧 projeto_ar.ino         ← Arquivo principal (setup + loop)
│   ├── 🔧 config.h               ← Constantes, pinos e credenciais
│   ├── 🔧 sensores.h             ← Funções de leitura DHT22 e MQ-135
│   ├── 🔧 atuadores.h            ← Controle de relés, LED e buzzer
│   └── 🔧 mqtt_client.h          ← Conexão, publicação e reconexão MQTT
├── 📂 docs/
│   ├── 📐 fluxograma_v2.svg      ← Fluxograma exportado (LucidChart)
│   └── 📄 planejamento.md        ← Planejamento de arquitetura do código
└── 📂 assets/
    └── 🖼️  circuito_wokwi.png    ← Print do circuito montado
```

---

## 🧠 Pensamento Computacional Aplicado

| Pilar | Como foi aplicado neste projeto |
|---|---|
| **Decomposição** | O problema foi dividido em 5 fases independentes: Inicialização, Leitura, Análise, Atuação e Publicação |
| **Abstração** | Os dados físicos (temperatura, umidade, ppm) são abstraídos em um payload JSON compacto para o dashboard |
| **Reconhecimento de Padrões** | O sistema identifica padrões de degradação do ar em horários de pico, registrando eventos por tipo |
| **Algoritmo** | A lógica de decisão opera com múltiplos níveis (Normal → Atenção → Crítico), garantindo respostas proporcionais |

---

## 📚 Bibliotecas Utilizadas

```cpp
#include <WiFi.h>            // Conexão Wi-Fi nativa do ESP32
#include <PubSubClient.h>    // Comunicação MQTT
#include <DHT.h>             // Leitura do sensor DHT22
#include <Adafruit_Sensor.h> // Dependência da biblioteca DHT
#include <ArduinoJson.h>     // Montagem do payload JSON
```

---

## 👨‍💻 Equipe

| Nome | Função |
|---|---|
| `[SEU NOME]` | Desenvolvedor IoT — MF Tecnologia e Sistemas |

---

## 📄 Licença

Este projeto foi desenvolvido para fins acadêmicos no contexto do **Projeto Integrador**, com tema alinhado ao **ODS 3 — Saúde e Bem-Estar** da Agenda 2030 da ONU.
o Integrador · 2025</sub>
</div>
