/*
 *@ Author: ding biao
 *@ Date  : 2020.10.8
 *@ 说明   ：在商场的场景下，每天晚上10点~次日8点（暂时未知准确时间）会断电，以致无网络连接
 *          供电问题采用电池供电，考虑续航问题，尽量使得芯片进入 Deep Sleep 模式
 * sleep 单位 ms
 * deepsleep 单位 us
 *
 *@ PWM 引脚 : GPIO12 15 14 04
*/
#include <WiFiClient.h>
#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "config.h"

#define _DEBUG 0
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// 舵机控制全局变量
Servo downServo;
Servo upServo;
Ticker blinker;

// 默认为未知状态
int lightState = LI_UNKNOW;

// 时间服务
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 28800);


void setup ()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  blinker.attach(1, LEDBlinker);
  
  SetupWiFi();
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(callback);

  timeClient.begin();
  digitalWrite(LED_PIN, 0);
  delay(1000);
  blinker.detach();
}

void ServoAttach()
{
  downServo.attach(DOWN_SERVO_PIN);
  upServo.attach(UP_SERVO_PIN);
}

void ServoDetach()
{
  downServo.detach();
  upServo.detach();
}
/*
 * 位置下的舵机工作，使得开关向上打
 * 取Open/Close  Light不太严谨 因为有些开关向下打是关
*/
void CloseLight()
{
  ServoAttach();
  ResetServo();
  //Serial.println("[-]Close");
  downServo.write(35);
  delay(2000);
  downServo.write(80);
  delay(500);           // 待舵机复位稳定

  lightState = LI_CLOSED;
  ServoDetach();
}
/*
 * 位置在上的舵机工作，使得开关向下打
 * 取Open/Close  Light为函数名不太严谨 因为有些开关向上打是开
*/
void OpenLight()
{
  ServoAttach();
  ResetServo();

  //Serial.println("[+]Open");
  upServo.write(80);
  delay(2000);
  upServo.write(35);
  delay(500);

  lightState = LI_OPENED;
  ServoDetach();
}


void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic == TOPIC_COMMAND){
      //Serial.print("Changing Room lamp to ");
      if(messageTemp == "ON"){
        OpenLight();
        mqttClient.publish(TOPIC_STATE, "ON");    
      }
      else if(messageTemp == "OFF"){
        CloseLight();
        mqttClient.publish(TOPIC_STATE, "OFF");
      }
  }
}

void LEDBlinker()
{
  digitalWrite(LED_PIN, !(digitalRead(LED_PIN)));
}

void ResetServo()
{
  //Serial.println("Reset Servo.");
  downServo.write(80);
  upServo.write(35);
  delay(500);
}

void SetupWiFi()
{
  //Serial.print("connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  int _try = 0;
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(500);
    _try++;
    if ( _try >= NB_TRY_WIFI ) {
        //Serial.println("Impossible to connect WiFi, go to Deep sleep");
        _try = 0;
         ESP.deepSleepInstant(5*60e6);  // n*1min
//        delay(5*60e3);             // 5 分钟后重试连接
    }
  }
  //Serial.println("连接成功 new");
}

void reconnect( ) {
  if (WiFi.status() != WL_CONNECTED) {
    //Serial.println("WiFi is not connected, retry setup");
    SetupWiFi();
  }
  int _try = 0;
  while (!mqttClient.connected()) {
    if (mqttClient.connect(client_id, mqtt_user, mqtt_password)) {              
      //Serial.println("MQTT connect success.");  
      mqttClient.subscribe(TOPIC_COMMAND);       
    } else {
      delay(1000); 
    }
  }
}


void loop ()
{

  int sensorValue = 0;
  if (!mqttClient.connected()) {
      reconnect();
  }
  mqttClient.loop();
  mqttClient.publish(TOPIC_SWITCH_STATUS, "ONLINE");

//  sensorValue = analogRead(Lightness_ADC_PIN);
//  Serial.print("sensor value:");
//  Serial.println(sensorValue);  
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();

  //Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();


  if (currentHour == 21 && currentMinute <= 45) {

    if (currentMinute >= 31 && lightState != LI_CLOSED) {
      CloseLight();
      mqttClient.publish(TOPIC_STATE, "OFF");  
    } else {
      delay(5e3);            // DELAY 1 min 
    } 
  }
  else if (currentHour == 9 &&  currentMinute <= 45) {
    if(currentMinute > 20 && lightState != LI_OPENED) {
      OpenLight();
      mqttClient.publish(TOPIC_STATE, "ON");  
    }
    else {
      delay(5e3);
    }
  } else {

    delay(2000);
    mqttClient.publish(TOPIC_SWITCH_STATUS, "OFFLINE");
    downServo.attach(DOWN_SERVO_PIN);
    downServo.write(80);
    delay(3000);
    ESP.deepSleepInstant(20*60e6);
  }
  delay(1000);
}
