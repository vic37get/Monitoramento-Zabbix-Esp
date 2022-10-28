//Inclusao das bibliotecas principais
#include <DHTesp.h> //Carrega a biblioteca do sensor de temperatura e umidade DHT.
#include <Wire.h> //Carrega a biblioteca para a comunicacao I2C.
#include "SSD1306Wire.h" //Carrega a biblioteca para o display OLED.
#include <WiFi.h> //Carrega a biblioteca da rede sem fio.
#include <HardwareSerial.h> //Carrega a biblioteca usada pelo SIM800L.
#include <String.h> //Carrega a biblioteca usada pelo SIM800L.
#include <NTPClient.h> //Carrega a biblioteaca usada para atualizar o relógio.
#include "Adafruit_MQTT.h"
 #include "Adafruit_MQTT_Client.h"
 #include "BluetoothSerial.h"
 //
 //Definicoes dos pinos no ESP32
 #define SIM_RX 17 //Cria um alias para o RX do SIM800L no pino 17 do ESP32 TX.
 #define SIM_TX 16 //Cria um alias para o TX do SIM800L no pino 16 do ESP32 RX.
 #define LEDR 12 //Cria um alias para o LED Vermelho no pino 12 do ESP32.
 #define LEDG 14 //Cria um alias para o LED Verde no pino 14 do ESP32.
 #define LEDB 32 //Cria um alias para o LED Azul no pino 32 do ESP32.
 #define BUZZER 33 // //Cria um alias para o Buzzer no pino 33 do ESP32.
 #define DHT01data 26 //Cria um alias para o modulo DHT1 no pino 26 do ESP32.
 #define OLED_SCL 22 //Cria um alias para o pino SCL (verde) do OLED com o pino 32 do ESP32.
 #define OLED_SDA 21 //Cria um alias para o pino SDA (azul) do OLED com o pino 33 do ESP32.
 #define MQ2_analog 34 //Cria um alias para DO do sensor fumaca MQ7 no pino 34 do ESP32.
 #define MQ2_digital 33 //Cria um alias para DO do sensor fumaca MQ7 no pino 33 do ESP32.
 #define MQ7_analog 35 //Cria um alias para DA do sensor gas MQ2 no pino 35 do ESP32.
 #define MQ7_digital 13 //Cria um alias para DA do sensor gas MQ2 no pino 13 do ESP32.
 //
 DHTesp dht01; //Cria um objeto do tipo DTHesp para o modulo DHT1.
 //
 //Definições do display OLED.
 SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);
 int OLEDState = 1;
 //

 //Definicoes do SIM800L (Enviar SMS e efetuar ligacao).
 HardwareSerial MSIM800L(2); //Para o SIM800L
 #define SIM800Lbauds 115200
 #define SIM800Lserial SERIAL_8N1
 int _timeout;
 String _buffer;
 String SMSNumber1 = "+55849XXXX"; //Tel. do coordenador Infraestrutura de TI.
 String SMSNumber2 = "+55849XXXX"; //Tel. do técnico de sobreaviso.
 String CallNumber = "90909XXXX"; //Liga a cobrar p/ tel. do técnico de sobreaviso.
 //
 //Define as variaveis da conexao sem fio (WiFi)
 WiFiClient client; //Inicializa a biblioteca client
 #define WIFI_SSID "XXXX" //Nome da rede (SSID)
 #define WIFI_PASS "XXXX" //Senha de rede sem fio
 String IP;
 //
 // Definições do Adafruit.IO
 #define AIO_SERVER "io.adafruit.com"
 #define AIO_SERVERPORT 1883
 #define AIO_USERNAME "XXXX"
 #define AIO_KEY "XXXX"
 Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
 #define DHT1T_FEED "XXXX/feeds/dht1Temp" //XXXX/feeds/nodeiot01.dht1temp
 #define DHT1U_FEED "XXXX/feeds/dht1Umid"
 #define MQ2_FEED "XXXX/feeds/mq2"
 #define MQ7_FEED "XXXX/feeds/mq7"
 Adafruit_MQTT_Subscribe mqttled = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led");
 Adafruit_MQTT_Publish mqttdht1t = Adafruit_MQTT_Publish(&mqtt, DHT1T_FEED);
 Adafruit_MQTT_Publish mqttdht1u = Adafruit_MQTT_Publish(&mqtt, DHT1U_FEED);
 Adafruit_MQTT_Publish mqttmq2 = Adafruit_MQTT_Publish(&mqtt, MQ2_FEED);
 Adafruit_MQTT_Publish mqttmq7 = Adafruit_MQTT_Publish(&mqtt, MQ7_FEED);
 //
 //Bluetooth Serial
 #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
 #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
 #endif
 BluetoothSerial SerialBT;
 //
 //Define as variaveis e APIs do IFTTT.
 String IFTTT_API_KEY = "XXXX"; //Define a API do IFTTT.
 String IFTTT_EVENT_NAME = "ReportESP32"; // Define o evento REPORT do IFTTT.
 String IFTTT_EVENT_NAME2 = "AlertDHTESP32"; // Define o evento ALERT do IFTTT.
 String IFTTT_EVENT_NAME3 = "AlertSmokeESP32"; // Define o evento ALERT do IFTTT.
 const char* IFTTT_host = "maker.ifttt.com"; //URL para usar o WebHook.
 //
 //Integracao com ThingSpeak (API)
 //String thingspeakAPI = "XXXX"; //API do ThingSpeak para o Modulo IoT 01.
 String thingspeakAPI = "XXXX"; //API do ThingSpeak para o Modulo IoT 00.

 //String thingspeakAPI = "XXXX"; //API do ThingSpeak para o Modulo IoT 05.
 const char* thingspeakHost = "api.thingspeak.com"; //URL da API do ThingSpeak.
 //
 // Configurações do Servidor NTP
 const char* servidorNTP = "a.st1.ntp.br"; // Servidor NTP para pesquisar a hora
 const int fusoHorario = -10800; // Fuso horário em segundos (-03h = -10800 seg)
 const int taxaDeAtualizacao = 1800000; // Taxa de atualização do servidor NTP em milisegundos
 WiFiUDP ntpUDP; // Declaração do Protocolo UDP
 NTPClient timeClient(ntpUDP, servidorNTP, fusoHorario, 60000);
 //
 //Define as variaveis e dados do Zabbix (trapper)
 float temperature;
 float humidity;
 const char* zbxserver = "0.0.0.0"; //IP do Zabbix.
 const char* zbxhostname = "nodeiot00"; //Host no Zabbix.
 int zbxporta = 10051;
 //
 //Outros
  // Contador de tempo para a funcao do IFTTT, SMS e Call...
 int IFTTT_Tempo = 0;
 int IFTTT_Intervalo = 0;
 int IFTTT_Alerta = 0;
 int SMS_Intervalo = 0;
 int SMS_Alerta = 0;
 int Call_Intervalo = 0;
 int Call_Alerta = 0;
 int MQ_Intervalo = 0;
 int MQ_Alerta = 0;
 int Zbx_Tempo = 0;
 int TS_Tempo = 0;
 int Loop = 30;
 int Loop2 = 60;
 unsigned long previousMillis = 0;
 unsigned long previousMillis2 = 0;
 unsigned long previousMillis3 = 0;
 const long interval10 = 10000;
 const long interval30 = 30000;
 const long interval60 = 60000;
 //
 Sensor MQ2 e MQ7
 int MQ2_digital_value;//variavel usada para receber o sinal digital do sensor.
 int MQ7_digital_value;//variavel usada para receber o sinal digital do sensor.
 int MQ2_analog_value;//variavel usada para receber o sinal analogico do sensor.
 int MQ7_analog_value;//variavel usada para receber o sinal analogico do sensor.
 //
 void setup() {
    MSIM800L.begin(SIM800Lbauds, SIM800Lserial, SIM_TX, SIM_RX); // Inicia o SIM800L.
    Serial.begin(115200); //Inicia o terminal para a leitura via console.
    while (! Serial);
        Serial.println("Inicializando o ESP32...");
        SerialBT.begin(zbxhostname); //Bluetooth nome visível.
        Serial.println("Hostname: " + String(zbxhostname));
        timeClient.begin(); //Inicia o cliente NTP
        pinMode(BUZZER, OUTPUT); //Define que o Buzzer sera um pino de saída.
        pinMode(LEDR, OUTPUT); //Define que o pino do LEDR sera um pino de saída.
        pinMode(LEDG, OUTPUT); //Define que o pino do LEDG sera um pino de saída.
        pinMode(LEDB, OUTPUT); //Define que o pino do LEDB sera um pino de saída.
        digitalWrite(BUZZER, LOW); //Define que o pino do Buzzer estará desligado.
        digitalWrite(LEDR, LOW); //Define que o pino do LEDR estará desligado.
        digitalWrite(LEDG, LOW); //Define que o pino do LEDG estará desligado.
        digitalWrite(LEDB, LOW); //Define que o pino do LEDB estará desligado.
        pinMode(MQ2_analog, INPUT); //Define que o pino do sensor MQ2 sera um pino de entrada.
        pinMode(MQ7_analog, INPUT); //Define que o pino do sensor MQ7 sera um pino de entrada.
        pinMode(MQ2_digital, INPUT); //Define que o pino do sensor MQ2 sera um pino de entrada.
        pinMode(MQ7_digital, INPUT); //Define que o pino do sensor MQ7 sera um pino de entrada.
        pinMode(DHT01data, INPUT); //Define que o pino do sensor DHT sera um pino de entrada.
        dht01.setup(DHT01data, DHTesp::DHT11); //Inicializacao do sensor DHT01.
        display.init(); //Inicia o display
        display.flipScreenVertically(); //Gira a exibicao do dislay.
        SetupOLED();
        testeLEDs();
        BeepLongo();
        BeepMedio();
        BeepCurto();
        wifiConnect(); //Conecta ao WiFi
 }

 void loop() {
    unsigned long currentMillis = millis();
    // Chama as seguintes funcoes:
    if (currentMillis - previousMillis >= interval10) {
        // Serial.println("Entrou no 10s loop... " + String(currentMillis) + " - "
        + String(previousMillis2));
        // Serial.println("Hora1: " + String(timeClient.getFormattedTime()) );
        previousMillis = currentMillis;
        if (OLEDState == 0) {
            OLEDState = 1;
            ambienteOLED(); //OLED1
        }
        else {
            previousMillis = currentMillis;
            OLEDState = 0;
            outrosOLED(); //OLED2
        }
    }
    if (currentMillis - previousMillis2 >= interval30) {
        // Serial.println("Entrou no 30s loop... " + String(currentMillis) + " - "
        + String(previousMillis2));
        // Serial.println("Hora2: " + String(timeClient.getFormattedTime()) );
        previousMillis2 = currentMillis;
        wifiConnect(); //Conecta a rede sem fio
        MQTT_connect(); //Adafruit MQTT
        MQTT_publish(); //Adafruit publica os feeds
        MQTT_Subscribe();
        blueToothSerial(); //Funcao para publicar no serial do BLE
        DHT1(); //Chama a funcao do sensor DHT para mostrar no serial
        sensoresMQ(); //Chama a funcao dos sensores MQ2 e MQ7 para mostrar no serial
        nodeESP32(); //Chama a funcao para mostrar informações sobre o ESP32
        notificacoes(); //Chama a funcao para notificar em dois niveis de notificacao
        IFTTTAlertDHT(); //IFTTT alerta em caso de incidente com temperatura/umidade
        IFTTTAlertSmoke(); //IFTTT alerta em caso de incidente com gás/fumaça
        IFFTTTReport(); //IFTTT report a cada 6 horas
        sendSMS(); //Chama aFuncao para enviar SMS em caso de incidente
        callNumber(); //Chama aFuncao para ligar em caso de incidente
    }

    if (currentMillis - previousMillis3 >= interval60) {
        // Serial.println("Entrou no 60s loop... " + String(currentMillis) + " - "
        + String(previousMillis3));
        // Serial.println("Hora3: " + String(timeClient.getFormattedTime()) );
        previousMillis3 = currentMillis;
        zabbixFog(); // Envia os dados para a fog (Zabbix)
        thingspeakCloud(); // Envia os dados para a cloud (ThingSpeak)
    }
 }
 void testeLEDs() {
    digitalWrite(LEDB, HIGH); //Liga o LED azul
    delay(2000);
    digitalWrite(LEDB, LOW); //Desliga o LED azul
    delay(100);
    digitalWrite(LEDG, HIGH); //Liga o LED verde
    delay(2000);
    digitalWrite(LEDG, LOW); //Desliga o LED verde
    delay(100);
    digitalWrite(LEDR, HIGH); //Liga o LED vermelho
    delay(2000);
    digitalWrite(LEDR, LOW); //Desliga o LED vermelho
 }
 //Funcao para conectar a rede Wifi...
 void wifiConnect() {
    int cont = 0;
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Tentando conectar na rede sem fio ");
        Serial.println(WIFI_SSID);
        do {
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        cont = cont + 1;
        delay(2000);
        } while (WiFi.status() != WL_CONNECTED && cont != 2);
    }
    else {
        Serial.print("SSID: ");
        Serial.println(WIFI_SSID);
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC: ");
        Serial.println(WiFi.macAddress()); //retorna o endereço MAC do node.
        Serial.println("Hostname: " + String(zbxhostname));
        IP = WiFi.localIP().toString();
    }
 }
 //Bluetooth serial
 void blueToothSerial() {
    SerialBT.println("-------------------------------------");
    SerialBT.println("Hora: " + String(timeClient.getFormattedTime()) );
    SerialBT.print("SSID: ");
    SerialBT.println(WIFI_SSID);
    SerialBT.print("IP: ");
    SerialBT.println(WiFi.localIP());
    SerialBT.print("MAC: ");
    SerialBT.println(WiFi.macAddress());
    SerialBT.println("Hostname: " + String(zbxhostname));
    TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
    MQ2_analog_value = analogRead(MQ2_analog);
    MQ7_analog_value = analogRead(MQ7_analog);
    SerialBT.println("Temperatura: " + String(LeituraDHT01.temperature, 0) + String(" ºC")
    + " | Umidade: " + String(LeituraDHT01.humidity, 0) + String(" %"));
    SerialBT.println("MQ2: " + String(MQ2_analog_value)
    + " | MQ7: " + String (MQ7_analog_value));
 }
 //Funcao para ler a temperatura e a umidade...
 void DHT1() {
    TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
    Serial.println("Temperatura: " + String(LeituraDHT01.temperature, 0) + String(" ºC")
    + " | Umidade: " + String(LeituraDHT01.humidity, 0) + String(" %"));
 }
 //Funcao responsavel por disparar alerta caso o sensor detectar gas/fumaca...
 void sensoresMQ() {
    MQ2_digital_value = digitalRead(MQ2_digital);
    MQ7_digital_value = digitalRead(MQ7_digital);
    MQ2_analog_value = analogRead(MQ2_analog);
    MQ7_analog_value = analogRead(MQ7_analog);
    Serial.println("MQ2: " + String(MQ2_analog_value)
    + " | MQ7: " + String (MQ7_analog_value));
 }
 //Funcao para mostrar informacoes do ESP32...
 void nodeESP32() {
    Serial.print("CPU Temp.: ");
    Serial.print(temperatureRead()); //returns CPU Temperature in ºC
    Serial.print(" ºC");
    Serial.print(" | Hall sensor: ");
    Serial.println(hallRead()); // reads Hall sensor
    //Serial.print(" Touch: ");
    //Serial.println(touchRead(4));// reads touch sensor on pin 4
    //
 }
 //Funcoes para definir os status entre OK, alerta e problema...

 void StatusOK() {
    digitalWrite(LEDB, LOW); //Desliga o LED azul caso esteja ligado
    digitalWrite(LEDR, LOW); //Desliga o LED vermelho caso esteja ligado
    digitalWrite(LEDG, HIGH); //Liga o LED verde (OK)
    digitalWrite(BUZZER, LOW); //Desliga o Buzzer caso esteja ligado
    Serial.println("Status: OK!");
 }
 void StatusAlert(int Beeps) {
    digitalWrite(LEDG, LOW); //Desliga o LED verde caso esteja ligado
    digitalWrite(LEDR, LOW); //Desliga o LED vermelho caso esteja ligado
    digitalWrite(LEDB, HIGH); //Liga o LED azul (Alert)
    switch (Beeps) {
    case 3:
    BeepMedio();
    BeepCurto();
    break;
    case 4:
    BeepMedio();
    break;
    }
    Serial.println("Status: Alerta...");
 }
 void StatusProblem(int Beeps) {
    digitalWrite(LEDB, LOW); //Desliga o LED azul caso esteja ligado
    digitalWrite(LEDG, LOW); //Desliga o LED verde caso esteja ligado
    digitalWrite(LEDR, HIGH); //Liga o LED vermelho (Problem)
    switch (Beeps) {
    case 1:
    BeepLongo();
    BeepMedio();
    BeepCurto();
    break;
    case 2:
    BeepLongo();
    BeepCurto();
    break;
    }
    Serial.println("Status: Problema...");
 }
 void notificacoes() {
    MQ2_analog_value = analogRead(MQ2_analog);
    MQ7_analog_value = analogRead(MQ7_analog);
    TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
    if (LeituraDHT01.temperature >= 35 || LeituraDHT01.humidity >= 90
    || MQ2_analog_value > 1000 || MQ7_analog_value > 1000) {
    StatusProblem(1);
    }
    else if (LeituraDHT01.temperature >= 30 || LeituraDHT01.humidity >= 80
    || MQ2_analog_value > 800 || MQ7_analog_value > 800) {
    StatusProblem(2);
    }
    else if (LeituraDHT01.temperature >= 29 || LeituraDHT01.humidity >= 70
    || MQ2_analog_value > 600 || MQ7_analog_value > 600) {
    StatusAlert(3);
    }
    else if (LeituraDHT01.temperature >= 28 || LeituraDHT01.humidity >= 60
    || MQ2_analog_value > 500 || MQ7_analog_value > 500) {
    StatusAlert(4);
    }
    else {
    StatusOK();
    }
 }
 //Funcao para mostrar uma mensagem inicial no OLED na inicializacao do ESP32...
 void SetupOLED() {
    //Limpa o display
    display.clear();
    //Centraliza o texto no display
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    //Seleciona a fonte
    display.setFont(ArialMT_Plain_10);
    //Mostra informacao no "boot"
    display.drawString(63, 10, "TCERN");
    display.drawString(63, 20, "Modulo Principal");
    display.drawString(63, 30, "Projeto IMD");
    display.drawString(63, 40, "IoT DCIM");
    display.drawString(63, 50, "v.1.0");
    display.display();
 }
 //Funcao para exibir leitura do ambiente no OLED...
 void ambienteOLED() {
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 display.clear();
 display.setTextAlignment(TEXT_ALIGN_CENTER);
 display.setFont(ArialMT_Plain_16);
 display.drawString(63, 10, IP);
 display.drawString(63, 26, zbxhostname);
 display.drawString(63, 42, String(LeituraDHT01.temperature, 0) + String(" ºC")
 + String(" | ") + String(LeituraDHT01.humidity, 0) + String(" %") );
 display.display();//Escreve as informações acima no display.
 }
 //Funcao para exibir outras informacoes no OLED...
 void outrosOLED() {
 //MQ2_analog_value = analogRead(MQ2_analog);
 //MQ7_analog_value = analogRead(MQ7_analog);
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 timeClient.update();
 String horario = timeClient.getFormattedTime();
 display.clear();
 display.setTextAlignment(TEXT_ALIGN_CENTER);
 display.setFont(ArialMT_Plain_16);
 display.drawString(63, 10, String(LeituraDHT01.temperature, 0) + String(" ºC")
 + String(" | ") + String(LeituraDHT01.humidity, 0) + String(" %") );
 display.drawString(63, 26, "MQ7: " + String(MQ7_analog_value));
 display.drawString(63, 42, "MQ2: " + String(MQ2_analog_value));
 display.display();
 }
 //Funcao para enviar os dados para a nuvem ThingSpeak (cloud)...
 void thingspeakCloud() {
 if (client.connect(thingspeakHost, 80)) {
 //MQ2_analog_value = analogRead(MQ2_analog);
 //MQ7_analog_value = analogRead(MQ7_analog);
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 TS_Tempo += Loop2;
 //Condicao para enviar as informações ao ThingSpeak a cada 2 minutos...
 if (TS_Tempo >= 120) {
 String thingspeak = thingspeakAPI;
 thingspeak += "&amp;field1=";
 thingspeak += String(LeituraDHT01.temperature);
 thingspeak += "&amp;field2=";
 thingspeak += String(LeituraDHT01.humidity);
 //thingspeak += "&amp;field3=";
 //thingspeak += String(MQ2_analog_value);
 //thingspeak += "&amp;field4=";
 //thingspeak += String(MQ7_analog_value);
 client.print("POST /update HTTP/1.1\n");
 client.print("Host: api.thingspeak.com\n");
 client.print("Connection: close\n");
 client.print("X-THINGSPEAKAPIKEY: " + thingspeakAPI + "\n");
 client.print("Content-Type: application/x-www-form-urlencoded\n");
 client.print("Content-Length: ");
 client.print(thingspeak.length());
 client.print("\n\n");
 client.print(thingspeak);
 Serial.println("ThingSpeak:");
 Serial.println(thingspeak);
 Serial.print("\n");
 TS_Tempo = 0;
 }
 }
 //Serial.println("Debug contador ThingSpeak:");
 //Serial.println(TS_Tempo);
 }
 //Funcao para enviar os dados p/ o Zabbix na LAN (fog)...
 void zabbixFog (void) {
 Zbx_Tempo += Loop2;
 //Condicao para enviar as informações ao Zabbix a cada 2 minutos...
 if (Zbx_Tempo >= 120) {
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 String zbxhost = zbxhostname;
 String itens[] = {"LeituraDHT01.temperature", "LeituraDHT01.humidity"};
 float valores[] = {LeituraDHT01.temperature, LeituraDHT01.humidity};
 for (int i = 0; i < (sizeof(valores) / sizeof(int)); i++) {
 if (client.connect(zbxserver, zbxporta)) {
 String zabbix = "";
 zabbix += String("{\"request\":\"sender data\",\"data\":");
 zabbix += String("[{");
 zabbix += String("\"host\":") + String("\"") + String (zbxhost)
 + String("\"") + String(",");
 zabbix += String("\"key\":") + String("\"") + String (itens[i])
 + String("\"") + String(",");
 zabbix += String("\"value\":") + String("\"") + String (valores[i])
 + String("\"");
 zabbix += String("}]}");
 Serial.println("Zabbix:");
 Serial.println(zabbix);
 client.println(zabbix);
 zabbix = "";
 String respostazbx = client.readStringUntil('\r');
 Serial.println(respostazbx);
 Zbx_Tempo = 0;
 }
 }
 }
 //Serial.println("Debug contador Zbx:");
 //Serial.println(Zbx_Tempo);
 }
 //Funcao para enviar os dados para o IFTTT (WebHook)...
 void IFFTTTReport() {
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 if (client.connect(IFTTT_host, 80)) {
 IFTTT_Tempo += Loop;
 //Condicao para enviar um relatorio a cada 6 horas...
 if (IFTTT_Tempo >= 21600) {
 //POST json.
 String IFTTT_url = "/trigger/";
 IFTTT_url += IFTTT_EVENT_NAME;
 IFTTT_url += "/with/key/";
 IFTTT_url += IFTTT_API_KEY;
 String IFTTT_jsonObject = String("{\"value1\":\"")
 + String(LeituraDHT01.temperature, 0)
 + "\",\"value2\":\""
 + String(LeituraDHT01.humidity, 0)
 + "\",\"value3\":\"" + String(IP) + "\"}";
 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
 client.println(String("Host: ") + IFTTT_host);
 client.println("Connection: close\r\nContent-Type: application/json");
 client.print("Content-Length: ");
 client.println(IFTTT_jsonObject.length());
 client.println();
 client.println(IFTTT_jsonObject);
 Serial.println("IFTTT:");
 Serial.println(IFTTT_jsonObject);
 IFTTT_Tempo = 0;
 }
 }
 //Serial.println("Debug contador IFTTT:");
 //Serial.println(IFTTT_Tempo);
 }
 void IFTTTAlertDHT() {
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 if (client.connect(IFTTT_host, 80)) {
 //Se temperatura estiver 30 graus e/ou umidade 90 por cento, alerta a cada 1 hora...
 if (IFTTT_Alerta == 0 && (LeituraDHT01.temperature >= 30
 || LeituraDHT01.humidity >= 90)) {
 //POST json.
 String IFTTT_url = "/trigger/";
 IFTTT_url += IFTTT_EVENT_NAME2;
 IFTTT_url += "/with/key/";
 IFTTT_url += IFTTT_API_KEY;
 String IFTTT_jsonObject = String("{\"value1\":\"")
 + String(LeituraDHT01.temperature, 0)
 + "\",\"value2\":\""
 + String(LeituraDHT01.humidity, 0)
 + "\",\"value3\":\""
 + String(IP) + "\"}";
 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
 client.println(String("Host: ") + IFTTT_host);
 client.println("Connection: close\r\nContent-Type: application/json");
 client.print("Content-Length: ");
 client.println(IFTTT_jsonObject.length());
 client.println();
 client.println(IFTTT_jsonObject);
 Serial.println("IFTTT:");
 Serial.println(IFTTT_jsonObject);
 Serial.println("Alerta para o IFTTT enviado...");
 IFTTT_Alerta = 1;
 }
 }
 if (IFTTT_Alerta == 1) {
 IFTTT_Intervalo += Loop;
 }
 if (IFTTT_Intervalo >= 3600) { // 1 hora
 IFTTT_Intervalo = 0;
 IFTTT_Alerta = 0;
 }
 //Serial.println("Debug contador IFTTT:");
 //Serial.println(IFTTT_Alerta);
 //Serial.println(IFTTT_Intervalo);
 }
 void IFTTTAlertSmoke() {
 if (MQ_Alerta == 0 && MQ2_analog_value > 600 || MQ7_analog_value > 600) {
 Serial.println("Status: Gas e/ou fumaca detectado...");
 if (client.connect(IFTTT_host, 80)) {
 //POST json.
 String IFTTT_url = "/trigger/";
 IFTTT_url += IFTTT_EVENT_NAME3;
 IFTTT_url += "/with/key/";
 IFTTT_url += IFTTT_API_KEY;
 String IFTTT_jsonObject = String("{\"value1\":\"") + String(MQ2_analog_value)
 + "\",\"value2\":\"" + String(MQ7_analog_value) + "\"}";
 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
 client.println(String("Host: ") + IFTTT_host);
 client.println("Connection: close\r\nContent-Type: application/json");
 client.print("Content-Length: ");
 client.println(IFTTT_jsonObject.length());
 client.println();
 client.println(IFTTT_jsonObject);
 Serial.println("IFTTT:");
 Serial.println(IFTTT_jsonObject);
 }
 MQ_Alerta = 1;
 }
 if (MQ_Alerta == 1) {
 MQ_Intervalo += Loop;
 }
 if (MQ_Intervalo >= 900) { //15 minutos
 MQ_Intervalo = 0;
 MQ_Alerta = 0;
 }
 //Serial.println("Debug contador MQ:");
 //Serial.println(MQ_Alerta);
 //Serial.println(MQ_Intervalo);
 }
 //Funcao para enviar SMS em caso de incidente, precisa de credito.
 void sendSMS() {
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 if (SMS_Alerta == 0 && WiFi.status() != WL_CONNECTED && (LeituraDHT01.temperature >= 30
 || LeituraDHT01.humidity >= 90)) {
 Serial.println ("Sending Message");
 MSIM800L.println("AT+CMGF=1"); //Coloca o modulo GSM em modo texto
 delay(1000);
 MSIM800L.println("AT+CMGS=\"" + SMSNumber1 + "\"\r"); //Numero para enviar o SMS
 delay(1000);
 String MensagemSMS = (String("Alerta - temperatura ")
 + String(LeituraDHT01.temperature, 0)
 + String(" C") + String(" e umidade ")
 + String(LeituraDHT01.humidity, 0)
 + String("% no node ")
 + String(zbxhostname) + String(".") );
 Serial.println(MensagemSMS);
 MSIM800L.println(MensagemSMS);
 delay(100);
 MSIM800L.println((char)26);// ASCII para CTRL+Z
 delay(1000);
 Serial.println("SMS enviado com sucesso...");
 _buffer = _readSerial();
 SMS_Alerta = 1;
 }
 if (SMS_Alerta == 1) {
 SMS_Intervalo += Loop;
 }
 if (SMS_Intervalo >= 1800) { //30 minutos
 SMS_Intervalo = 0;
 SMS_Alerta = 0;
 }
 //Serial.println("Debug contador SMS:");
 //Serial.println(SMS_Alerta);
 //Serial.println(SMS_Intervalo);
 }
 //Funcao para ligar a cobrar para os telefones em caso de incidente.
 void callNumber() {
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 if (Call_Alerta == 0 && LeituraDHT01.temperature >= 25
 || LeituraDHT01.humidity >= 90) {
 delay(1000);
 Serial.println("Ligando...");
 MSIM800L.print (F("ATD"));
 MSIM800L.print (CallNumber);
 MSIM800L.print (F(";\r\n"));
 MSIM800L.print (F("WAIT=4"));
 MSIM800L.print (F("ATH")); //Comando para desligar
 MSIM800L.print (F(";\r\n"));
 Serial.println("Desligando...");
 _buffer = _readSerial();
 Serial.println(_buffer);
 delay(1000);
 Call_Alerta = 1;
 }
 if (Call_Alerta == 1) {
 Call_Intervalo += Loop;
 }
 if (Call_Intervalo >= 900) { //15 minutos
 Call_Intervalo = 0;
 Call_Alerta = 0;
 }
 //Serial.println("Debug contador Call:");
 //Serial.println(Call_Alerta);
 //Serial.println(Call_Intervalo);
 }
 //String para ler o serial do SIM800L, usado pelas funcoes do SMS e Call...
 String _readSerial() {
 _timeout = 0;
 while (!MSIM800L.available() && _timeout < 12000 )
 {
 delay(13);
 _timeout++;
 }
 if (MSIM800L.available()) {
 return MSIM800L.readString();
 }
 }
 * /
 void MQTT_connect() {
 int8_t ret;

 // Stop if already connected.
 if (mqtt.connected()) {
 return;
 }

 Serial.print(F("Connecting to MQTT... "));

 uint8_t retries = 3;
 while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
 Serial.println(mqtt.connectErrorString(ret));
 Serial.println(F("Retrying MQTT connection in 5 seconds..."));
 mqtt.disconnect();
 delay(5000); // wait 5 seconds
 retries--;
 if (retries == 0) {
 // basically die and wait for WDT to reset me
 while (1);
 }
 }
 Serial.println(F("MQTT Connected!"));
 }
 void MQTT_publish() {
 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
 //MQ2_analog_value = analogRead(MQ2_analog);
 //MQ7_analog_value = analogRead(MQ7_analog);
 //mqttmq2.publish(MQ2_analog_value);
 //mqttmq7.publish(MQ7_analog_value);
 mqttdht1t.publish(LeituraDHT01.temperature);
 mqttdht1u.publish(LeituraDHT01.humidity);
 /*if (! mqttcpu.publish(temperatureRead()) && mqttmq2.publish(MQ2_analog_value)
 && mqttmq7.publish(MQ7_analog_value) ) {
 Serial.println(F("MQTT Adafruit.IO: Publish Failed."));
 } else {
 Serial.println(F("MQTT Adafruit.IO: Publish Success!"));
 delay(500);
 }
 delay(10000);*/
 }
 void MQTT_Subscribe () {
 mqtt.subscribe(&mqttled);
 Adafruit_MQTT_Subscribe *subscription;
 while ((subscription = mqtt.readSubscription(5000))) {
 // Check if its the onoff button feed
 if (subscription == &mqttled) {
 Serial.print(F("On-Off button: "));
 Serial.println((char *)mqttled.lastread);

 if (strcmp((char *)mqttled.lastread, "ON") == 0) {
 digitalWrite(LEDR, LOW);
 }
 if (strcmp((char *)mqttled.lastread, "OFF") == 0) {
 digitalWrite(LEDR, HIGH);
 }
 }
 }
 }