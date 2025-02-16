# 📌 MedTime - Sistema de Lembrete de Medicamentos com Raspberry Pi Pico W

## 📖 Descrição
Este projeto utiliza um **Raspberry Pi Pico** para criar um sistema de lembrete de medicamentos. O dispositivo exibe um relógio em um **display OLED**, aciona **LEDs e um buzzer** para alertar sobre os horários dos medicamentos e registra o consumo dos remédios na **plataforma ThingSpeak** via Wi-Fi.

## 🚀 Funcionalidades
- **Exibição do Relógio** no display OLED
- **Configuração manual do horário**
- **Programação dos horários dos medicamentos**
- **Alertas com LEDs e buzzer** para lembrar de tomar os medicamentos
- **Confirmação do consumo** pressionando um botão
- **Registro dos horários de consumo no ThingSpeak** via conexão Wi-Fi

## 🛠️ Hardware Necessário
- Raspberry Pi Pico
- Display OLED SSD1306 (I2C)
- Módulo Wi-Fi (ex: ESP8266 via UART)
- LEDs RGB
- Buzzer
- Botões para interação
- Fonte de alimentação (bateria ou USB)

## 📦 Configuração
### 1️⃣ **Bibliotecas Necessárias**
Para compilar e rodar o código, instale as seguintes bibliotecas:

- **Pico SDK**
- **Biblioteca para SSD1306** (ex: `ssd1306.h`)
- **Biblioteca para Wi-Fi** (ex: `cyw43_arch.h` para o Pico W)

### 2️⃣ **Configurar o Wi-Fi**
No arquivo principal do código, edite as credenciais da rede Wi-Fi:
```c
#define WIFI_SSID "Seu_SSID"
#define WIFI_PASS "Sua_Senha"
```

### 3️⃣ **Configurar o ThingSpeak**
Crie uma conta no [ThingSpeak](https://thingspeak.com/) e obtenha sua chave API de escrita:
```c
#define API_KEY "SUA_CHAVE_API"
```

### 4️⃣ **Programação dos Horários dos Medicamentos**
No código principal, os horários dos medicamentos podem ser configurados na função `verificar_medicamentos()`, onde os horários podem ser ajustados conforme necessário:
```c
void verificar_medicamentos() {
    if (hora_atual == 8 && !medicamento1_consumido) {
        rotina_medicamento(Medicamento1, &medicamento1_consumido);
    }
    if (hora_atual == 10 && !medicamento2_consumido) {
        rotina_medicamento(Medicamento2, &medicamento2_consumido);
    }
    if (hora_atual == 21 && !medicamento3_consumido) {
        rotina_medicamento(Medicamento3, &medicamento3_consumido);
    }
}
```
Esses horários podem ser modificados conforme a necessidade do usuário.

## ▶️ Como Executar
1. Compile e grave o firmware no Raspberry Pi Pico.
2. Conecte o hardware conforme o esquema.
3. Ligue o sistema e configure o horário usando os botões.
4. Aguarde os alertas de medicamento e pressione o botão para confirmar o consumo.
5. Verifique os registros no ThingSpeak.

## 🛠 Possíveis Melhorias
- 📡 Integração com **aplicativos móveis** para alertas via notificações
- 🔋 **Otimização de energia** para uso com bateria
- 💾 **Armazenamento local** dos horários caso o Wi-Fi esteja offline

## 📜 Licença
Este projeto é de uso livre. Fique à vontade para modificá-lo e adaptá-lo conforme necessário! 🚀

