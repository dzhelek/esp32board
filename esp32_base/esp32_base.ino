#include <Arduino_JSON.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "WiFiCredentials.h"

//temp_sens
#define THERMISTORNOMINAL 10000
#define TEMPERATURENOMINAL 15
#define BCOEFFICIENT 3950
#define SERIESRESISTOR 10000
#define ledPin 32
#define tempPin 33
#define wlPin 35
#define motorPin 34
#define heaterPin 22


int wl_sens = 0;

float average;

IPAddress ip;

int measure_temp();
int measure_wl();

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);
  Serial.begin(115200);
  wifi();
  //get_time(); 
  get_motorset();
  //get_notif();
  delay(1000);  
}

void loop() {
  
  int temp_s  = measure_temp();
  int wl_s = measure_wl();
  if (temp_s < 18) low_temp();
  else heater_off();
  
  if (wl_s < 20) low_wl();
  Serial.println(" ");
  Serial.println(" ");
  put_sens(temp_s, wl_s);
  getjs();
  
  delay(15000);
}


void wifi() {
  WiFi.begin(CUSTOM_SSID, CUSTOM_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay (1000);
  }
  while(1) {
    if (WiFi.status() == WL_CONNECTED) {
        ip = WiFi.localIP();
        Serial.println(ip);
        break;
    }
  }
}

void getjs() {
  HTTPClient http;
  http.begin("https://kokoshkite.pythonanywhere.com/sensors");
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(payload);
  JSONVar myObject = JSON.parse(payload);
  Serial.println(int(myObject["water_level"]));
  http.end();
  delay(1000);
}
void get_notif() {
  HTTPClient http;
  http.begin("https://kokoshkite.pythonanywhere.com/notifications");
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(payload);
  JSONVar myObject = JSON.parse(payload);
  Serial.println(myObject["has_not_water"]);
  Serial.println(myObject["has_not_food"]);
  Serial.println(myObject["is_heater_on"]);
  Serial.println(myObject["is_motor_on"]);
  http.end();
  delay(1000);  
}

void get_motorset() {
  HTTPClient http;
  http.begin("https://kokoshkite.pythonanywhere.com/settings/motor/");
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(payload);
  JSONVar myObject = JSON.parse(payload);
  Serial.println(myObject[0]["time_on"]);
  Serial.println(myObject[2]["time_off"]);
  http.end();
  delay(1000);  

}

void get_time() {
  HTTPClient http;
  http.begin("https://kokoshkite.pythonanywhere.com/settings/timing/");
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(payload);
  JSONVar myObject = JSON.parse(payload);
  Serial.println(myObject["last_filled"]);
  Serial.println(myObject["finishing_time"]);
  http.end();
  delay(1000);
}

void put_sens(int temp_s, int wl_s) {
  HTTPClient http;
  http.begin("https://kokoshkite.pythonanywhere.com/sensors");
  JSONVar myObject;
  myObject["water_level"] = wl_s;
  myObject["temperature"] = temp_s;
  String data_t = JSON.stringify(myObject);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(data_t);
  Serial.println(http.getString());
  http.end();
  delay(1000);
}


int measure_temp() {
    average = analogRead(tempPin);
    average = 4095 / average - 1;
    average = SERIESRESISTOR / average;
    float temperature;
    temperature = average / THERMISTORNOMINAL; 
    temperature = log(temperature);
    temperature /= BCOEFFICIENT;
    temperature += 1.0 / (TEMPERATURENOMINAL + 273.15); 
    temperature = 1.0 / temperature;
    temperature -= 273.15; 
//    Serial.print("Temperature: "); 
 //   Serial.print(temperature);
 //   Serial.println("°C");   
    return (temperature);
}

void low_temp() { 
  digitalWrite(ledPin, HIGH); //turn on the led
  digitalWrite(heaterPin, HIGH);
  Serial.println("The temperature is low!"); //send notification
  Serial.println("The heater is on!");
  
  HTTPClient http;
  http.begin("https://kokoshkite.pythonanywhere.com/notifications");
  JSONVar myObject;
  myObject["is_heater_on"] = "True";
  String data_t = JSON.stringify(myObject);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(data_t);
  Serial.println(http.getString());
  http.end();
  delay(1000);
  
}

int measure_wl() {
  wl_sens = analogRead(wlPin) / 100;
//  Serial.print("Water Level: ");
 // Serial.println(wl_sens);
  return (wl_sens);
}

void low_wl() {
  Serial.println("The water level is low!");
  //digitalWrite(ledPin, HIGH);
}

void motor() {
  digitalWrite(motorPin, HIGH);
  Serial.println("The motor is on!");  
}



void heater_off() {
  digitalWrite(ledPin, LOW);
  digitalWrite(heaterPin, LOW);
  HTTPClient http;
  http.begin("https://kokoshkite.pythonanywhere.com/notifications");
  JSONVar myObject;
  myObject["is_heater_on"] = "False";
  String data_t = JSON.stringify(myObject);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(data_t);
  Serial.println(http.getString());
  http.end();
  delay(1000);
}
