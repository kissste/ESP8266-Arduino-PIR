/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include "DHT.h"
#include "_passwords.h"

#define PIRPIN 5 // This is where PIR is connected

#define DHTPIN 2 // This is where DHT is connected
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE, 15);
float t_old = 99.00; // Dummy, unset temperature
float h_old = 99.00; // Dummy, unset humidity

//const char* ssid = "wifi-hostname"; 
//const char* password = "wifi-password";

//const char* host = "test.com";
const char* streamId = "pir01";
const char* privateKey = "pir01";

const char* tick = "tick";
const char* up = "up";
const char* start = "start";

volatile int value = 0;
volatile bool pirState = 0;
volatile unsigned long lastTimeUp = 0;
volatile unsigned long lastTimeUpSent = 0;
volatile unsigned long lastTimeTickSent = 0;
volatile unsigned long randomTick = random(1,5);

void setup() {
  Serial.begin(115200);
  delay(10);
  
  dht.begin();
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && (retries < 10)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  delay(10);

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
    
  unsigned long time = millis();
  lastTimeTickSent = time;
  lastTimeUpSent = time;
  lastTimeUp = time;
  notify((String) start);
  
  pinMode(PIRPIN, INPUT_PULLUP);
  delay(500);
}

void loop() {
  unsigned long time = millis();
  bool pir = digitalRead(PIRPIN);
  //analogRead(A0);
  //digitalRead(PIRPIN);
  Serial.println(pir);    
  if (pir && !pirState) {
    pirState = pir;
    lastTimeUp = time;  
    if ((lastTimeUp - lastTimeUpSent) > 30*1000UL) {
      lastTimeUpSent = time;  
      ++value;
      notify((String)up);
      Serial.println("notify");    
    }
    Serial.println(up);    
  } else if (!pir && pirState) {
    pirState = pir;
  } else if ((time - lastTimeTickSent > 5*60*1000UL) && (time - lastTimeUpSent > 5*60*1000UL)) //1*60*1,000,000micros=10min
  {
    randomTick = random(5,10);
    lastTimeTickSent = time;
    ++value;  
    notify((String)tick);
    Serial.println(tick); 
  }
  delay(2000); //in 2*1000millis = 2s
}

String readDHT() {
    float t = dht.readTemperature();
    if (isnan(t)){
      Serial.println("Failed to read Temperature from DHT sensor!");
    } else {
      t_old = t;
    }
    String tempValue = "&t=";
    tempValue += t_old;    
    t = dht.readHumidity();
    if (isnan(t)){
      Serial.println("Failed to read Humidity from DHT sensor!");
    } else {
      h_old = t;
    }
    tempValue += "&h=";
    tempValue += h_old;       
    //notify(tempValue);    
    //Serial.println(tempValue);    
    return tempValue;
}

void notify(String msg) {
  //Serial.print("connecting to ");
  //Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    //Serial.println("connection failed");
    ESP.reset();
    return;
  }
  
  // We now create a URI for the request
  String url = "/HOME.php?";
  url += streamId;
  url += "&m=";
  url += msg;
  url += readDHT();
  url += "&v=";
  url += value;
  
  //Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  //while(client.available()){
  //  String line = client.readStringUntil('\r');
  //  Serial.print(line);
  //}
  
  //Serial.println();
  //Serial.println("closing connection");
}  
