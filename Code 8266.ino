
//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"	// Install Firebase ESP8266 library
#include <ESP8266WiFi.h>
#include <DHT.h>		// Install DHT11 Library and Adafruit Unified Sensor Library

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <string.h>

#define FIREBASE_HOST "https://internet-of-thing-6c4d7-default-rtdb.firebaseio.com/"                          // the project name address from firebase id
#define FIREBASE_AUTH "oygjlIxQMXj4UltIHRgyJpivYJZBzwD4AaEtJZFE"            // the secret key generated from firebase
#define WIFI_SSID "Nha Tro Vui Ve"
#define WIFI_PASSWORD "295bachmai"

String value;
String value_light;
//float value_tem = 23;
//float value_hum = 68;
//light D0,led-D5
int led1 = 14;
int cambien_light = 16;
//temperature, humidity D1,led-D6
int led2 = 12;
DHT dht(4, DHT11);
//gas D3
int led3 = 0;
//int gas1 = 5;
int gas2 = 2;
float gas_value = 10;
String get_time;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);		//dong bo thoi gian 

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseData ledData;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  connectWifi();
  timeClient.begin();

  pinMode(cambien_light, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  //  pinMode(gas1,INPUT);
  pinMode(gas2, INPUT);
  dht.begin();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
  get_time = gettime();

  dht1();
  light();
  gas();
  delay(1000);
}

//kiểm tra chuỗi có bắt đầu bằng một chuỗi khác
boolean startwith(String a, String b) {
  for (int i = 0; i < b.length(); i++) {
    if (!(a[i] == b[i])) {
      return false;
    }
  }
  return true;
}
//lấy thời gian thực GTM+7
String gettime() {
  timeClient.update();
  timeClient.setTimeOffset(25200);
  return timeClient.getFormattedTime();
}

//cảm biến khí gas
void gas() {
  //    Serial.println(analogRead(gas1));
  gas_value = digitalRead(gas2);				//doc du lieu tu chan D0 cua mq2
  Serial.println(gas_value);
  if (Firebase.getString(firebaseData, "user1/fire/fire1/auto")) {					//lay chuoi ki tu tu firebase 
    if (firebaseData.stringData() == "true") {
      Serial.println("gas auto");
      if (gas_value == 0) {															
        Serial.println("Có gas");
        digitalWrite(led3, HIGH);
        //        if (Firebase.getString(gasData, "user1/fire/fire1/status")) {
        //          if (startwith(gasData.stringData(), "0"))
        Firebase.setString(ledData, "user1/fire/fire1/status", "1+" + get_time);
        //        }
      }
      else {
        Serial.println("Không có gas");
        digitalWrite(led3, LOW);
        //        if (Firebase.getString(gasData, "user1/fire/fire1/status")) {
        //          if (startwith(gasData.stringData(), "1"))
        Firebase.setString(ledData, "user1/fire/fire1/status", "0+" + get_time);
        //        }
      }
    }

    else if (firebaseData.stringData() == "false") {
      digitalWrite(led3, LOW);
      Serial.println("gas không auto");
      if (gas_value == 0) {
        Serial.println("Có gas");
        //        if (Firebase.getString(gasData, "user1/fire/fire1/status")) {
        //          if (startwith(gasData.stringData(), "0"))
        Firebase.setString(ledData, "user1/fire/fire1/status", "1+" + get_time);
        //        }
      }
      else {
        Serial.println("Không có gas");
        //        if (Firebase.getString(gasData, "user1/fire/fire1/status")) {
        //          if (startwith(gasData.stringData(), "1"))
        Firebase.setString(ledData, "user1/fire/fire1/status", "0+" + get_time);
        //        }
      }
    }
  }
}

//cảm biến nhiệt độ,độ ẩm
void dht1() {

  float h = dht.readHumidity();						//doc du lieu tu chan D0 cua dht11
  float t = dht.readTemperature();
  // Check if any reads failed
  if (isnan(h) || isnan(t)) {						//kiem tra xem co ket noi voi dht11 hay khong
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("C  ,"));
  Serial.println();
  //gui du lieu len firebase
  if (Firebase.pushString(firebaseData, "user1/tem_hum/tem_hum1/data/tem", String(t) + "+" + get_time))
    Serial.println("PASSED");
  else
    Serial.println("FAILED");

  if (Firebase.pushString(firebaseData, "user1/tem_hum/tem_hum1/data/hum", String(h) + "+" + get_time))
    Serial.println("PASSED");
  else
    Serial.println("FAILED");

  if (Firebase.getString(firebaseData, "user1/tem_hum/tem_hum1/auto")) {				//kiem tra xem che do auto co duoc bat hay khong
    if (firebaseData.stringData() == "true") {											//neu chuoi ki tu la true thi che do auto duoc bat
      if (t > 33) {
        Firebase.setString(ledData, "user1/tem_hum/tem_hum1/status", "1+" + get_time);		//kiem tra xem khi t > 33do thi den 2 duoc bat
        digitalWrite(led2, HIGH);
      }
      else {
        Firebase.setString(ledData, "user1/tem_hum/tem_hum1/status", "0+" + get_time);	
        digitalWrite(led2, LOW);
      }
      Serial.print("auto");
    }
    else if (firebaseData.stringData() == "false") {									//false thi auto tat
      Serial.println("không auto");
      if (Firebase.getString(ledData, "user1/tem_hum/tem_hum1/status")) {				//kiem tra xem che do hand co duoc bat hay khong 
        if (startwith(ledData.stringData(), "1"))										// chuoi ki tu la 1 thi che do hand duoc bat den sang
          digitalWrite(led2, HIGH);		
        else
          digitalWrite(led2, LOW);
      }
    }
  }
}

//cảm biến ánh sáng
void light() {
  value = String(digitalRead(cambien_light));											//doc du lieu tu cam bien anh sang
  if (Firebase.getString(firebaseData, "user1/light/light1/auto")) {					//chuoi ki tu la truee thi che do auto duoc bat
    if (firebaseData.stringData() == "truee") {
      value_light = "reset_value_light";
      Firebase.setString(firebaseData, "user1/light/light1/auto", "true");
    }
    else if (firebaseData.stringData() == "true") {
      if (value_light != value) {															
        Serial.println("Giá trị sáng đã thay đổi");									
        Firebase.setString(firebaseData, "user1/light/light1/status", value + "+" + get_time);			//che do hand
        Serial.print("Giá trị là :");
        if (Firebase.getString(ledData, "user1/light/light1/status")) {
          if (startwith(ledData.stringData(), "0")) {
            Serial.println("sáng");
            digitalWrite(led1, LOW);
          }
          else {
            Serial.println("tối");
            digitalWrite(led1, HIGH);
          }
        }
      }
    }
    else if (firebaseData.stringData() == "time") {										//che do hen gio
      Serial.println("Hẹn giờ");
      String t = "";
      if (Firebase.getString(ledData, "user1/light/light1/time/everyday")) {
        Serial.println("pass");
        t = ledData.stringData();
      }
      if (Firebase.getString(ledData, "user1/light/light1/status")) {
        Serial.println("Pass");
        if (startwith(ledData.stringData(), "1")) {
          Serial.println("Đang bật đèn");		//lay theo chuan HH/MM/SS
          if (((get_time[0] - 48) * 10 * 3600 + (get_time[1] - 48) * 3600 + (get_time[3] - 48) * 600 + (get_time[4] - 48) * 60 + (get_time[6] - 48) * 10 + (get_time[7] - 48)) >=
              ((t[9] - 48) * 10 * 3600 + (t[10] - 48) * 3600 + (t[12] - 48) * 600 + (t[13] - 48) * 60 + (t[15] - 48) * 10 + (t[16] - 48))) {
            digitalWrite(led1, LOW);
            Firebase.setString(ledData, "user1/light/light1/status", "0+" + get_time);
          }
        }
        else {
          Serial.println("Đang tắt đèn");
          if (((get_time[0] - 48) * 10 * 3600 + (get_time[1] - 48) * 3600 + (get_time[3] - 48) * 600 + (get_time[4] - 48) * 60 + (get_time[6] - 48) * 10 + (get_time[7] - 48)) >=
              ((t[0] - 48) * 10 * 3600 + (t[1] - 48) * 3600 + (t[3] - 48) * 600 + (t[4] - 48) * 60 + (t[6] - 48) * 10 + (t[7] - 48))) {
            digitalWrite(led1, HIGH);
            Firebase.setString(ledData, "user1/light/light1/status", "1+" + get_time);
          }
        }
      }

    }
    else {
      Serial.println("Không tự động");
      Serial.print("Giá trị là :");
      if (Firebase.getString(ledData, "user1/light/light1/status")) {
        if (startwith(ledData.stringData(), "0")) {
          Serial.println("sáng");
          digitalWrite(led1, LOW);
        }
        else {
          Serial.println("tối");
          digitalWrite(led1, HIGH);
        }
      }
      value_light = value;
    }
  }
}

//connect wifi
void connectWifi() {
  Serial.print("Kết nối wifi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Kết nối thành công");
}
