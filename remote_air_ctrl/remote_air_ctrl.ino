
#include "WiFi.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <PubSubClient.h>

# WiFi 
char ssid[] = "QMGY";
char password[] = "qingmugongyu2017";

# MQTT 配置
const char* mqtt_server = "xxx";    //服务器网址或者IP地址
const int mqtt_server_port = 18083;
const char* mqtt_user = "xxx";
const char* mqtt_password = "xxx";
// 空调命名
const char* TOPIC_COMMAND = "home/air/set";
// 空调的状态                    
const char* TOPIC_STATE =   "home/air/state";
const char* client_id = "client-home-air";     // 标识当前设备的客户端编号

//定义红外发射的管脚
const uint16_t kIrLed = 23;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.


//空调开：26度,环保
uint16_t power_on[197] = {6034, 7348,  586, 1678,  586, 1678,  584, 1680,  586, 1678,  586, 1678,  586, 1680,  586, 1678,  586, 1678,  586, 562,  588, 562,  586, 562,  586, 562,  586, 562,  586, 562,  586, 560,  588, 562,  586, 1678,  586, 1678,  586, 1678,  586, 1680,  584, 1680,  586, 1678,  586, 1680,  586, 1678,  586, 562,  586, 562,  586, 564,  584, 562,  588, 562,  584, 562,  588, 562,  586, 562,  586, 1680,  586, 1676,  586, 1678,  588, 1678,  586, 1678,  586, 1678,  586, 1678,  586, 1678,  584, 564,  584, 562,  586, 564,  586, 564,  584, 564,  586, 562,  586, 562,  586, 564,  584, 1678,  586, 564,  586, 564,  584, 1680,  586, 562,  586, 562,  586, 1678,  586, 1680,  586, 564,  584, 1678,  586, 1680,  586, 562,  586, 1678,  586, 1678,  586, 562,  586, 562,  586, 1680,  586, 562,  586, 1678,  586, 564,  584, 1680,  586, 564,  586, 1678,  586, 1678,  586, 564,  584, 1680,  584, 564,  584, 1680,  584, 562,  586, 1680,  586, 562,  586, 564,  584, 564,  586, 1678,  586, 562,  586, 1678,  586, 562,  586, 1678,  586, 564,  586, 1678,  586, 1678,  586, 562,  586, 1678,  586, 564,  584, 1680,  586, 562,  586, 1680,  586, 562,  586, 7370,  586};  // GOODWEATHER 552A36000000
//空调关
uint16_t power_off[197] = {6034, 7350,  586, 1678,  586, 1680,  584, 1680,  586, 1680,  586, 1680,  584, 1680,  586, 1678,  586, 1678,  586, 562,  586, 562,  586, 562,  586, 564,  584, 562,  586, 562,  586, 564,  586, 564,  586, 1678,  586, 1678,  586, 1680,  586, 1678,  586, 1680,  586, 1680,  586, 1678,  586, 1678,  586, 562,  586, 562,  586, 562,  586, 564,  586, 562,  586, 562,  586, 562,  586, 562,  586, 1680,  586, 1680,  586, 1678,  586, 1678,  586, 1680,  586, 1680,  586, 1678,  586, 1680,  586, 562,  584, 564,  586, 562,  586, 562,  586, 562,  586, 562,  586, 564,  584, 562,  586, 1678,  586, 1678,  586, 562,  586, 1678,  586, 564,  586, 562,  586, 1680,  586, 1678,  586, 564,  586, 562,  586, 1678,  562, 586,  586, 1680,  584, 1680,  584, 564,  584, 562,  586, 1678,  586, 1678,  586, 564,  586, 562,  584, 1680,  586, 564,  586, 1678,  586, 1678,  586, 562,  586, 564,  584, 1680,  586, 1680,  584, 564,  584, 1680,  562, 586,  562, 586,  564, 584,  562, 1702,  584, 564,  560, 1704,  584, 564,  568, 1696,  586, 564,  584, 1680,  584, 1680,  584, 566,  582, 1680,  562, 588,  584, 1678,  586, 564,  584, 1680,  584, 566,  560, 7394,  562};


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


void mqtt_callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if(topic == TOPIC_COMMAND){
      if(messageTemp == "ON"){
        mqtt_power_on();
        mqttClient.publish(TOPIC_STATE, "ON");    
      }
      else if(messageTemp == "OFF"){
        mqtt_power_off();
        mqttClient.publish(TOPIC_STATE, "OFF");
      }
  }
}

void setup()
{
  //初始化串口
  Serial.begin(115200);
  SetupWiFi();
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(mqtt_callback);
  
  //红外初始化
  irsend.begin();

}

void SetupWiFi()
{
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int _try = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    _try++;
  }
  Serial.println("连接成功 new");
}

void reconnect( ) {
  if (WiFi.status() != WL_CONNECTED) {

    SetupWiFi();
  }
  while (!mqttClient.connected()) {
    if (mqttClient.connect(client_id, mqtt_user, mqtt_password)) {              
      Serial.println("MQTT connect success.");  
      mqttClient.subscribe(TOPIC_COMMAND);       
    } else {
      delay(1000); 
    }
  }
}

void loop()
{
  if (!mqttClient.connected()) {
      reconnect();
  }
  mqttClient.loop();
  delay(200);
}


//回调函数
void mqtt_power_on()
{
    //发送红外指令
    irsend.sendRaw(power_on, 197, 38);  // Send a raw data capture at 38kHz.
    Serial.print("MQTT SET ON\n");
}
//回调函数
void mqtt_power_off()
{
    irsend.sendRaw(power_off, 197, 38);  // Send a raw data capture at 38kHz.
    Serial.print("MQTT SET OFF\n");
}
