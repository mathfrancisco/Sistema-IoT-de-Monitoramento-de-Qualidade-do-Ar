# 📐 Planejamento de Arquitetura do Código
### Sistema IoT — Monitoramento de Qualidade do Ar | MF Tecnologia e Sistemas

---

## 1. Visão Geral da Arquitetura

O sistema foi planejado seguindo o princípio de **Responsabilidade Única**: cada módulo cuida de uma única responsabilidade, facilitando testes, manutenção e a leitura do código por terceiros (incluindo a banca avaliadora).

```
┌─────────────────────────────────────────────────────┐
│                   projeto_ar.ino                     │
│            (Orquestrador — setup + loop)             │
└────────┬──────────┬──────────┬──────────┬───────────┘
         │          │          │          │
    config.h   sensores.h  atuadores.h  mqtt_client.h
  (Constantes) (Leitura)   (Saídas)    (Comunicação)
```

---

## 2. Descrição de Cada Módulo

### `config.h` — Configurações e Constantes
**Responsabilidade:** Centralizar todas as configurações em um único lugar. Qualquer mudança de credencial ou limite de alerta é feita **somente aqui**, sem precisar mexer no restante do código.

**Conteúdo planejado:**
- Credenciais Wi-Fi (SSID e senha)
- Credenciais MQTT (servidor, porta, usuário, token, tópico)
- Definição dos pinos de cada componente
- Limites de alerta (CO2_ATENCAO, CO2_CRITICO, TEMP_LIMITE, UMIDADE_MINIMA)
- Constantes de tempo (INTERVALO_LEITURA = 5000ms, WARMUP_MQ135 = 30000ms)
- Tipo do sensor DHT (DHT22)

---

### `sensores.h` — Leitura dos Sensores
**Responsabilidade:** Abstrair a leitura física dos sensores, tratando erros antes de devolver os dados para o restante do sistema.

**Funções planejadas:**

| Função | Retorno | Descrição |
|---|---|---|
| `inicializarSensores()` | void | Configura o DHT22 e registra o tempo de início para o warm-up do MQ-135 |
| `lerDHT22(float &temp, float &umi)` | bool | Lê temperatura e umidade. Retorna `false` se a leitura for NaN (inválida) |
| `lerMQ135()` | int | Lê o valor analógico do MQ-135 e converte para ppm aproximado |
| `sensorAquecido()` | bool | Verifica se o tempo de warm-up (30s) já passou desde a inicialização |

**Decisões técnicas:**
- O DHT22 tem tempo mínimo de 2 segundos entre leituras. A função deve proteger contra leituras muito frequentes.
- O MQ-135 no Wokwi é simulado por um potenciômetro. O valor analógico (0–4095 no ESP32) é mapeado para ppm usando `map()`.
- Leituras inválidas (NaN) são logadas no Serial e a função retorna `false` para o orquestrador não usar dados corrompidos.

---

### `atuadores.h` — Controle das Saídas
**Responsabilidade:** Controlar todos os componentes de saída (relés, LED, buzzer) com base no estado recebido.

**Funções planejadas:**

| Função | Parâmetro | Descrição |
|---|---|---|
| `inicializarAtuadores()` | void | Define todos os pinos de saída como OUTPUT e garante estado inicial desligado |
| `acionarAtuadores(String estado)` | estado | Decide quais relés, LED e buzzer ligar/desligar conforme o estado |
| `setRele(int pino, bool ligar)` | pino, bool | Liga ou desliga um relé específico (abstração de `digitalWrite`) |
| `setLED(String cor)` | cor | Controla o LED de status: "verde", "amarelo", "vermelho", "apagado" |
| `setBuzzer(bool ligar)` | bool | Liga ou desliga o buzzer |

**Mapa de estados × atuadores:**

| Estado | Relé 1 (Exaustor) | Relé 2 (Umidificador) | LED | Buzzer |
|---|---|---|---|---|
| NORMAL | OFF | OFF | Verde | OFF |
| ATENCAO | ON | OFF | Amarelo | OFF |
| CALOR | ON | OFF | Laranja | OFF |
| SECO | OFF | ON | Azul | OFF |
| CRITICO | ON | ON | Vermelho (pisca) | ON |

---

### `mqtt_client.h` — Comunicação MQTT
**Responsabilidade:** Gerenciar toda a comunicação com o broker MQTT: conexão, reconexão automática e publicação de dados.

**Funções planejadas:**

| Função | Retorno | Descrição |
|---|---|---|
| `inicializarMQTT()` | void | Configura o cliente com servidor e porta do broker |
| `conectarMQTT()` | bool | Tenta conectar ao broker com credenciais. Retorna `true` se bem-sucedido |
| `verificarConexaoMQTT()` | void | Chamada a cada iteração do loop: reconecta automaticamente se a conexão caiu |
| `publicarDados(String payload)` | bool | Publica o JSON no tópico configurado e retorna resultado da publicação |
| `montarPayloadJSON(float t, float u, int co2, String status)` | String | Monta e retorna a string JSON formatada com todos os dados |
| `mqttLoop()` | void | Wrapper para `client.loop()` — mantém a conexão MQTT viva |

**Formato do Payload JSON publicado:**
```json
{
  "temperatura": 27.5,
  "umidade": 38.2,
  "co2": 1120,
  "status": "ATENCAO",
  "local": "escritorio_mf",
  "timestamp": 83500
}
```

---

### `projeto_ar.ino` — Orquestrador Principal
**Responsabilidade:** Chamar as funções dos outros módulos na ordem correta. O arquivo `.ino` deve ser o mais enxuto possível — apenas coordena, não implementa lógica.

**Estrutura do `setup()`:**
```
1. Iniciar comunicação Serial (para debug)
2. Chamar inicializarAtuadores()   → pinos como OUTPUT, tudo desligado
3. Chamar inicializarSensores()    → DHT22 pronto, registrar tempo de início
4. Conectar ao Wi-Fi               → loop até obter IP
5. Chamar inicializarMQTT()        → configurar cliente
6. Chamar conectarMQTT()           → conectar ao broker
7. Ligar LED verde                 → sistema pronto
```

**Estrutura do `loop()`:**
```
1. Chamar verificarConexaoMQTT()   → reconecta se necessário (toda iteração)
2. Chamar mqttLoop()               → mantém conexão viva (toda iteração)
3. Verificar se 5s passaram via millis()
4. Se sim:
   a. Verificar sensorAquecido()  → aguarda warm-up se necessário
   b. lerDHT22()                  → temperatura e umidade
   c. lerMQ135()                  → CO2 em ppm
   d. analisarQualidade()         → determina o estado atual
   e. acionarAtuadores(estado)    → liga/desliga saídas
   f. montarPayloadJSON()         → prepara os dados
   g. publicarDados()             → envia ao dashboard
   h. Atualizar ultimaLeitura     → registrar tempo da última execução
```

---

## 3. Variáveis Globais no `projeto_ar.ino`

| Variável | Tipo | Descrição |
|---|---|---|
| `temperatura` | float | Última temperatura lida |
| `umidade` | float | Última umidade lida |
| `co2` | int | Último valor de CO2 lido |
| `estadoAtual` | String | Estado atual do sistema |
| `ultimaLeitura` | unsigned long | Timestamp da última leitura (para `millis()`) |

---

## 4. Decisão Técnica Crítica: `millis()` vs `delay()`

**Problema com `delay(5000)`:**
Travar o processador por 5 segundos impede que o `client.loop()` do MQTT seja chamado, fazendo a conexão com o broker cair. Também impede reconexões Wi-Fi e torna o sistema não responsivo.

**Solução com `millis()`:**
```
// Lógica conceitual:
se (millis() - ultimaLeitura >= INTERVALO_LEITURA):
    executar ciclo de leitura
    ultimaLeitura = millis()

// O loop() continua correndo sem travar,
// permitindo que client.loop() seja chamado a todo momento.
```

Esta escolha demonstra conhecimento de **sistemas embarcados não-bloqueantes**, um conceito avançado para o nível do projeto.

---

## 5. Tratamento de Erros Planejado

| Situação | Comportamento do Sistema |
|---|---|
| DHT22 retorna NaN | Log no Serial, pular ciclo de publicação, tentar novamente em 2s |
| MQ-135 ainda frio (< 30s) | Log "Aguardando warm-up", não publicar leitura, LED amarelo piscando |
| Wi-Fi desconecta | `verificarConexaoMQTT()` detecta, tenta reconexão automática com log |
| MQTT desconecta | Reconexão automática a cada iteração do loop, sem travar o sistema |
| Publicação MQTT falha | Log de erro no Serial, tentar novamente no próximo ciclo |

---

## 6. Ordem de Implementação Recomendada

Para evitar bugs difíceis de rastrear, implementar **um módulo por vez**, testando antes de avançar:

```
Etapa 1 → config.h + inicializarAtuadores() + Serial
          Testar: ESP32 inicializa? Pinos configuram corretamente?

Etapa 2 → sensores.h completo
          Testar: DHT22 imprime temperatura/umidade no Serial?
                  MQ-135 (potenciômetro) varia conforme giro?

Etapa 3 → analisarQualidade() com valores FIXOS (hardcoded)
          Testar: A lógica de decisão funciona sem depender dos sensores?

Etapa 4 → atuadores.h completo
          Testar: Relés ligam/desligam? LED muda conforme o estado?

Etapa 5 → mqtt_client.h: Wi-Fi + conexão MQTT + publicação de teste
          Testar: Dados chegam no dashboard com valores fixos?

Etapa 6 → Integração completa no loop()
          Testar: Ciclo completo funcionando? Simular CO2 alto no potenciômetro?

Etapa 7 → Testes de resiliência
          Testar: Desligar Wi-Fi → sistema reconecta? Girar pot para crítico → buzzer toca?
```

---

## 7. Conexão com o Pensamento Computacional

| Pilar | Aplicação Concreta no Código |
|---|---|
| **Decomposição** | Cada módulo (`.h`) resolve um subproblema isolado. `sensores.h` não sabe nada sobre MQTT; `mqtt_client.h` não sabe nada sobre relés. |
| **Abstração** | `acionarAtuadores("CRITICO")` esconde toda a complexidade de qual pino ligar. O orquestrador não precisa saber dos detalhes. |
| **Reconhecimento de Padrões** | A função `analisarQualidade()` encapsula os padrões de degradação do ar em uma estrutura `if/else if` clara e expansível. |
| **Algoritmo** | O `loop()` é um algoritmo de 7 passos sequenciais e repetitivos, com decisões condicionais em cada etapa. |

---
