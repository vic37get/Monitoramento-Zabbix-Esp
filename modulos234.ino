//Inclusão das bibliotecas
2 #include <DHTesp.h> //Carrega a biblioteca usada pelo sensor DHT.
3 #include <WiFi.h> //Carrega a biblioteca da rede sem fio.
4 //
5 //Definições dos pinos no ESP32
6 #define IR_TX_LED 25 //Cria um alias para o LED IR no pino 25 do ESP32.
7 #define LEDR 12 //Cria um alias para o LED Vermelho no pino 12 do ESP32.
8 #define LEDG 14 //Cria um alias para o LED Verde no pino 14 do ESP32.
9 #define LEDB 32 //Cria um alias para o LED Azul no pino 32 do ESP32.
10 #define BUZZER 33 //Cria um alias para o Buzzer no pino 33 do ESP32.
11 #define DHT01data 26 //Cria um alias para o módulo DHT1 no pino 26 do ESP32.
69
12 #define DHT02data 27 //Cria um alias para o módulo DHT2 no pino 27 do ESP32.
13 //
14 DHTesp dht01; //Cria um objeto do tipo DTHesp para o DHT1 no IoT ou frente do rack.
15 DHTesp dht02; //Cria um objeto do tipo DTHesp para o DHT2 na boca do ar ou traseira do rack.
16 //
17 //Define as variáveis e APIs do FTTT
18 String IFTTT_API_KEY = "XXXX"; //Define a API do IFTTT
19 String IFTTT_EVENT_NAME = "ReportESP32"; // Define o evento do IFTTT
20 const char* IFTTT_host = "maker.ifttt.com";
21 //
22 //Define as variáveis e APIs do ThingSpeak
23 //String thingspeakAPI = "XXXX"; //API do Módulo IoT 02.
24 //String thingspeakAPI = "XXXX"; //API do Módulo IoT 03.
25 //String thingspeakAPI = "XXXX"; //API do Módulo IoT 04.
26 String thingspeakAPI = "XXXX"; //API do Módulo IoT 05.
27 const char* thingspeakHost = "api.thingspeak.com"; //URL da API do ThingSpeak.
28 //
29 //Define as variáveis da conexão sem fio (wifi)
30 #define WIFI_SSID "XXXX" //Nome da rede (SSID)
31 #define WIFI_PASS "XXXX" // senha de rede sem fio
32 String IP;
33 WiFiClient client; //Inicializa o Wifi...
34 //
35 //Define as variaveis e dados do Zabbix (trapper)
36 float temperature;
37 float humidity;
38 const char* zbxserver = "0.0.0.0"; //IP do Zabbix.
39 const char* zbxhostname = "nodeiot01"; //Host no Zabbix.
40 int zbxporta = 10051;
41 //
42 //Outros
43 // Contador de tempo para a funcao do IFTTT...
44 int IFTTT_Tempo = 0;
45 int IFTTT_Intervalo = 0;
46 int IFTTT_Alerta = 0;
47 int TS_Tempo = 0;
48 int Zbx_Tempo = 0;
49 int IR_Tempo = 0;
50 int Loop = 30;
51 int Loop2 = 60;
52 unsigned long previousMillis = 0;
53 unsigned long previousMillis2 = 0;
54 unsigned long previousMillis3 = 0;
55 const long interval10 = 10000;
56 const long interval30 = 30000;
57 const long interval60 = 60000;
58 //
59 void setup() {
70
60 // put your setup code here, to run once:
61 Serial.begin(115200); //Inicia o terminal para a leitura via console.
62 Serial.println("Inicializando o ESP32...");
63 pinMode(BUZZER, OUTPUT); //Define que o Buzzer sera um pino de saída.
64 pinMode(LEDR, OUTPUT); //Define que o pino do LEDR sera um pino de saída.
65 pinMode(LEDG, OUTPUT); //Define que o pino do LEDG sera um pino de saída.
66 pinMode(LEDB, OUTPUT); //Define que o pino do LEDB sera um pino de saída.
67 digitalWrite(IR_TX_LED, LOW); //Define que o pino está desligado.
68 digitalWrite(BUZZER, LOW);//Define que o pino está desligado.
69 digitalWrite(LEDR, LOW);//Define que o pino está desligado.
70 digitalWrite(LEDG, LOW);//Define que o pino está desligado.
71 digitalWrite(LEDB, LOW);//Define que o pino está desligado.
72 pinMode(DHT01data, INPUT); //Define que o pindo do sensor DHT será um pino de entrada.
73 pinMode(DHT02data, INPUT); //Define que o pindo do sensor DHT será um pino de entrada.
74 dht01.setup(DHT01data, DHTesp::DHT11); //Inicialização do sensor DHT01.
75 dht02.setup(DHT02data, DHTesp::DHT11); //Inicialização do sensor DHT02.
76 testeLEDs();
77 BeepLongo();
78 BeepMedio();
79 BeepCurto();
80 wifiConnect(); //Conecta ao WiFi
81 }
82 void loop() {
83 unsigned long currentMillis = millis();
84 // Chama as seguintes funcoes:
85 if (currentMillis - previousMillis >= interval10) {
86 previousMillis = currentMillis;
87 notificacoes();
88 //Serial.println("Entrou no loop de 10s");
89 }
90 if (currentMillis - previousMillis2 >= interval30) {
91 previousMillis2 = currentMillis;
92 IFFTTTReport(); //IFTTT report a cada 6 horas
93 IFTTTAlert(); //IFTTT alerta em caso de incidente
94 //Serial.println("Entrou no loop de 30s");
95 }
96 if (currentMillis - previousMillis3 >= interval60) {
97 previousMillis3 = currentMillis;
98 ThingSpeakCloud(); //ThingSpeak (cloud)
99 ZabbixFog(); //Zabbix (Fog)
100 wifiConnect(); //Conecta ao WiFi
101 //Serial.println("Entrou no loop de 60s");
102 }
103 }
104 void testeLEDs() {
105 digitalWrite(LEDB, HIGH); //Liga o LED azul.
106 delay(2000);
107 digitalWrite(LEDB, LOW); //Desliga o LED azul.
71
108 delay(100);
109 digitalWrite(LEDG, HIGH); //Liga o LED verde.
110 delay(2000);
111 digitalWrite(LEDG, LOW); //Desliga o LED verde.
112 delay(100);
113 digitalWrite(LEDR, HIGH); //Liga o LED vermelho.
114 delay(2000);
115 digitalWrite(LEDR, LOW); //Desliga o LED vermelho.
116 }
117 //Função para conectar a rede Wifi...
118 void wifiConnect() {
119 int cont = 0;
120 if (WiFi.status() != WL_CONNECTED) {
121 Serial.print("Tentando conectar na rede sem fio ");
122 Serial.println(WIFI_SSID);
123 do {
124 WiFi.begin(WIFI_SSID, WIFI_PASS);
125 cont = cont + 1;
126 delay(2000);
127 } while (WiFi.status() != WL_CONNECTED && cont != 2);
128 }
129 else {
130 Serial.print("SSID: ");
131 Serial.println(WIFI_SSID);
132 Serial.print("IP: ");
133 Serial.println(WiFi.localIP());
134 Serial.print("MAC: ");
135 Serial.println(WiFi.macAddress()); //retorna o endereço MAC do node.
136 Serial.println("Hostname: " + String(zbxhostname));
137 IP = WiFi.localIP().toString();
138 }
139 }
140 void BeepLongo() {
141 digitalWrite(BUZZER, HIGH);
142 delay(2000);
143 digitalWrite(BUZZER, LOW);
144 }
145 void BeepMedio() {
146 digitalWrite(BUZZER, HIGH);
147 delay(1000);
148 digitalWrite(BUZZER, LOW);
149 }
150 void BeepCurto() {
151 digitalWrite(BUZZER, HIGH);
152 delay(500);
153 digitalWrite(BUZZER, LOW);
154 }
155 void StatusOK() {
72
156 digitalWrite(LEDB, LOW); //Desliga o LED azul caso esteja ligado.
157 digitalWrite(LEDR, LOW); //Desliga o LED vermelho caso esteja ligado.
158 digitalWrite(LEDG, HIGH); //Liga o LED verde (OK).
159 digitalWrite(BUZZER, LOW); //Desliga o Buzzer caso esteja ligado.
160 Serial.println("Status: OK!");
161 }
162 void StatusAlert(int Beeps) {
163 digitalWrite(LEDG, LOW); //Desliga o LED verde caso esteja ligado.
164 digitalWrite(LEDR, LOW); //Desliga o LED vermelho caso esteja ligado.
165 digitalWrite(LEDB, HIGH); //Liga o LED azul (Alert).
166 switch (Beeps) {
167 case 3:
168 BeepMedio();
169 BeepCurto();
170 break;
171 case 4:
172 BeepMedio();
173 break;
174 }
175 Serial.println("Status: Alerta...");
176 }
177 void StatusProblem(int Beeps) {
178 digitalWrite(LEDB, LOW); //Desliga o LED azul caso esteja ligado.
179 digitalWrite(LEDG, LOW); //Desliga o LED verde caso esteja ligado.
180 digitalWrite(LEDR, HIGH); //Liga o LED vermelho (Problem).
181 switch (Beeps) {
182 case 1:
183 BeepLongo();
184 BeepMedio();
185 BeepCurto();
186 break;
187 case 2:
188 BeepLongo();
189 BeepCurto();
190 break;
191 }
192 Serial.println("Status: Problema...");
193 }
194 void notificacoes() {
195 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
196 TempAndHumidity LeituraDHT02 = dht02.getTempAndHumidity();
197 Serial.println("DHT1 - Temperatura: " + String(LeituraDHT01.temperature, 0) + String(" ºC")
198 + " | Umidade: " + String(LeituraDHT01.humidity, 0) + String(" %"));
199 Serial.println("DHT2 - Temperatura: " + String(LeituraDHT02.temperature, 0) + String(" ºC")
200 + " | Umidade: " + String(LeituraDHT02.humidity, 0) + String(" %"));
201 if (LeituraDHT01.temperature >= 35 || LeituraDHT01.humidity >= 90) {
202 StatusProblem(1);
203 }
73
204 else if (LeituraDHT01.temperature >= 32 || LeituraDHT01.humidity >= 80) {
205 StatusProblem(2);
206 }
207 else if (LeituraDHT01.temperature >= 31 || LeituraDHT01.humidity >= 70) {
208 StatusAlert(3);
209 }
210 else if (LeituraDHT01.temperature >= 30 || LeituraDHT01.humidity >= 60) {
211 StatusAlert(4);
212 }
213 else {
214 StatusOK();
215 }
216 }
217 //Funcao para enviar os dados para a nuvem ThingSpeak (cloud)...
218 void ThingSpeakCloud() {
219 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
220 TempAndHumidity LeituraDHT02 = dht02.getTempAndHumidity();
221 if (client.connect(thingspeakHost, 80)) {
222 TS_Tempo += Loop2;
223 //Condicao para enviar as informações ao ThingSpeak a cada 4 minutos...
224 if (TS_Tempo >= 240) {
225 String thingspeak = thingspeakAPI;
226 thingspeak += "&amp;field1=";
227 thingspeak += String(LeituraDHT01.temperature);
228 thingspeak += "&amp;field2=";
229 thingspeak += String(LeituraDHT01.humidity);
230 thingspeak += "&amp;field3=";
231 thingspeak += String(LeituraDHT02.temperature);
232 thingspeak += "&amp;field4=";
233 thingspeak += String(LeituraDHT02.humidity);
234 thingspeak += "\r\n\r\n";
235 client.print("POST /update HTTP/1.1\n");
236 client.print("Host: api.thingspeak.com\n");
237 client.print("Connection: close\n");
238 client.print("X-THINGSPEAKAPIKEY: " + thingspeakAPI + "\n");
239 client.print("Content-Type: application/x-www-form-urlencoded\n");
240 client.print("Content-Length: ");
241 client.print(thingspeak.length());
242 client.print("\n\n");
243 client.print(thingspeak);
244 Serial.println("ThingSpeak:");
245 Serial.println(thingspeak);
246 Serial.print("\n");
247 TS_Tempo = 0;
248 }
249 }
250 else {
251 Serial.println("Dados não enviados para a cloud (ThingSpeak).");
74
252 }
253 Serial.println("Debug contador ThingSpeak:");
254 Serial.println(TS_Tempo);
255 }
256 //Funcao para enviar os dados p/ o Zabbix na LAN (fog)...
257 void ZabbixFog (void) {
258 Zbx_Tempo += Loop2;
259 //Condicao para enviar as informações ao Zabbix a cada 3 minutos...
260 if (Zbx_Tempo >= 180) {
261 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
262 TempAndHumidity LeituraDHT02 = dht02.getTempAndHumidity();
263 String zbxhost = zbxhostname;
264 String itens[] = {"LeituraDHT01.temperature", "LeituraDHT01.humidity",
265 "LeituraDHT02.temperature", "LeituraDHT02.humidity"
266 };
267 float valores[] = {LeituraDHT01.temperature, LeituraDHT01.humidity,
268 LeituraDHT02.temperature, LeituraDHT02.humidity
269 };
270 for (int i = 0; i < (sizeof(valores) / sizeof(int)); i++) {
271 if (client.connect(zbxserver, zbxporta)) {
272 String zabbix = "";
273 zabbix += String("{\"request\":\"sender data\",\"data\":");
274 zabbix += String("[{");
275 zabbix += String("\"host\":") + String("\"") + String (zbxhost)
276 + String("\"") + String(",");
277 zabbix += String("\"key\":") + String("\"") + String (itens[i])
278 + String("\"") + String(",");
279 zabbix += String("\"value\":") + String("\"") + String (valores[i])
280 + String("\"");
281 zabbix += String("}]}");
282 Serial.println("Zabbix:");
283 Serial.println(zabbix);
284 client.println(zabbix);
285 zabbix = "";
286 String respostazbx = client.readStringUntil('\r');
287 Serial.println(respostazbx);
288 Zbx_Tempo = 0;
289 }
290 }
291 }
292 Serial.println("Debug contador Zbx:");
293 Serial.println(Zbx_Tempo);
294 }
295 //Envia os dados para o IFTTT (WebHook) para enviar p/ email, Twitter e Slack ou outro...
296 void IFFTTTReport() {
297 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
298 TempAndHumidity LeituraDHT02 = dht02.getTempAndHumidity();
299 if (client.connect(IFTTT_host, 80)) {
75
300 IFTTT_Tempo += Loop;
301 //Condicao para enviar um relatorio a cada 6 horas...
302 if (IFTTT_Tempo >= 21600) {
303 //POST json.
304 String IFTTT_url = "/trigger/";
305 IFTTT_url += IFTTT_EVENT_NAME;
306 IFTTT_url += "/with/key/";
307 IFTTT_url += IFTTT_API_KEY;
308 String IFTTT_jsonObject = String("{\"value1\":\"") + String(LeituraDHT01.temperature, 0)
309 + "\",\"value2\":\"" + String(LeituraDHT01.humidity, 0)
310 + "\",\"value3\":\"" + String(IP) + "\"}";
311 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
312 client.println(String("Host: ") + IFTTT_host);
313 client.println("Connection: close\r\nContent-Type: application/json");
314 client.print("Content-Length: ");
315 client.println(IFTTT_jsonObject.length());
316 client.println();
317 client.println(IFTTT_jsonObject);
318 Serial.println("IFTTT:");
319 Serial.println(IFTTT_jsonObject);
320 IFTTT_Tempo = 0;
321 }
322 }
323 Serial.println("Debug contador IFTTT:");
324 Serial.println(IFTTT_Tempo);
325 }
326 void IFTTTAlert() {
327 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
328 if (client.connect(IFTTT_host, 80)) {
329 if (IFTTT_Alerta == 0 && (LeituraDHT01.temperature >= 28 || LeituraDHT01.humidity >= 90)) {
330 //POST json.
331 String IFTTT_url = "/trigger/";
332 IFTTT_url += IFTTT_EVENT_NAME;
333 IFTTT_url += "/with/key/";
334 IFTTT_url += IFTTT_API_KEY;
335 String IFTTT_jsonObject = String("{\"value1\":\"") + String(LeituraDHT01.temperature, 0)
336 + "\",\"value2\":\"" + String(LeituraDHT01.humidity, 0)
337 + "\",\"value3\":\"" + String(IP) + "\"}";
338 client.println(String("POST ") + IFTTT_url + " HTTP/1.1");
339 client.println(String("Host: ") + IFTTT_host);
340 client.println("Connection: close\r\nContent-Type: application/json");
341 client.print("Content-Length: ");
342 client.println(IFTTT_jsonObject.length());
343 client.println();
344 client.println(IFTTT_jsonObject);
345 Serial.println("IFTTT:");
346 Serial.println(IFTTT_jsonObject);
347 IFTTT_Alerta = 1;
76
348
}
349
}
350 if (IFTTT_Alerta ==
1) {
351 IFTTT_Intervalo += Loop;
352
}
353 if (IFTTT_Intervalo >= 1800) { //30 minutos
354 IFTTT_Intervalo
=
0
;
355 IFTTT_Alerta
=
0
;
356
}
357 Serial.println("Debug contador IFTTT:");
358 Serial.println(IFTTT_Alerta);
359 Serial.println(IFTTT_Intervalo);
360
}