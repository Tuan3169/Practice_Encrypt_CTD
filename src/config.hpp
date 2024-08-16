#ifndef CONFIG_H
#define CONFIG_H

#define FLAG_HARD_SENSOR     0
#define FLAG_HARD_SD         1
#define FLAG_HARD_RTC        2 
#define FLAG_HARD_ETHERNET   3
#define FLAG_HARD_WIFI       4
#define FLAG_HARD_LORA       5


#define SIZE_SEND_LORA  96
#define SIZEBUFFSD      100

#define SD_CS           15
#define SD_MOSI         23
#define SD_MISO         19
#define SD_SCK          18

#define ETHERNET_SCK    18
#define ETHERNET_MOSI   23
#define ETHERNET_MISO   19
#define ETHERNET_CS     5

#define TFT_CS          33
#define TFT_DC          25
#define TFT_MOSI        26
#define TFT_SCK         27
#define TFT_RST         32

#define LORA_CS         12
#define LORA_RST        0
#define LORA_IRQ        2
#define LORA_SCK        14
#define LORA_MISO       13
#define LORA_MOSI       4

#define IN
#define OUT
#define PROJECT_LOG int

#define API_KEY "AIzaSyBl1j4qIZFoohAnieYYShZo4nwVJCnnqvQ"
#define DATABASE_URL "https://energy-644ec-default-rtdb.firebaseio.com/"
#define USER_EMAIL "tuandinh12a3@gmail.com"
#define USER_PASSWORD "tuan1234"




#define SEND_OK_SET_KEY     56
#define SEND_CYPHER_KEY_AES 55
#define SEND_PUBLIC_KEY     54
#define REQUEST_KEY         53
#define REBOOT_LORA         52
#define RESET_SENSOR_LORA   51

// chế độ mã hóa
#define MODE_NONE           1
#define MODE_AES            2
#define MODE_AES_RSA        3
#define MODE_SIGNATURE      4
#define MODE_RSA            5      
// chế độ gửi bản tin lora
#define MODE_LORA_SEND_DATA 1   // send data read sensor
#define MODE_LORA_SEND_TEST 2   // send message test
// chế độ hệ thống
#define MODE_NODE           1
#define MODE_GATEWAY        2
#define MODE_NODE_GATEWAY   3
// chế độ lưu trữ sd
#define MODE_STORE_SD       1
#define MODE_NO_STORE_SD    0
// chế độ cập nhật thời gian khi có internet
#define HAS_UPDATE_TIME     1
#define NO_UPDATE_TIME      0
// Key and Message
#define MESSAGE_TEST        "Dinh Van Tuan" // < 80
#define KEY_AES_NODE        "abcdefghikmlt456836"   // 16
#define KEY_AES_GATEWAY     ""

// config system
// #define MAC_GATWAY               "232:107:234:246:222:104"
// #define MAC_NODE                 "216:188:56:250:127:40"
#define MODE_CRYPT               MODE_AES_RSA
#define MODE_SYSTEM_LORA         MODE_NODE
#define MODE_SD                  MODE_STORE_SD
#define UPDATE_TIME              HAS_UPDATE_TIME
#define MODE_LORA_SEND           MODE_LORA_SEND_TEST
#define TIME_READ_SENSOR         15


typedef enum {
    fail,
    pass
} project_log;



const long  gmtOffset_sec = 7*3600;
const int   daylightOffset_sec = 0;

const long frequency = 915E6;  // LoRa Frequency

#endif