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
29 //
30 //Definições do display OLED.
31 SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);
32 int OLEDState = 1;
33 //
51
34 //Definicoes do SIM800L (Enviar SMS e efetuar ligacao).
35 HardwareSerial MSIM800L(2); //Para o SIM800L
36 #define SIM800Lbauds 115200
37 #define SIM800Lserial SERIAL_8N1
38 int _timeout;
39 String _buffer;
40 String SMSNumber1 = "+55849XXXX"; //Tel. do coordenador Infraestrutura de TI.
41 String SMSNumber2 = "+55849XXXX"; //Tel. do técnico de sobreaviso.
42 String CallNumber = "90909XXXX"; //Liga a cobrar p/ tel. do técnico de sobreaviso.
43 //
44 //Define as variaveis da conexao sem fio (WiFi)
45 WiFiClient client; //Inicializa a biblioteca client
46 #define WIFI_SSID "XXXX" //Nome da rede (SSID)
47 #define WIFI_PASS "XXXX" //Senha de rede sem fio
48 String IP;
49 //
50 // Definições do Adafruit.IO
51 #define AIO_SERVER "io.adafruit.com"
52 #define AIO_SERVERPORT 1883
53 #define AIO_USERNAME "XXXX"
54 #define AIO_KEY "XXXX"
55 Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
56 #define DHT1T_FEED "XXXX/feeds/dht1Temp" //XXXX/feeds/nodeiot01.dht1temp
57 #define DHT1U_FEED "XXXX/feeds/dht1Umid"
58 #define MQ2_FEED "XXXX/feeds/mq2"
59 #define MQ7_FEED "XXXX/feeds/mq7"
60 Adafruit_MQTT_Subscribe mqttled = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led");
61 Adafruit_MQTT_Publish mqttdht1t = Adafruit_MQTT_Publish(&mqtt, DHT1T_FEED);
62 Adafruit_MQTT_Publish mqttdht1u = Adafruit_MQTT_Publish(&mqtt, DHT1U_FEED);
63 Adafruit_MQTT_Publish mqttmq2 = Adafruit_MQTT_Publish(&mqtt, MQ2_FEED);
64 Adafruit_MQTT_Publish mqttmq7 = Adafruit_MQTT_Publish(&mqtt, MQ7_FEED);
65 //
66 //Bluetooth Serial
67 #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
68 #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
69 #endif
70 BluetoothSerial SerialBT;
71 //
72 //Define as variaveis e APIs do IFTTT.
73 String IFTTT_API_KEY = "XXXX"; //Define a API do IFTTT.
74 String IFTTT_EVENT_NAME = "ReportESP32"; // Define o evento REPORT do IFTTT.
75 String IFTTT_EVENT_NAME2 = "AlertDHTESP32"; // Define o evento ALERT do IFTTT.
76 String IFTTT_EVENT_NAME3 = "AlertSmokeESP32"; // Define o evento ALERT do IFTTT.
77 const char* IFTTT_host = "maker.ifttt.com"; //URL para usar o WebHook.
78 //
79 //Integracao com ThingSpeak (API)
80 //String thingspeakAPI = "XXXX"; //API do ThingSpeak para o Modulo IoT 01.
81 String thingspeakAPI = "XXXX"; //API do ThingSpeak para o Modulo IoT 00.
52
82 //String thingspeakAPI = "XXXX"; //API do ThingSpeak para o Modulo IoT 05.
83 const char* thingspeakHost = "api.thingspeak.com"; //URL da API do ThingSpeak.
84 //
85 // Configurações do Servidor NTP
86 const char* servidorNTP = "a.st1.ntp.br"; // Servidor NTP para pesquisar a hora
87 const int fusoHorario = -10800; // Fuso horário em segundos (-03h = -10800 seg)
88 const int taxaDeAtualizacao = 1800000; // Taxa de atualização do servidor NTP em milisegundos
89 WiFiUDP ntpUDP; // Declaração do Protocolo UDP
90 NTPClient timeClient(ntpUDP, servidorNTP, fusoHorario, 60000);
91 //
92 //Define as variaveis e dados do Zabbix (trapper)
93 float temperature;
94 float humidity;
95 const char* zbxserver = "0.0.0.0"; //IP do Zabbix.
96 const char* zbxhostname = "nodeiot00"; //Host no Zabbix.
97 int zbxporta = 10051;
98 //
99 //Outros
100 // Contador de tempo para a funcao do IFTTT, SMS e Call...
101 int IFTTT_Tempo = 0;
102 int IFTTT_Intervalo = 0;
103 int IFTTT_Alerta = 0;
104 int SMS_Intervalo = 0;
105 int SMS_Alerta = 0;
106 int Call_Intervalo = 0;
107 int Call_Alerta = 0;
108 int MQ_Intervalo = 0;
109 int MQ_Alerta = 0;
110 int Zbx_Tempo = 0;
111 int TS_Tempo = 0;
112 int Loop = 30;
113 int Loop2 = 60;
114 unsigned long previousMillis = 0;
115 unsigned long previousMillis2 = 0;
116 unsigned long previousMillis3 = 0;
117 const long interval10 = 10000;
118 const long interval30 = 30000;
119 const long interval60 = 60000;
120 //
121 Sensor MQ2 e MQ7
122 int MQ2_digital_value;//variavel usada para receber o sinal digital do sensor.
123 int MQ7_digital_value;//variavel usada para receber o sinal digital do sensor.
124 int MQ2_analog_value;//variavel usada para receber o sinal analogico do sensor.
125 int MQ7_analog_value;//variavel usada para receber o sinal analogico do sensor.
126 //
127 void setup() {
128 MSIM800L.begin(SIM800Lbauds, SIM800Lserial, SIM_TX, SIM_RX); // Inicia o SIM800L.
129 Serial.begin(115200); //Inicia o terminal para a leitura via console.
53
130 while (! Serial);
131 Serial.println("Inicializando o ESP32...");
132 SerialBT.begin(zbxhostname); //Bluetooth nome visível.
133 Serial.println("Hostname: " + String(zbxhostname));
134 timeClient.begin(); //Inicia o cliente NTP
135 pinMode(BUZZER, OUTPUT); //Define que o Buzzer sera um pino de saída.
136 pinMode(LEDR, OUTPUT); //Define que o pino do LEDR sera um pino de saída.
137 pinMode(LEDG, OUTPUT); //Define que o pino do LEDG sera um pino de saída.
138 pinMode(LEDB, OUTPUT); //Define que o pino do LEDB sera um pino de saída.
139 digitalWrite(BUZZER, LOW); //Define que o pino do Buzzer estará desligado.
140 digitalWrite(LEDR, LOW); //Define que o pino do LEDR estará desligado.
141 digitalWrite(LEDG, LOW); //Define que o pino do LEDG estará desligado.
142 digitalWrite(LEDB, LOW); //Define que o pino do LEDB estará desligado.
143 pinMode(MQ2_analog, INPUT); //Define que o pino do sensor MQ2 sera um pino de entrada.
144 pinMode(MQ7_analog, INPUT); //Define que o pino do sensor MQ7 sera um pino de entrada.
145 pinMode(MQ2_digital, INPUT); //Define que o pino do sensor MQ2 sera um pino de entrada.
146 pinMode(MQ7_digital, INPUT); //Define que o pino do sensor MQ7 sera um pino de entrada.
147 pinMode(DHT01data, INPUT); //Define que o pino do sensor DHT sera um pino de entrada.
148 dht01.setup(DHT01data, DHTesp::DHT11); //Inicializacao do sensor DHT01.
149 display.init(); //Inicia o display
150 display.flipScreenVertically(); //Gira a exibicao do dislay.
151 SetupOLED();
152 testeLEDs();
153 BeepLongo();
154 BeepMedio();
155 BeepCurto();
156 wifiConnect(); //Conecta ao WiFi
157 }
158 void loop() {
159 unsigned long currentMillis = millis();
160 // Chama as seguintes funcoes:
161 if (currentMillis - previousMillis >= interval10) {
162 // Serial.println("Entrou no 10s loop... " + String(currentMillis) + " - "
163 + String(previousMillis2));
164 // Serial.println("Hora1: " + String(timeClient.getFormattedTime()) );
165 previousMillis = currentMillis;
166 if (OLEDState == 0) {
167 OLEDState = 1;
168 ambienteOLED(); //OLED1
169 } else {
170 previousMillis = currentMillis;
171 OLEDState = 0;
172 outrosOLED(); //OLED2
173 }
174 }
175 if (currentMillis - previousMillis2 >= interval30) {
176 // Serial.println("Entrou no 30s loop... " + String(currentMillis) + " - "
177 + String(previousMillis2));
54
178 // Serial.println("Hora2: " + String(timeClient.getFormattedTime()) );
179 previousMillis2 = currentMillis;
180 wifiConnect(); //Conecta a rede sem fio
181 MQTT_connect(); //Adafruit MQTT
182 MQTT_publish(); //Adafruit publica os feeds
183 MQTT_Subscribe();
184 blueToothSerial(); //Funcao para publicar no serial do BLE
185 DHT1(); //Chama a funcao do sensor DHT para mostrar no serial
186 sensoresMQ(); //Chama a funcao dos sensores MQ2 e MQ7 para mostrar no serial
187 nodeESP32(); //Chama a funcao para mostrar informações sobre o ESP32
188 notificacoes(); //Chama a funcao para notificar em dois niveis de notificacao
189 IFTTTAlertDHT(); //IFTTT alerta em caso de incidente com temperatura/umidade
190 IFTTTAlertSmoke(); //IFTTT alerta em caso de incidente com gás/fumaça
191 IFFTTTReport(); //IFTTT report a cada 6 horas
192 sendSMS(); //Chama aFuncao para enviar SMS em caso de incidente
193 callNumber(); //Chama aFuncao para ligar em caso de incidente
194 }
195 if (currentMillis - previousMillis3 >= interval60) {
196 // Serial.println("Entrou no 60s loop... " + String(currentMillis) + " - "
197 + String(previousMillis3));
198 // Serial.println("Hora3: " + String(timeClient.getFormattedTime()) );
199 previousMillis3 = currentMillis;
200 zabbixFog(); // Envia os dados para a fog (Zabbix)
201 thingspeakCloud(); // Envia os dados para a cloud (ThingSpeak)
202 }
203 }
204 void testeLEDs() {
205 digitalWrite(LEDB, HIGH); //Liga o LED azul
206 delay(2000);
207 digitalWrite(LEDB, LOW); //Desliga o LED azul
208 delay(100);
209 digitalWrite(LEDG, HIGH); //Liga o LED verde
210 delay(2000);
211 digitalWrite(LEDG, LOW); //Desliga o LED verde
212 delay(100);
213 digitalWrite(LEDR, HIGH); //Liga o LED vermelho
214 delay(2000);
215 digitalWrite(LEDR, LOW); //Desliga o LED vermelho
216 }
217 //Funcao para conectar a rede Wifi...
218 void wifiConnect() {
219 int cont = 0;
220 if (WiFi.status() != WL_CONNECTED) {
221 Serial.print("Tentando conectar na rede sem fio ");
222 Serial.println(WIFI_SSID);
223 do {
224 WiFi.begin(WIFI_SSID, WIFI_PASS);
225 cont = cont + 1;
55
226 delay(2000);
227 } while (WiFi.status() != WL_CONNECTED && cont != 2);
228 }
229 else {
230 Serial.print("SSID: ");
231 Serial.println(WIFI_SSID);
232 Serial.print("IP: ");
233 Serial.println(WiFi.localIP());
234 Serial.print("MAC: ");
235 Serial.println(WiFi.macAddress()); //retorna o endereço MAC do node.
236 Serial.println("Hostname: " + String(zbxhostname));
237 IP = WiFi.localIP().toString();
238 }
239 }
240 //Bluetooth serial
241 void blueToothSerial() {
242 SerialBT.println("-------------------------------------");
243 SerialBT.println("Hora: " + String(timeClient.getFormattedTime()) );
244 SerialBT.print("SSID: ");
245 SerialBT.println(WIFI_SSID);
246 SerialBT.print("IP: ");
247 SerialBT.println(WiFi.localIP());
248 SerialBT.print("MAC: ");
249 SerialBT.println(WiFi.macAddress());
250 SerialBT.println("Hostname: " + String(zbxhostname));
251 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
252 MQ2_analog_value = analogRead(MQ2_analog);
253 MQ7_analog_value = analogRead(MQ7_analog);
254 SerialBT.println("Temperatura: " + String(LeituraDHT01.temperature, 0) + String(" ºC")
255 + " | Umidade: " + String(LeituraDHT01.humidity, 0) + String(" %"));
256 SerialBT.println("MQ2: " + String(MQ2_analog_value)
257 + " | MQ7: " + String (MQ7_analog_value));
258 }
259 //Funcao para ler a temperatura e a umidade...
260 void DHT1() {
261 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
262 Serial.println("Temperatura: " + String(LeituraDHT01.temperature, 0) + String(" ºC")
263 + " | Umidade: " + String(LeituraDHT01.humidity, 0) + String(" %"));
264 }
265 //Funcao responsavel por disparar alerta caso o sensor detectar gas/fumaca...
266 void sensoresMQ() {
267 MQ2_digital_value = digitalRead(MQ2_digital);
268 MQ7_digital_value = digitalRead(MQ7_digital);
269 MQ2_analog_value = analogRead(MQ2_analog);
270 MQ7_analog_value = analogRead(MQ7_analog);
271 Serial.println("MQ2: " + String(MQ2_analog_value)
272 + " | MQ7: " + String (MQ7_analog_value));
273 }
56
274 //Funcao para mostrar informacoes do ESP32...
275 void nodeESP32() {
276 Serial.print("CPU Temp.: ");
277 Serial.print(temperatureRead()); //returns CPU Temperature in ºC
278 Serial.print(" ºC");
279 Serial.print(" | Hall sensor: ");
280 Serial.println(hallRead()); // reads Hall sensor
281 //Serial.print(" Touch: ");
282 //Serial.println(touchRead(4));// reads touch sensor on pin 4
283 //
284 }
285 //Funcoes para definir os status entre OK, alerta e problema...
286 void BeepLongo() {
287 digitalWrite(BUZZER, HIGH);
288 delay(2000);
289 digitalWrite(BUZZER, LOW);
290 }
291 void BeepMedio() {
292 digitalWrite(BUZZER, HIGH);
293 delay(1000);
294 digitalWrite(BUZZER, LOW);
295 }
296 void BeepCurto() {
297 digitalWrite(BUZZER, HIGH);
298 delay(500);
299 digitalWrite(BUZZER, LOW);
300 }
301 void StatusOK() {
302 digitalWrite(LEDB, LOW); //Desliga o LED azul caso esteja ligado
303 digitalWrite(LEDR, LOW); //Desliga o LED vermelho caso esteja ligado
304 digitalWrite(LEDG, HIGH); //Liga o LED verde (OK)
305 digitalWrite(BUZZER, LOW); //Desliga o Buzzer caso esteja ligado
306 Serial.println("Status: OK!");
307 }
308 void StatusAlert(int Beeps) {
309 digitalWrite(LEDG, LOW); //Desliga o LED verde caso esteja ligado
310 digitalWrite(LEDR, LOW); //Desliga o LED vermelho caso esteja ligado
311 digitalWrite(LEDB, HIGH); //Liga o LED azul (Alert)
312 switch (Beeps) {
313 case 3:
314 BeepMedio();
315 BeepCurto();
316 break;
317 case 4:
318 BeepMedio();
319 break;
320 }
321 Serial.println("Status: Alerta...");
322 }
323 void StatusProblem(int Beeps) {
324 digitalWrite(LEDB, LOW); //Desliga o LED azul caso esteja ligado
325 digitalWrite(LEDG, LOW); //Desliga o LED verde caso esteja ligado
326 digitalWrite(LEDR, HIGH); //Liga o LED vermelho (Problem)
327 switch (Beeps) {
328 case 1:
329 BeepLongo();
330 BeepMedio();
331 BeepCurto();
332 break;
333 case 2:
334 BeepLongo();
335 BeepCurto();
336 break;
337 }
338 Serial.println("Status: Problema...");
339 }
340 void notificacoes() {
341 MQ2_analog_value = analogRead(MQ2_analog);
342 MQ7_analog_value = analogRead(MQ7_analog);
343 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
344 if (LeituraDHT01.temperature >= 35 || LeituraDHT01.humidity >= 90
345 || MQ2_analog_value > 1000 || MQ7_analog_value > 1000) {
346 StatusProblem(1);
347 }
348 else if (LeituraDHT01.temperature >= 30 || LeituraDHT01.humidity >= 80
349 || MQ2_analog_value > 800 || MQ7_analog_value > 800) {
350 StatusProblem(2);
351 }
352 else if (LeituraDHT01.temperature >= 29 || LeituraDHT01.humidity >= 70
353 || MQ2_analog_value > 600 || MQ7_analog_value > 600) {
354 StatusAlert(3);
355 }
356 else if (LeituraDHT01.temperature >= 28 || LeituraDHT01.humidity >= 60
357 || MQ2_analog_value > 500 || MQ7_analog_value > 500) {
358 StatusAlert(4);
359 }
360 else {
361 StatusOK();
362 }
363 }
364 //Funcao para mostrar uma mensagem inicial no OLED na inicializacao do ESP32...
365 void SetupOLED() {
366 //Limpa o display
367 display.clear();
368 //Centraliza o texto no display
369 display.setTextAlignment(TEXT_ALIGN_CENTER);
58
370 //Seleciona a fonte
371 display.setFont(ArialMT_Plain_10);
372 //Mostra informacao no "boot"
373 display.drawString(63, 10, "TCERN");
374 display.drawString(63, 20, "Modulo Principal");
375 display.drawString(63, 30, "Projeto IMD");
376 display.drawString(63, 40, "IoT DCIM");
377 display.drawString(63, 50, "v.1.0");
378 display.display();
379 }
380 //Funcao para exibir leitura do ambiente no OLED...
381 void ambienteOLED() {
382 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
383 display.clear();
384 display.setTextAlignment(TEXT_ALIGN_CENTER);
385 display.setFont(ArialMT_Plain_16);
386 display.drawString(63, 10, IP);
387 display.drawString(63, 26, zbxhostname);
388 display.drawString(63, 42, String(LeituraDHT01.temperature, 0) + String(" ºC")
389 + String(" | ") + String(LeituraDHT01.humidity, 0) + String(" %") );
390 display.display();//Escreve as informações acima no display.
391 }
392 //Funcao para exibir outras informacoes no OLED...
393 void outrosOLED() {
394 //MQ2_analog_value = analogRead(MQ2_analog);
395 //MQ7_analog_value = analogRead(MQ7_analog);
396 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
397 timeClient.update();
398 String horario = timeClient.getFormattedTime();
399 display.clear();
400 display.setTextAlignment(TEXT_ALIGN_CENTER);
401 display.setFont(ArialMT_Plain_16);
402 display.drawString(63, 10, String(LeituraDHT01.temperature, 0) + String(" ºC")
403 + String(" | ") + String(LeituraDHT01.humidity, 0) + String(" %") );
404 display.drawString(63, 26, "MQ7: " + String(MQ7_analog_value));
405 display.drawString(63, 42, "MQ2: " + String(MQ2_analog_value));
406 display.display();
407 }
408 //Funcao para enviar os dados para a nuvem ThingSpeak (cloud)...
409 void thingspeakCloud() {
410 if (client.connect(thingspeakHost, 80)) {
411 //MQ2_analog_value = analogRead(MQ2_analog);
412 //MQ7_analog_value = analogRead(MQ7_analog);
413 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
414 TS_Tempo += Loop2;
415 //Condicao para enviar as informações ao ThingSpeak a cada 2 minutos...
416 if (TS_Tempo >= 120) {
417 String thingspeak = thingspeakAPI;
59
418 thingspeak += "&amp;field1=";
419 thingspeak += String(LeituraDHT01.temperature);
420 thingspeak += "&amp;field2=";
421 thingspeak += String(LeituraDHT01.humidity);
422 //thingspeak += "&amp;field3=";
423 //thingspeak += String(MQ2_analog_value);
424 //thingspeak += "&amp;field4=";
425 //thingspeak += String(MQ7_analog_value);
426 client.print("POST /update HTTP/1.1\n");
427 client.print("Host: api.thingspeak.com\n");
428 client.print("Connection: close\n");
429 client.print("X-THINGSPEAKAPIKEY: " + thingspeakAPI + "\n");
430 client.print("Content-Type: application/x-www-form-urlencoded\n");
431 client.print("Content-Length: ");
432 client.print(thingspeak.length());
433 client.print("\n\n");
434 client.print(thingspeak);
435 Serial.println("ThingSpeak:");
436 Serial.println(thingspeak);
437 Serial.print("\n");
438 TS_Tempo = 0;
439 }
440 }
441 //Serial.println("Debug contador ThingSpeak:");
442 //Serial.println(TS_Tempo);
443 }
444 //Funcao para enviar os dados p/ o Zabbix na LAN (fog)...
445 void zabbixFog (void) {
446 Zbx_Tempo += Loop2;
447 //Condicao para enviar as informações ao Zabbix a cada 2 minutos...
448 if (Zbx_Tempo >= 120) {
449 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
450 String zbxhost = zbxhostname;
451 String itens[] = {"LeituraDHT01.temperature", "LeituraDHT01.humidity"};
452 float valores[] = {LeituraDHT01.temperature, LeituraDHT01.humidity};
453 for (int i = 0; i < (sizeof(valores) / sizeof(int)); i++) {
454 if (client.connect(zbxserver, zbxporta)) {
455 String zabbix = "";
456 zabbix += String("{\"request\":\"sender data\",\"data\":");
457 zabbix += String("[{");
458 zabbix += String("\"host\":") + String("\"") + String (zbxhost)
459 + String("\"") + String(",");
460 zabbix += String("\"key\":") + String("\"") + String (itens[i])
461 + String("\"") + String(",");
462 zabbix += String("\"value\":") + String("\"") + String (valores[i])
463 + String("\"");
464 zabbix += String("}]}");
465 Serial.println("Zabbix:");
60
466 Serial.println(zabbix);
467 client.println(zabbix);
468 zabbix = "";
469 String respostazbx = client.readStringUntil('\r');
470 Serial.println(respostazbx);
471 Zbx_Tempo = 0;
472 }
473 }
474 }
475 //Serial.println("Debug contador Zbx:");
476 //Serial.println(Zbx_Tempo);
477 }
478 //Funcao para enviar os dados para o IFTTT (WebHook)...
479 void IFFTTTReport() {
480 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
481 if (client.connect(IFTTT_host, 80)) {
482 IFTTT_Tempo += Loop;
483 //Condicao para enviar um relatorio a cada 6 horas...
484 if (IFTTT_Tempo >= 21600) {
485 //POST json.
486 String IFTTT_url = "/trigger/";
487 IFTTT_url += IFTTT_EVENT_NAME;
488 IFTTT_url += "/with/key/";
489 IFTTT_url += IFTTT_API_KEY;
490 String IFTTT_jsonObject = String("{\"value1\":\"")
491 + String(LeituraDHT01.temperature, 0)
492 + "\",\"value2\":\""
493 + String(LeituraDHT01.humidity, 0)
494 + "\",\"value3\":\"" + String(IP) + "\"}";
495 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
496 client.println(String("Host: ") + IFTTT_host);
497 client.println("Connection: close\r\nContent-Type: application/json");
498 client.print("Content-Length: ");
499 client.println(IFTTT_jsonObject.length());
500 client.println();
501 client.println(IFTTT_jsonObject);
502 Serial.println("IFTTT:");
503 Serial.println(IFTTT_jsonObject);
504 IFTTT_Tempo = 0;
505 }
506 }
507 //Serial.println("Debug contador IFTTT:");
508 //Serial.println(IFTTT_Tempo);
509 }
510 void IFTTTAlertDHT() {
511 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
512 if (client.connect(IFTTT_host, 80)) {
513 //Se temperatura estiver 30 graus e/ou umidade 90 por cento, alerta a cada 1 hora...
61
514 if (IFTTT_Alerta == 0 && (LeituraDHT01.temperature >= 30
515 || LeituraDHT01.humidity >= 90)) {
516 //POST json.
517 String IFTTT_url = "/trigger/";
518 IFTTT_url += IFTTT_EVENT_NAME2;
519 IFTTT_url += "/with/key/";
520 IFTTT_url += IFTTT_API_KEY;
521 String IFTTT_jsonObject = String("{\"value1\":\"")
522 + String(LeituraDHT01.temperature, 0)
523 + "\",\"value2\":\""
524 + String(LeituraDHT01.humidity, 0)
525 + "\",\"value3\":\""
526 + String(IP) + "\"}";
527 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
528 client.println(String("Host: ") + IFTTT_host);
529 client.println("Connection: close\r\nContent-Type: application/json");
530 client.print("Content-Length: ");
531 client.println(IFTTT_jsonObject.length());
532 client.println();
533 client.println(IFTTT_jsonObject);
534 Serial.println("IFTTT:");
535 Serial.println(IFTTT_jsonObject);
536 Serial.println("Alerta para o IFTTT enviado...");
537 IFTTT_Alerta = 1;
538 }
539 }
540 if (IFTTT_Alerta == 1) {
541 IFTTT_Intervalo += Loop;
542 }
543 if (IFTTT_Intervalo >= 3600) { // 1 hora
544 IFTTT_Intervalo = 0;
545 IFTTT_Alerta = 0;
546 }
547 //Serial.println("Debug contador IFTTT:");
548 //Serial.println(IFTTT_Alerta);
549 //Serial.println(IFTTT_Intervalo);
550 }
551 void IFTTTAlertSmoke() {
552 if (MQ_Alerta == 0 && MQ2_analog_value > 600 || MQ7_analog_value > 600) {
553 Serial.println("Status: Gas e/ou fumaca detectado...");
554 if (client.connect(IFTTT_host, 80)) {
555 //POST json.
556 String IFTTT_url = "/trigger/";
557 IFTTT_url += IFTTT_EVENT_NAME3;
558 IFTTT_url += "/with/key/";
559 IFTTT_url += IFTTT_API_KEY;
560 String IFTTT_jsonObject = String("{\"value1\":\"") + String(MQ2_analog_value)
561 + "\",\"value2\":\"" + String(MQ7_analog_value) + "\"}";
62
562 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
563 client.println(String("Host: ") + IFTTT_host);
564 client.println("Connection: close\r\nContent-Type: application/json");
565 client.print("Content-Length: ");
566 client.println(IFTTT_jsonObject.length());
567 client.println();
568 client.println(IFTTT_jsonObject);
569 Serial.println("IFTTT:");
570 Serial.println(IFTTT_jsonObject);
571 }
572 MQ_Alerta = 1;
573 }
574 if (MQ_Alerta == 1) {
575 MQ_Intervalo += Loop;
576 }
577 if (MQ_Intervalo >= 900) { //15 minutos
578 MQ_Intervalo = 0;
579 MQ_Alerta = 0;
580 }
581 //Serial.println("Debug contador MQ:");
582 //Serial.println(MQ_Alerta);
583 //Serial.println(MQ_Intervalo);
584 }
585 //Funcao para enviar SMS em caso de incidente, precisa de credito.
586 void sendSMS() {
587 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
588 if (SMS_Alerta == 0 && WiFi.status() != WL_CONNECTED && (LeituraDHT01.temperature >= 30
589 || LeituraDHT01.humidity >= 90)) {
590 Serial.println ("Sending Message");
591 MSIM800L.println("AT+CMGF=1"); //Coloca o modulo GSM em modo texto
592 delay(1000);
593 MSIM800L.println("AT+CMGS=\"" + SMSNumber1 + "\"\r"); //Numero para enviar o SMS
594 delay(1000);
595 String MensagemSMS = (String("Alerta - temperatura ")
596 + String(LeituraDHT01.temperature, 0)
597 + String(" C") + String(" e umidade ")
598 + String(LeituraDHT01.humidity, 0)
599 + String("% no node ")
600 + String(zbxhostname) + String(".") );
601 Serial.println(MensagemSMS);
602 MSIM800L.println(MensagemSMS);
603 delay(100);
604 MSIM800L.println((char)26);// ASCII para CTRL+Z
605 delay(1000);
606 Serial.println("SMS enviado com sucesso...");
607 _buffer = _readSerial();
608 SMS_Alerta = 1;
609 }
63
610 if (SMS_Alerta == 1) {
611 SMS_Intervalo += Loop;
612 }
613 if (SMS_Intervalo >= 1800) { //30 minutos
614 SMS_Intervalo = 0;
615 SMS_Alerta = 0;
616 }
617 //Serial.println("Debug contador SMS:");
618 //Serial.println(SMS_Alerta);
619 //Serial.println(SMS_Intervalo);
620 }
621 //Funcao para ligar a cobrar para os telefones em caso de incidente.
622 void callNumber() {
623 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
624 if (Call_Alerta == 0 && LeituraDHT01.temperature >= 25
625 || LeituraDHT01.humidity >= 90) {
626 delay(1000);
627 Serial.println("Ligando...");
628 MSIM800L.print (F("ATD"));
629 MSIM800L.print (CallNumber);
630 MSIM800L.print (F(";\r\n"));
631 MSIM800L.print (F("WAIT=4"));
632 MSIM800L.print (F("ATH")); //Comando para desligar
633 MSIM800L.print (F(";\r\n"));
634 Serial.println("Desligando...");
635 _buffer = _readSerial();
636 Serial.println(_buffer);
637 delay(1000);
638 Call_Alerta = 1;
639 }
640 if (Call_Alerta == 1) {
641 Call_Intervalo += Loop;
642 }
643 if (Call_Intervalo >= 900) { //15 minutos
644 Call_Intervalo = 0;
645 Call_Alerta = 0;
646 }
647 //Serial.println("Debug contador Call:");
648 //Serial.println(Call_Alerta);
649 //Serial.println(Call_Intervalo);
650 }
651 //String para ler o serial do SIM800L, usado pelas funcoes do SMS e Call...
652 String _readSerial() {
653 _timeout = 0;
654 while (!MSIM800L.available() && _timeout < 12000 )
655 {
656 delay(13);
657 _timeout++;
64
658 }
659 if (MSIM800L.available()) {
660 return MSIM800L.readString();
661 }
662 }
663 * /
664 void MQTT_connect() {
665 int8_t ret;
666
667 // Stop if already connected.
668 if (mqtt.connected()) {
669 return;
670 }
671
672 Serial.print(F("Connecting to MQTT... "));
673
674 uint8_t retries = 3;
675 while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
676 Serial.println(mqtt.connectErrorString(ret));
677 Serial.println(F("Retrying MQTT connection in 5 seconds..."));
678 mqtt.disconnect();
679 delay(5000); // wait 5 seconds
680 retries--;
681 if (retries == 0) {
682 // basically die and wait for WDT to reset me
683 while (1);
684 }
685 }
686 Serial.println(F("MQTT Connected!"));
687 }
688 void MQTT_publish() {
689 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
690 //MQ2_analog_value = analogRead(MQ2_analog);
691 //MQ7_analog_value = analogRead(MQ7_analog);
692 //mqttmq2.publish(MQ2_analog_value);
693 //mqttmq7.publish(MQ7_analog_value);
694 mqttdht1t.publish(LeituraDHT01.temperature);
695 mqttdht1u.publish(LeituraDHT01.humidity);
696 /*if (! mqttcpu.publish(temperatureRead()) && mqttmq2.publish(MQ2_analog_value)
697 && mqttmq7.publish(MQ7_analog_value) ) {
698 Serial.println(F("MQTT Adafruit.IO: Publish Failed."));
699 } else {
700 Serial.println(F("MQTT Adafruit.IO: Publish Success!"));
701 delay(500);
702 }
703 delay(10000);*/
704 }
705 void MQTT_Subscribe () {
65
706 mqtt.subscribe(&mqttled);
707 Adafruit_MQTT_Subscribe *subscription;
708 while ((subscription = mqtt.readSubscription(5000))) {
709 // Check if its the onoff button feed
710 if (subscription == &mqttled) {
711 Serial.print(F("On-Off button: "));
712 Serial.println((char *)mqttled.lastread);
713
714 if (strcmp((char *)mqttled.lastread, "ON") == 0) {
715 digitalWrite(LEDR, LOW);
716 }
717 if (strcmp((char *)mqttled.lastread, "OFF") == 0) {
718 digitalWrite(LEDR, HIGH);
719 }
 }
 }
}