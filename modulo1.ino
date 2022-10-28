 //Inclusao das bibliotecas principais
 #include <DHTesp.h> //Carrega a biblioteca do sensor de temperatura e umidade DHT.
 #include <Wire.h> //Carrega a biblioteca para a comunicacao I2C.
 #include <WiFi.h> //Carrega a biblioteca da rede sem fio.
 #include <String.h> //Carrega a biblioteca usada pelo SIM800L.
 #include <NTPClient.h> //Carrega a biblioteaca usada para atualizar o relógio.
 //Definição dos tempos
 53 unsigned long previousMillis2 = 0;
 54 unsigned long previousMillis3 = 0;
 56 const long interval30 = 30000;
 57 const long interval60 = 60000;
 //Definicoes dos pinos no ESP32
 #define DHT01data 26 //Cria um alias para o modulo DHT1 no pino 26 do ESP32.
 //
 DHTesp dht01; //Cria um objeto do tipo DTHesp para o modulo DHT1.

 //Define as variaveis da conexao sem fio (WiFi)
 WiFiClient client; //Inicializa a biblioteca client
 #define WIFI_SSID "XXXX" //Nome da rede (SSID)
 #define WIFI_PASS "XXXX" //Senha de rede sem fio
 String IP;
 //
 //Define as variaveis e dados do Zabbix (trapper)
 float temperature;
 float humidity;
 const char* zbxserver = "0.0.0.0"; //IP do Zabbix.
 const char* zbxhostname = "nodeiot00"; //Host no Zabbix.
 int zbxporta = 10051;
 //
 void setup() {
    Serial.begin(115200); //Inicia o terminal para a leitura via console.
    while (! Serial);
        Serial.println("Inicializando o ESP32...");
        SerialBT.begin(zbxhostname); //Bluetooth nome visível.
        Serial.println("Hostname: " + String(zbxhostname));
        timeClient.begin(); //Inicia o cliente NTP
        pinMode(DHT01data, INPUT); //Define que o pino do sensor DHT sera um pino de entrada.
        dht01.setup(DHT01data, DHTesp::DHT11); //Inicializacao do sensor DHT01.
        wifiConnect(); //Conecta ao WiFi
 }

 void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis2 >= interval30) {
        previousMillis2 = currentMillis;
        wifiConnect(); //Conecta a rede sem fio
        DHT1(); //Chama a funcao do sensor DHT para mostrar no serial
    }

    if (currentMillis - previousMillis3 >= interval60) {
        previousMillis3 = currentMillis;
        zabbixFog(); // Envia os dados para a fog (Zabbix)
    }
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
 //Funcao para ler a temperatura e a umidade...
 void DHT1() {
    TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
    Serial.println("Temperatura: " + String(LeituraDHT01.temperature, 0) + String(" ºC")
    + " | Umidade: " + String(LeituraDHT01.humidity, 0) + String(" %"));
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
 }