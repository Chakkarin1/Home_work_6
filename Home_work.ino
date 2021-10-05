#include <PubSubClient.h>  
#include <WiFiClientSecure.h>
#include <WiFi.h>   

TaskHandle_t t0;
TaskHandle_t t1;

const char* ssid = "";       
const char* wifi_password = "";

const char* mqtt_server = "";  
const char* mqtt_username = ""; 
const char* mqtt_password = "";  
const char* clientID = "";  

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883,wifiClient); 

#define MSG_BUFFER_SIZE  (50)
long trylastReconnect = 0;
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;


void setup() {
  
 pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

    //create a task to handle Mqtt (core 0)
  xTaskCreatePinnedToCore(
    tMqtt,        
    "Mqtt",       
    10000,        
    NULL,         
    1,            
    &t0,          
    1);           
  delay(500);

  // create a task to handle led LED_BUILTIN (core 1)
  xTaskCreatePinnedToCore(
    tLedFunc,     
    "Led",        
    10000,        
    NULL,         
    0,            
    &t1,          
    0);           
  delay(500);
}


void loop() {
  
  Serial.print("loop running on core ");
  Serial.println(xPortGetCoreID());
  delay(1000);

}


void tWifi(){    
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);
  client.setServer(mqtt_server, 1883);

 while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);   
    //led turn off when can't connect Wifi access point
    Serial.println("turn off led when cannot connect Wifi");
    delay(500);
    Serial.print(".");
    WiFi.begin(ssid, wifi_password);
  client.setServer(mqtt_server, 1883);
 }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   //connect MQTT Broker
   if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected MQTT Broker!");
  }
  else {
    Serial.println("Connection MQTT Broker fail.");
  }
}
  

boolean reconnect() {

  if (!client.connected() ) {
    Serial.println("connecting...");
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
      Serial.println("Connected MQTT broker waiting publish");
      
      client.subscribe("#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5sec");
    }
  }
  return client.connected();
}


void tMqtt(void *params){
  // local Variables
  bool lastState = false;
  
  Serial.print("tButtonFunc running on core ");
  Serial.println(xPortGetCoreID());

  // loop
  while (true) {
    

    if(WiFi.status() == WL_CONNECTED){
    Serial.println("Connect WIFI, turn on led 500ms ");  
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.println("Connect WIFI, turn off led 500ms ");  
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500); 
    }
    if(client.connected()){
    digitalWrite(LED_BUILTIN, LOW);  
    Serial.println("Connect MQTT, Turn on led ");
    }

  } 
}
  

// tLedFunc: blinks every 1000ms
void tLedFunc(void *params) {
  // local variable
  
  // setup  
  Serial.print("tLedFunc running on core ");
  Serial.println(xPortGetCoreID());
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // connect WiFi
  tWifi();
  
  // Connect MQTT 
  while (true) {
    if (!client.connected()) {
    long now = millis();  
    if (now - trylastReconnect > 5000) {
      trylastReconnect = now;
      Serial.println("Counting.. ,not connect");
      Serial.println("Counter is reset : 0");
      Serial.print("Counter Change to : ");
      Serial.println(trylastReconnect);
      // try to reconnect
      if (reconnect()) {
        trylastReconnect = 0;
        Serial.println("Reconnecting");   
      }
    }
  }
    else if (client.connected()) {
    client.loop();
    Serial.println("loop core1 : Connected...");
    Serial.print("IP address:  ");
    Serial.println(WiFi.localIP());       
     delay(50);
  }
  delay(100);
  }
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
     client.disconnect();
}
