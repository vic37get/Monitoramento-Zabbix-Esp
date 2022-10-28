#include "ESP8266ZabbixSender.h"
#include <Base64.h>
#include <DHT.h>
#include <Wire.h> //Carrega a biblioteca para a comunicacao I2C.
#include <WiFi.h> //Carrega a biblioteca da rede sem fio.
#include <String.h> //Carrega a biblioteca usada pelo SIM800L.
#include <NTPClient.h> //Carrega a biblioteaca usada para atualizar o relógio.

ESP8266ZabbixSender zSender;

//Definição dos tempos
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
const long interval30 = 30000;
const long interval60 = 60000;
int Loop2 = 60;

/* WiFi settings */
String ssid = "TESTE";
String pass = "TESTE#test3";
//

//Define as variaveis e dados do Zabbix (trapper)
WiFiClient client; //Inicializa a biblioteca client
const char* zbxserver = "10.2.50.71"; //IP do Zabbix.
const char* zbxhostname = "ESP"; //Host no Zabbix.
int zbxporta = 10051;

/*Temperatura DHT11*/
float temp;

/* Zabbix server setting */
#define SERVERADDR 10, 2, 50, 71 // IP Address example 192.168.0.123
#define ZABBIXPORT 10051      
#define ZABBIXAGHOST "ESP"
#define ZABBIX_KEY "ServerRoom"


// DHT11
#define DHTPIN 4 //Pino digital D2 (GPIO5) DHT11
#define DHTTYPE DHT11 //Tipo do sensor DHT11

DHT dht(DHTPIN, DHTTYPE);

// WiFi connectivity checker
boolean checkConnection() {
  Serial.println(" ");
  Serial.println("Checking Wifi...");
  while (true) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wifi connected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      return (true);
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println("Timed out.");
  return false;
}

void zabbixFog (void) {
   Zbx_Tempo += Loop2;
   //Condicao para enviar as informações ao Zabbix a cada 2 minutos...
   if (Zbx_Tempo >= 120) {
      //TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
      String zbxhost = zbxhostname;
      String itens[] = {"temperature"};
      float valores[] = {temp};
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

void setup() {

  // Initialize terminal
  Serial.begin(115200);
  Serial.println();

  // Configure WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  // Wait for connectivity
  while (!checkConnection());

  // Initialize Zabbix sender
  zSender.Init(IPAddress(SERVERADDR), ZABBIXPORT, ZABBIXAGHOST);
}

void loop() {
  // Get temperature from DHT11
  temp = dht.readTemperature();
  Serial.print("Temperatura: ");
  Serial.println(temp);

  //Pega o tempo atual
  unsigned long currentMillis = millis();

  // Check connectivity
  checkConnection();

  //Enviando os dados para o Zabbix a cada 60 segundos
  if (currentMillis - previousMillis3 >= interval60) {
      previousMillis3 = currentMillis;
      zabbixFog(); // Envia os dados para a fog (Zabbix)
   }
   
  // If temperature is good - send it to Zabbix
  /*
  if (! (temp == 85.0 || temp == (-127.0))) {
    zSender.ClearItem();              
    zSender.AddItem(ZABBIX_KEY, temp); 
    if (zSender.Send() == EXIT_SUCCESS) {
      Serial.println("ZABBIX SEND: OK");
    } else {
      Serial.println("ZABBIX SEND: Not Good");
    }
  }
  */
  delay(1000); 
}