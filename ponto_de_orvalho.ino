1 /**************************************************************/
2 /* Example how to read DHT sensors from an ESP32 using multi- */
3 /* tasking. */
4 /* This example depends on the ESP32Ticker library to wake up */
5 /* the task every 20 seconds */
6 /* Please install Ticker-esp32 library first */
7 /* bertmelis/Ticker-esp32 */
8 /* https://github.com/bertmelis/Ticker-esp32 */
9 /**************************************************************/
10
11 DHTesp dht01;
12
13 float heatIndex;
14 float dewPoint;
15
16 void tempTask(void *pvParameters);
17 bool getTemperature();
18 void triggerGetTemp();
19
20 /** Task handle for the light value read task */
21 TaskHandle_t tempTaskHandle = NULL;
22 /** Ticker for temperature reading */
23 Ticker tempTicker;
24 /** Comfort profile */
25 ComfortState cf;
66
26 /** Flag if task should run */
27 bool tasksEnabled = false;
28 /** Pin number for DHT11 data pin */
29 int dhtPin = 26;
30
31 /**
32 * initTemp
33 * Setup DHT library
34 * Setup task and timer for repeated measurement
35 * @return bool
36 * true if task and timer are started
37 * false if task or timer couldn't be started
38 */
39 bool initTemp() {
40 byte resultValue = 0;
41 // Initialize temperature sensor
42 dht01.setup(dhtPin, DHTesp::DHT11);
43 Serial.println("DHT initiated");
44
45 // Start task to get temperature
46 xTaskCreatePinnedToCore(
47 tempTask, /* Function to implement the task */
48 "tempTask ", /* Name of the task */
49 4000, /* Stack size in words */
50 NULL, /* Task input parameter */
51 5, /* Priority of the task */
52 &tempTaskHandle, /* Task handle. */
53 1); /* Core where the task should run */
54
55 if (tempTaskHandle == NULL) {
56 Serial.println("Failed to start task for temperature update");
57 return false;
58 } else {
59 // Start update of environment data every 20 seconds
60 tempTicker.attach(20, triggerGetTemp);
61 }
62 return true;
63 }
64
65 /**
66 * triggerGetTemp
67 * Sets flag dhtUpdated to true for handling in loop()
68 * called by Ticker getTempTimer
69 */
70 void triggerGetTemp() {
71 if (tempTaskHandle != NULL) {
72 xTaskResumeFromISR(tempTaskHandle);
73 }
67
74 }
75
76 /**
77 * Task to reads temperature from DHT11 sensor
78 * @param pvParameters
79 * pointer to task parameters
80 */
81 void tempTask(void *pvParameters) {
82 Serial.println("tempTask loop started");
83 while (1) // tempTask loop
84 {
85 if (tasksEnabled) {
86 // Get temperature values
87 getTemperature();
88 }
89 // Got sleep again
90 vTaskSuspend(NULL);
91 }
92 }
93
94 /**
95 * getTemperature
96 * Reads temperature from DHT11 sensor
97 * @return bool
98 * true if temperature could be aquired
99 * false if aquisition failed
100 */
101 bool getTemperature() {
102 // Reading temperature for humidity takes about 250 milliseconds!
103 // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
104 TempAndHumidity LeituraDHT01 = dht01.getTempAndHumidity();
105 // Check if any reads failed and exit early (to try again).
106 if (dht01.getStatus() != 0) {
107 Serial.println("DHT11 error status: " + String(dht01.getStatusString()));
108 return false;
109 }
110
111 heatIndex = dht01.computeHeatIndex(LeituraDHT01.temperature, LeituraDHT01.humidity);
112 dewPoint = dht01.computeDewPoint(LeituraDHT01.temperature, LeituraDHT01.humidity);
113 float cr = dht01.getComfortRatio(cf, LeituraDHT01.temperature, LeituraDHT01.humidity);
114
115 String comfortStatus;
116 switch(cf) {
117 case Comfort_OK:
118 comfortStatus = "OK";
119 break;
120 case Comfort_TooHot:
121 comfortStatus = "TooHot";
68
122 break;
123 case Comfort_TooCold:
124 comfortStatus = "TooCold";
125 break;
126 case Comfort_TooDry:
127 comfortStatus = "TooDry";
128 break;
129 case Comfort_TooHumid:
130 comfortStatus = "TooHumid";
131 break;
132 case Comfort_HotAndHumid:
133 comfortStatus = "HotAndHumid";
134 break;
135 case Comfort_HotAndDry:
136 comfortStatus = "HotAndDry";
137 break;
138 case Comfort_ColdAndHumid:
139 comfortStatus = "ColdAndHumid";
140 break;
141 case Comfort_ColdAndDry:
142 comfortStatus = "ColdAndDry";
143 break;
144 default:
145 comfortStatus = "Unknown:";
146 break;
147 };
148
149 Serial.println(" T:" + String(LeituraDHT01.temperature) + " H:" + String(LeituraDHT01.humidity) 150 console.println(" T:" + String(LeituraDHT01.temperature) + " H:" + String(LeituraDHT01.hum151 return true;
152 }
