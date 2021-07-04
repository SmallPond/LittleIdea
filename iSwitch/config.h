#ifndef _CONFIG_H_
#define _CONFIG_H_
// LED 连接到 D0 GPIO16上，测试到底要填入哪个值
#define LED_PIN          2
#define SELF_TEST_KEY_PIN   0    // GPIO 0共用 Flash KEY
#define UP_SERVO_PIN    4
#define DOWN_SERVO_PIN  14 
//#define DOWN_SERVO_PIN  15
#define Lightness_ADC_PIN A0
#define NB_TRY_WIFI     30   // 15s  = 30 * 500ms
const char *ssid =      "xxx";
const char *password =  "xxx";

const char* mqtt_server = "xxx";    //服务器网址或者IP地址
const char* mqtt_user = "xxx";
const char* mqtt_password = "xxx";
const int mqtt_server_port = 18083;  // 修改端口号
const char* TOPIC_CONFIG =  "store/switch/config";                    
const char* TOPIC_COMMAND = "store/switch/set";
// 灯的状态                    
const char* TOPIC_STATE =   "store/switch/state";
// 开关的在线状态  
const char* TOPIC_SWITCH_STATUS =   "store/switch/status"; 
const char* client_id = "client-001";     // 标识当前设备的客户端编号
const char* device_name = "nodemcu_switch_1";



enum LightStates {
  LI_UNKNOW = 0,
  LI_OPENED,
  LI_CLOSED,
};
#endif
