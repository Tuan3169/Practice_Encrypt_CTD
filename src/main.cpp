

#include <main.hpp>
#include <encrypt\encryptLora.h>
#include <encrypt\rsa\rsa.h>

TaskHandle_t readData;
TaskHandle_t serialData;
TaskHandle_t sendData;
TaskHandle_t updateHardware;
TaskHandle_t updateRTC;
TaskHandle_t displayHome;
TaskHandle_t storeDataSD;
TaskHandle_t resetSensorT;
TaskHandle_t gatewayLora;

const char* ntpServer = "pool.ntp.org";
SPIClass spi;

byte mac[6];
String macS = "";
// String MacDes = "228:101:184:113:147:96";
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

Sensor sensor;
RtcDS1307<TwoWire> Rtc(Wire);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);
SDCard sd;

EthernetClass ethernet;
WiFiClass wifi;
LoRaClass lora;

EthernetClient ethernet_client;
WiFiClient wifi_client;

FIREBASE_CLASS firebase;
FirebaseData fbdo;
// FirebaseData fbdo1;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
// FirebaseData stream;
String usid;

int c = 0;

EventGroupHandle_t flag_hardware;
StaticEventGroup_t flag_hardwareStatic;




void init_Config(_ConfigSystem *configSystem) {
    configSystem->encryptConfig = &Encryption;
    configSystem->decryptConfig = &Decryption;

    configSystem->encryptConfig->mode = MODE_CRYPT;
    configSystem->encryptConfig->key = KEY_AES_NODE;
    configSystem->encryptConfig->setKey = 0;

    configSystem->decryptConfig->mode = MODE_CRYPT;
    configSystem->decryptConfig->key = KEY_AES_GATEWAY;
    configSystem->decryptConfig->setKey = 0;

    genKeysRSA(configSystem->encryptConfig->private_key, 
                configSystem->encryptConfig->public_key,
                configSystem->encryptConfig->n);
    
    // configSystem->list_mac_receive[0] = MAC_GATWAY;
    // configSystem->list_mac_receive[1] = MAC_NODE;
    configSystem->list_mac_receive[0] = "212:138:252:198:165:156";
    configSystem->list_mac_receive[1] = "232:107:234:246:222:104";
    configSystem->list_mac_receive[2] = "216:188:56:251:182:176";
    configSystem->list_mac_receive[3] = "228:101:184:113:147:96";
    configSystem->len_list_mac_receive = 4;

    configSystem->MacDes = "228:101:184:113:147:96";
    // configSystem->MacDes = MAC_GATWAY;

    configSystem->ssidWifi = "FPT-Telecom-0CE";
    configSystem->passwordWifi = "0373986375";

    configSystem->timeDelayReadSensor = TIME_READ_SENSOR;
}









void setup() {
    Serial.begin(115200);
    Serial.println("--------------------------------CONFIG------------------------------");
    init_tft(tft);
    displayStart(tft);
    setupDisplayIinit(tft);

    init_EEPROM();
    init_Config(&ConfigSystem);
    init_flag();
    init_queue();
    init_mutex();

    init_hardware();
    Serial.println("MAC: " + macS);
    Serial.println("------------------------------------------------------------------");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    

    
    
    xTaskCreatePinnedToCore(TaskDisplayHome, "displayHome", 3072, NULL, 2, &displayHome, 1);
#if MODE_SYSTEM_LORA == MODE_NODE || MODE_SYSTEM == MODE_NODE_GATEWAY || MODE_CRYPT == MODE_AES_RSA
    xTaskCreatePinnedToCore(TaskReadSensor, "readData", 3072, NULL, 2, &readData, 1);
#endif
#if MODE_SYSTEM_LORA == MODE_GATEWAY || MODE_SYSTEM == MODE_NODE_GATEWAY || MODE_CRYPT == MODE_AES_RSA
    xTaskCreatePinnedToCore(TaskGateWayLora, "gateway Lora", 3072, NULL, 2, &gatewayLora, 0);
#endif
#if UPDATE_TIME
    xTaskCreatePinnedToCore(TaskUpdateTimeRTC, "updateRTC", 2048, NULL, 1, &updateHardware, 1);
#endif
    // xTaskCreatePinnedToCore(TaskSerialData, "serial data", 2048, NULL, 2, &serialData, 1);
#if MODE_SD
    xTaskCreatePinnedToCore(TaskStoreDataSD, "storeDataSD", 4096, NULL, 2, &storeDataSD, 1);
#endif
    xTaskCreatePinnedToCore(TaskSendData, "send data", 32468, NULL, 3, &sendData, 1);
    
    
}

void loop() {
    UBaseType_t flag;
    bool beth, bwifi, brtc, bsd, bsensor;
    while (true)
    {
        xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
        flag = xEventGroupGetBits(flag_hardware);
        xSemaphoreGive(mutex.flag_hardware);

        xSemaphoreTake(mutex.LAN_SD, portMAX_DELAY);
        beth = (ethernet.hardwareStatus()==EthernetNoHardware || ethernet.linkStatus()==LinkOFF);
        xSemaphoreGive(mutex.LAN_SD);
        if(beth) {
            if(flag & 1<<FLAG_HARD_ETHERNET) {
                xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
                xEventGroupClearBits(flag_hardware, 1 << FLAG_HARD_ETHERNET);
                xSemaphoreGive(mutex.flag_hardware);
                
                xSemaphoreTake(mutex.firebasem, portMAX_DELAY);
                fbdo.setGenericClient(&wifi_client, networkConnection_wifi, networkStatusRequestCallback_wifi);
                xSemaphoreGive(mutex.firebasem);  
            }
        } else {
            
            if(!(flag & 1<<FLAG_HARD_ETHERNET)) {
                xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
                xEventGroupSetBits(flag_hardware, 1 << FLAG_HARD_ETHERNET);
                xSemaphoreGive(mutex.flag_hardware);

                xSemaphoreTake(mutex.firebasem, portMAX_DELAY);
                fbdo.setGenericClient(&ethernet_client, networkConnection_eth, networkStatusRequestCallback_eth);
                xSemaphoreGive(mutex.firebasem);
            }
        }


        xSemaphoreTake(mutex.WIFI, portMAX_DELAY);
        bwifi = (wifi.status() == WL_CONNECTED);
        if(bwifi == 0) {
            // wifi.begin(ssidWifi, passwordWifi);
            wifi.reconnect();
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
        bwifi = (wifi.status() == WL_CONNECTED);
        xSemaphoreGive(mutex.WIFI);
        
        if (bwifi && (!(flag & (1<<FLAG_HARD_WIFI)))) {
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupSetBits(flag_hardware, 1<<FLAG_HARD_WIFI);
            xSemaphoreGive(mutex.flag_hardware);
        } else if((bwifi == 0) && (flag & (1<<FLAG_HARD_WIFI))) {
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupClearBits(flag_hardware, 1<<FLAG_HARD_WIFI);
            xSemaphoreGive(mutex.flag_hardware);
            
        }
        

        xSemaphoreTake(mutex.SENSOR, portMAX_DELAY);
        bsensor = sensor.check();
        xSemaphoreGive(mutex.SENSOR);
        if(bsensor && (!(flag & (1<<FLAG_HARD_SENSOR)))) {
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupSetBits(flag_hardware, 1<<FLAG_HARD_SENSOR);
            xSemaphoreGive(mutex.flag_hardware);
        } else if(bsensor == 0 && (flag & (1<<FLAG_HARD_SENSOR))){
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupClearBits(flag_hardware, 1<<FLAG_HARD_SENSOR);
            xSemaphoreGive(mutex.flag_hardware);
        }
        
        xSemaphoreTake(mutex.RTC, portMAX_DELAY);
        brtc = (Rtc.LastError() == 0);
        xSemaphoreGive(mutex.RTC);
        if(brtc == 1 && (!(flag & (1<<FLAG_HARD_RTC)))) {
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupSetBits(flag_hardware, 1<<FLAG_HARD_RTC);
            xSemaphoreGive(mutex.flag_hardware);
        } else if(brtc == 0 && (flag & (1<<FLAG_HARD_RTC))) {
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupClearBits(flag_hardware, 1<<FLAG_HARD_RTC);
            xSemaphoreGive(mutex.flag_hardware);
        }
        
        xSemaphoreTake(mutex.LAN_SD,portMAX_DELAY);
        SD.end();
        bsd = SD.begin(SD_CS);
        xSemaphoreGive(mutex.LAN_SD);
        if(bsd == 1 && (!(flag & (1<<FLAG_HARD_SD)))) {
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupSetBits(flag_hardware, 1<<FLAG_HARD_SD);
            xSemaphoreGive(mutex.flag_hardware);
        } else if(bsd == 0 && (flag & (1<<FLAG_HARD_SD))) {
            xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
            xEventGroupClearBits(flag_hardware, 1<<FLAG_HARD_SD);
            xSemaphoreGive(mutex.flag_hardware);
        }
        // xSemaphoreTake(mutex.Serial, portMAX_DELAY);
        // Serial.print("Free heap: ");
        // Serial.println(ESP.getFreeHeap());
        // xSemaphoreGive(mutex.Serial);

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void init_EEPROM() {
    Serial.print("init EEPROM: ");
    EEPROM.begin(512);
    tft.setTextColor(ST7735_GREEN);
    tft.print("init EEPROM: Success\n");
    Serial.print("Success\n");
}

void init_hardware() {
    SPI.begin(18,19,23);
    
    
    init_wifi();
    init_ethernet();
    TaskInitSensor(NULL);
    init_RTC();
    init_SD();
    init_LORA();

    setupFirebase();

    // delay(100);
}

void init_wifi() {
    // tft.print("wifi: ");
    Serial.print("connect wifi: ");
    int i = 0;
    wifi.mode(WIFI_AP_STA);
    do {
        wifi.begin(ConfigSystem.ssidWifi.c_str(), ConfigSystem.passwordWifi.c_str());
        // tft.print(".");
        Serial.print(".");
        delay(1000);
        i++;
    } while (wifi.status()!= WL_CONNECTED && i < 2);
    

    wifi.macAddress(mac);
    macS = String(mac[0]) + ":" + String(mac[1]) + ":" + String(mac[2]) + ":" + String(mac[3]) + ":" + String(mac[4]) + ":" + String(mac[5]);
    if(i<10) {
        
        xEventGroupSetBits(flag_hardware, 1<<FLAG_HARD_WIFI);
        tft.setTextColor(ST7735_GREEN);
        tft.print("wifi: ");
        tft.print(wifi.localIP());
        tft.print("\n");
        Serial.println(wifi.localIP());
    } else {
        xEventGroupSetBits(flag_hardware, 0<<FLAG_HARD_WIFI);
        tft.setTextColor(ST7735_RED);
        tft.print("wifi: ");
        tft.print("fail\n");
    }
}
void init_ethernet() {
    
    ethernet.init(ETHERNET_CS);
    wifi.macAddress(mac);
    ethernet_client.setTimeout(1000);
    Serial.println("Initialize Ethernet with DHCP:");
    if (ethernet.begin(mac, 1000, 5000) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
            tft.setTextColor(ST7735_RED);
            tft.print("ethernet: ");
            tft.print("Ethernet shield was not found.\n");
            xEventGroupSetBits(flag_hardware, 0 << FLAG_HARD_ETHERNET);
        }
        
        if (ethernet.linkStatus() == LinkOFF) {
            Serial.println("Ethernet cable is not connected.");
            tft.setTextColor(ST7735_RED);
            tft.print("ethernet: ");
            tft.print("Ethernet cable is not connected.\n");
            xEventGroupSetBits(flag_hardware, 0 << FLAG_HARD_ETHERNET);
        }
        // try to configure using IP address instead of DHCP:
        ethernet.begin(mac, ip, myDns);
    } else {
        Serial.print("  DHCP assigned IP ");
        Serial.println(ethernet.localIP());
        tft.setTextColor(ST7735_GREEN);
        tft.print("ethernet: ");
        tft.print(ethernet.localIP());
        tft.print("\n");
        xEventGroupSetBits(flag_hardware, 1<<FLAG_HARD_ETHERNET);
    }
}

void init_SD() {
    if(SD.begin(SD_CS)) {
        xEventGroupSetBits(flag_hardware, 1 << FLAG_HARD_SD);
        Serial.printf("init sd: success\n");
        tft.setTextColor(ST7735_GREEN);
        tft.print("init sd: success\n");
    } else {
        xEventGroupClearBits(flag_hardware, 1 << FLAG_HARD_SD);
        Serial.printf("init sd: fail\n");
        tft.setTextColor(ST7735_RED);
        tft.print("init sd: fail\n");
    }
}

void init_LORA() {
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI);
    lora.setSPI(spi);
    lora.setPins(LORA_CS, LORA_RST, LORA_IRQ);
    
    if(lora.begin(frequency)) {
        tft.setTextColor(ST7735_GREEN);
        tft.print("init lora: success\n");
        Serial.print("init lora: success\n");
        xEventGroupSetBits(flag_hardware, 1 << FLAG_HARD_LORA);
    } else {
        tft.setTextColor(ST7735_RED);
        tft.print("init lora: fail\n");
        Serial.print("init lora: fail\n");
        xEventGroupClearBits(flag_hardware, 1 << FLAG_HARD_LORA);
    }
}


void init_flag() {
    tft.setTextColor(ST7735_GREEN);
    tft.print("init flag: ");
    while(!flag_hardware) {
        Serial.print("init flag:");
        flag_hardware = xEventGroupCreateStatic(&flag_hardwareStatic);
        // vTaskDelay(100/portTICK_PERIOD_MS);
    }
    tft.print("success\n");
    Serial.println("success");
}


void init_queue() {
    tft.setTextColor(ST7735_GREEN);
    tft.print("init queue: ");
    queue.data_send = xQueueCreate(10, sizeof(Data));
    queue.data_serial = xQueueCreate(1, sizeof(Data));
    queue.data_display = xQueueCreate(1, sizeof(Data));
    queue.data_store = xQueueCreate(1, sizeof(Data));
    
    while ((!queue.data_send) || (!queue.data_display) || (!queue.data_serial))
    {
        Serial.println("fail");
        queue.data_send = xQueueCreate(10, sizeof(Data));
        queue.data_serial = xQueueCreate(1, sizeof(Data));
        queue.data_display = xQueueCreate(1, sizeof(Data));
        queue.data_store = xQueueCreate(1, sizeof(Data));
    }
    tft.print("success\n");
    Serial.println("init queue: success");
}

void init_mutex() {
    tft.setTextColor(ST7735_GREEN);
    tft.print("init mutex: ");
    while(!mutex.queue_data_send) {
        mutex.queue_data_send = xSemaphoreCreateMutex();
    }
    while(!mutex.queue_data_display) {
        mutex.queue_data_display = xSemaphoreCreateMutex();
    }
    while(!mutex.queue_data_serial) {
        mutex.queue_data_serial = xSemaphoreCreateMutex();
    }

    while(!mutex.Serial) {
        mutex.Serial = xSemaphoreCreateMutex();
    }
    while(!mutex.LAN_SD) {
        mutex.LAN_SD = xSemaphoreCreateMutex();
    }
    while(!mutex.WIFI) {
        mutex.WIFI = xSemaphoreCreateMutex();
    }
    while(!mutex.SENSOR) {
        mutex.SENSOR = xSemaphoreCreateMutex();
    }
    while(!mutex.RTC) {
        mutex.RTC = xSemaphoreCreateMutex();
    }
    while(!mutex.TFT_LCD) {
        mutex.TFT_LCD = xSemaphoreCreateMutex();
    }
    while(!mutex.LORA_) {
        mutex.LORA_ = xSemaphoreCreateMutex();
    }

    while(!mutex.flag_hardware) {
        mutex.flag_hardware = xSemaphoreCreateMutex();
    }
    while(!mutex.firebasem) {
        mutex.firebasem = xSemaphoreCreateMutex();
    }
    
    tft.print("success\n");
    Serial.println("init mutex: success");
}

void networkConnection_eth() {}
void networkStatusRequestCallback_eth() {fbdo.setNetworkStatus(true);}
void networkConnection_wifi() {}
void networkStatusRequestCallback_wifi() {fbdo.setNetworkStatus(true);}


void setupFirebase() {
    
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    
    fbdo.setResponseSize(8192);
    // fbdo1.setResponseSize(2048);
    // stream.setResponseSize(1024);
    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;
    // Firebase.begin(&config, &auth);
    // fbdo1.setBSSLBufferSize(2048, 1024);
    fbdo.setBSSLBufferSize(2048, 1024);
    // stream.setBSSLBufferSize(2048 , 1024);
    
    if(ethernet.linkStatus()==LinkON) {
        fbdo.setGenericClient(&ethernet_client, networkConnection_eth, networkStatusRequestCallback_eth);
    } else {
        fbdo.setGenericClient(&wifi_client, networkConnection_wifi, networkStatusRequestCallback_wifi);
    }
    firebase.setFloatDigits(4);
    firebase.reconnectNetwork(true);
    firebase.reconnectWiFi(true);
}


void init_RTC() {
    
    Serial.print("init RTC: ");
    Rtc.Begin();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Rtc.GetIsRunning();
    if(!Rtc.LastError()) {
        xEventGroupSetBits(flag_hardware, 1 << FLAG_HARD_RTC);
        tft.setTextColor(ST7735_GREEN);
        tft.print("init RTC: ");
        tft.print("success\n");
        Serial.print("success\n");
    } else {
        xEventGroupClearBits(flag_hardware, 1 << FLAG_HARD_RTC);
        tft.setTextColor(ST7735_RED);
        tft.print("init RTC: ");
        tft.print("fail\n");
        Serial.print("fail\n");
    }
}

void TaskInitSensor(void *pvParameter) {
    tft.setTextColor(ST7735_GREEN);
        tft.print("init sensor: ");
        Serial.print("init sensor: ");
        while (sensor.check()==fail) {
            sensor.init();
            // vTaskDelay(1000/portTICK_PERIOD_MS);
        }
        Serial.println("success");
        xEventGroupSetBits(flag_hardware, 1 << FLAG_HARD_SENSOR);
        tft.print("success\n");
}

void TaskUpdateTimeRTC(void *pvParameter) {
    struct tm timeinfo;
    RtcDateTime now;
    char datestring[26];
    EventBits_t flag_rtc, flag_internet;
    uint8_t check = 0;

    vTaskDelay(10000/portTICK_PERIOD_MS);

    while (true)
    {
        flag_rtc = xEventGroupWaitBits(flag_hardware, 1 << FLAG_HARD_RTC, pdFAIL, pdFAIL, portMAX_DELAY);
        flag_internet = xEventGroupWaitBits(flag_hardware, 1 << FLAG_HARD_ETHERNET | 1 << FLAG_HARD_WIFI, pdFAIL, pdFAIL, portMAX_DELAY);

        if((flag_rtc & (1 << FLAG_HARD_RTC)) && 
         ((flag_internet & (1<<FLAG_HARD_ETHERNET)) || flag_internet & (1<<FLAG_HARD_WIFI))) {

            xSemaphoreTake(mutex.LAN_SD, portMAX_DELAY);
            xSemaphoreTake(mutex.WIFI, portMAX_DELAY);
            getLocalTime(&timeinfo);
            xSemaphoreGive(mutex.WIFI);
            xSemaphoreGive(mutex.LAN_SD);
            
            if(timeinfo.tm_year+1900 > 2023) {
                xSemaphoreTake(mutex.RTC, portMAX_DELAY);
                Rtc.SetDateTime(RtcDateTime(timeinfo.tm_year+1900,timeinfo.tm_mon+1,
                    timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec));
                now = Rtc.GetDateTime();
                xSemaphoreGive(mutex.RTC);

                check = 1;

                snprintf_P(datestring, 
                countof(datestring),
                PSTR("%02u-%02u-%04u %02u:%02u:%02u\n"),
                now.Month(),
                now.Day(),
                now.Year(),
                now.Hour(),
                now.Minute(),
                now.Second() );

                xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                Serial.print(datestring);
                xSemaphoreGive(mutex.Serial);
            }
            

            
            if(check) {
                vTaskDelete(NULL);
            } else {
                vTaskDelay(60000/portTICK_PERIOD_MS);
            }
        } 
    }
}



void TaskSerialRTC(void *pvParameter) {
    RtcDateTime now;
    char datestring[26];
    EventBits_t flag_rtc;

    vTaskDelay(100/portTICK_PERIOD_MS);

    while(true) {
        xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
        flag_rtc = xEventGroupGetBits(flag_hardware);
        xSemaphoreGive(mutex.flag_hardware);

        if(flag_rtc & (1 << FLAG_HARD_RTC)) {
            xSemaphoreTake(mutex.RTC, portMAX_DELAY);
            now = Rtc.GetDateTime();
            xSemaphoreGive(mutex.RTC);

            snprintf_P(datestring, 
                countof(datestring),
                PSTR("%02u-%02u-%04u %02u:%02u:%02u\n"),
                now.Month(),
                now.Day(),
                now.Year(),
                now.Hour(),
                now.Minute(),
                now.Second() );

            xSemaphoreTake(mutex.Serial, portMAX_DELAY);
            Serial.print(datestring);
            xSemaphoreGive(mutex.Serial);

            vTaskDelete(NULL);
        } else {
            xSemaphoreTake(mutex.Serial, portMAX_DELAY);
            Serial.print("RTC error\n");
            xSemaphoreGive(mutex.Serial);
            vTaskDelete(NULL);
        }
    }
}



void TaskReadSensor(void *pvParameter) {
    Data data[10];
    UBaseType_t numsdata;
    RtcDateTime timeRead;
    char datestring[26];
    int i = 0;
    int kq;

    vTaskDelay(100/portTICK_PERIOD_MS);
    // TickType_t lastWake = xTaskGetTickCount();
    while (true)
    {
        // while(sensor.readData(&data[i]) == fail) {
        // // Serial.println("read data fail");
        //     vTaskDelay(100/portTICK_PERIOD_MS);
        // }

        xSemaphoreTake(mutex.SENSOR, portMAX_DELAY);
        sensor.readData(&data[i]);
        xSemaphoreGive(mutex.SENSOR);

        xSemaphoreTake(mutex.RTC, portMAX_DELAY);
        timeRead = Rtc.GetDateTime();
        xSemaphoreGive(mutex.RTC);

        snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u-%02u-%04u %02u:%02u:%02u"),
            timeRead.Day(),
            timeRead.Month(),
            timeRead.Year(),
            timeRead.Hour(),
            timeRead.Minute(),
            timeRead.Second() );
        
        data[i].setTime(String(datestring));
        data[i].setMac(macS);
        
        kq = pdFAIL;
        while(kq == pdFAIL) {
            xSemaphoreTake(mutex.queue_data_send, portMAX_DELAY);
            kq = xQueueSend(queue.data_send, (void*) &data[i], 0);
            xSemaphoreGive(mutex.queue_data_send);
        }
        xSemaphoreTake(mutex.queue_data_display,portMAX_DELAY);
        xQueueOverwrite(queue.data_display, (void*) &data[i]);
        xSemaphoreGive(mutex.queue_data_display);

        xSemaphoreTake(mutex.queue_data_serial, portMAX_DELAY);
        xQueueOverwrite(queue.data_serial, (void*)&data[i]);
        xSemaphoreGive(mutex.queue_data_serial);

        xQueueOverwrite(queue.data_store, (void*)&data[i]);
        i = (i+1)%10;

        // xTaskCreatePinnedToCore(TaskStoreDataSD, "storeDataSD", 4096, NULL, 2, &storeDataSD, 1);
        vTaskResume(storeDataSD);
        vTaskResume(sendData);
        // xTaskCreatePinnedToCore(TaskSendData, "send data", 16384, NULL, 2, &sendData, 1);
        // xTaskCreatePinnedToCore(TaskSerialData, "serial data", 2048, NULL, 3, &serialData, 1);
        
        // if(sendData != NULL) {
        //     vTaskResume(sendData);
        // } else {
        //     xTaskCreatePinnedToCore(TaskSendDataToFirebase, "send data", 16384, NULL, 2, &sendData, 1);
        // }
        
        vTaskDelay(ConfigSystem.timeDelayReadSensor*1000/portTICK_PERIOD_MS);
    }
}

void TaskResetSensor(void *pvParameter) {
    vTaskDelay(100/portTICK_PERIOD_MS);
    
    while (true)
    {
        while(sensor.resetEnergy()==fail) {
            xSemaphoreTake(mutex.Serial, portMAX_DELAY);
            Serial.println("reset energy fail");
            xSemaphoreGive(mutex.Serial);
            vTaskDelay(10000/portTICK_PERIOD_MS);
        }
        xSemaphoreTake(mutex.Serial, portMAX_DELAY);
        Serial.println("reset energy success");
        xSemaphoreGive(mutex.Serial);
        vTaskDelete(NULL);
    }
}

void TaskSerialData(void *pvParameter) {
    
    int numsData = 0;
    Data da;
    UBaseType_t kq;

    vTaskDelay(100/portTICK_PERIOD_MS);
    while (true)
    {

        xSemaphoreTake(mutex.queue_data_serial,portMAX_DELAY);
        kq = xQueueReceive(queue.data_serial, (void*) &da, 0);
        xSemaphoreGive(mutex.queue_data_serial);

        if(kq) {
            xSemaphoreTake(mutex.Serial, portMAX_DELAY);
            Serial.printf("get data serial: %d\n", numsData);
            Serial.printf("-----------------------data read------------------------------\n");
            // Serial.printf("get data serial succes: %d\n", numsData);
            Serial.println("Mac: " + da.getMac());
            Serial.print("Voltage: ");      Serial.print(da.getVolt());      Serial.println("V");
            Serial.print("Current: ");      Serial.print(da.getCurent());      Serial.println("A");
            Serial.print("Power: ");        Serial.print(da.getPower());        Serial.println("W");
            Serial.print("Energy: ");       Serial.print(da.getEnery(),3);     Serial.println("kWh");
            Serial.print("Frequency: ");    Serial.print(da.getFrequency(), 1); Serial.println("Hz");
            Serial.print("PF: ");           Serial.println(da.getPF());
            Serial.println("Time: " + da.getTime());
            Serial.printf("-----------------------------------------------------\n");
                
            xSemaphoreGive(mutex.Serial);
        }
        // vTaskDelay(100/portTICK_PERIOD_MS);
        vTaskDelete(NULL);
        // vTaskSuspend(NULL);
       
    }
}

void TaskSendData(void *pv) {
    Data dataSend;
    EventBits_t flag_connect;
    FirebaseJson json;
    UBaseType_t numsData;
    uint8_t check;
    String dataS;
    char *dataP;
    int resetSensor;
    int i;
    int checkCypher = 0;
    

    vTaskDelay(pdMS_TO_TICKS(100));
    vTaskSuspend(NULL);
    while (true)
    {
        xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
        flag_connect = xEventGroupGetBits(flag_hardware);
        xSemaphoreGive(mutex.flag_hardware);
        
        // Serial.printf("connected \n");

        do {
            resetSensor = 0;

            xSemaphoreTake(mutex.queue_data_send, portMAX_DELAY);
            numsData = uxQueueMessagesWaiting(queue.data_send);
            // Serial.printf("numsData: %d\n", numsData);
            if(numsData > 0) {
                xQueueReceive(queue.data_send, (void*)&dataSend, 0);
            }
            xSemaphoreGive(mutex.queue_data_send);
            // if(WiFi.status()==WL_CONNECTED) {
            //     firebase.reconnectWiFi(true);
            // }

            if(numsData > 0) {
                json.add("Volt", dataSend.getVolt());
                json.add("Current", dataSend.getCurent());
                json.add("Power", dataSend.getPower());
                json.add("Energy", dataSend.getEnery());
                json.add("Frequency", dataSend.getFrequency());
                json.add("PF", dataSend.getPF());
                json.add("Time", dataSend.getTime());
                json.add("MAC", dataSend.getMac());
                
                check = 0;
                
                if((flag_connect & (1<<FLAG_HARD_ETHERNET) ) && check == 0) {
                    xSemaphoreTake(mutex.LAN_SD, portMAX_DELAY);
                    xSemaphoreTake(mutex.firebasem,portMAX_DELAY);
                    if(!firebase.ready()) {
                        firebase.begin(&config, &auth);
                    }
                    usid = auth.token.uid.c_str();
                    json.add("UserID", usid);
                    json.add("Method", "ethernet");
                    firebase.RTDB.getInt(&fbdo, F(usid + "/" + dataSend.getMac() + "/resetSensor"), &resetSensor);
                    
                    
                    if(resetSensor) {
                        if(dataSend.getMac() == macS) {
                            xTaskCreatePinnedToCore(TaskResetSensor, "reset sensor", 1024, NULL, 3, &resetSensorT, 1);
                        } else {

                        }
                        
                        firebase.RTDB.setInt(&fbdo, F(usid + "/" + dataSend.getMac() + "/resetSensor"), 0);
                    }
                    
                    xSemaphoreGive(mutex.firebasem);
                    xSemaphoreGive(mutex.LAN_SD);
                    // fbdo.setEthernetClient(&ethernet_client, mac, ETHERNET_CS, -1);
                    // xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                    
                    // Serial.print("User UID: ");
                    // Serial.println(usid);
                    // xSemaphoreGive(mutex.Serial);

                    for(int i = 0; i < 10; i++) {
                        if(check == 0) {
                            xSemaphoreTake(mutex.LAN_SD, portMAX_DELAY);
                            xSemaphoreTake(mutex.firebasem,portMAX_DELAY);
                            check = firebase.RTDB.setJSON(&fbdo, usid + "/" + dataSend.getMac() + "/data/"+dataSend.getTime(),&json);
                            firebase.RTDB.setJSON(&fbdo, usid + "/" + dataSend.getMac() + "/datastream", &json);
                            xSemaphoreGive(mutex.firebasem);
                            xSemaphoreGive(mutex.LAN_SD);
                            if(check) {
                                // vTaskDelay(100/portTICK_PERIOD_MS);
                                xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                                Serial.println("send ethernet success");
                                xSemaphoreGive(mutex.Serial);

                                check = 1;
                                break;
                                // xSemaphoreGive(mutex.mutexInternet);
                            }
                        };
                    }
                
                    if(check == 0) {
                        xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                        Serial.println("send ethernet fail");
                        xSemaphoreGive(mutex.Serial);
                    }
                }

                if((flag_connect & (1<<FLAG_HARD_WIFI)) && check == 0){
                    xSemaphoreTake(mutex.WIFI, portMAX_DELAY);
                    xSemaphoreTake(mutex.firebasem, portMAX_DELAY);
                    if(!firebase.ready()) {
                        firebase.begin(&config, &auth);
                    }

                    usid = auth.token.uid.c_str();
                    json.add("UserID", usid);
                    json.add("Method", "wifi");
                    firebase.RTDB.getInt(&fbdo, F(usid + "/" + dataSend.getMac() + "/resetSensor"), &resetSensor);
                    if(resetSensor) {
                        if(dataSend.getMac() == macS) {
                            xTaskCreatePinnedToCore(TaskResetSensor, "reset sensor", 1024, NULL, 3, &resetSensorT, 1);
                        } else {
                            
                        }
                        firebase.RTDB.setInt(&fbdo, F(usid + "/" + dataSend.getMac() + "/resetSensor"), 0);
                    }
                    xSemaphoreGive(mutex.firebasem);
                    xSemaphoreGive(mutex.WIFI);
                    // xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                    // Serial.print("User UID: ");
                    // Serial.println(usid);
                    // xSemaphoreGive(mutex.Serial);

                    
                    for(int i = 0; i < 10; i++) {
                        if(check == 0) {
                            xSemaphoreTake(mutex.WIFI, portMAX_DELAY);
                            xSemaphoreTake(mutex.firebasem, portMAX_DELAY);
                            check = firebase.RTDB.setJSON(&fbdo, usid + "/" + dataSend.getMac() + "/data/"+dataSend.getTime(),&json);
                            firebase.RTDB.setJSON(&fbdo, usid + "/" + dataSend.getMac() + "/datastream", &json);
                            check = 1;
                            xSemaphoreGive(mutex.firebasem);
                            xSemaphoreGive(mutex.WIFI);

                            if(check) {
                            
                                xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                                Serial.println("send wifi success");
                                xSemaphoreGive(mutex.Serial);
                                check = 1;
                                break;
                            };
                        }
                    }
                    

                    if(check == 0) {
                        xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                        Serial.println("send wifi fail");
                        xSemaphoreGive(mutex.Serial);
                    }
                    
                }
#if MODE_SYSTEM_LORA == MODE_NODE
                if((flag_connect & (1<<FLAG_HARD_LORA)) && check == 0){
                    // json.add("UserID", usid);
                    // json.add("Method", "lora");
                    // json.toString(dataS);
#if MODE_LORA_SEND == MODE_LORA_SEND_DATA
                    dataS = "Miao|" + String(dataSend.getVolt()) + "|" +
                        String(dataSend.getCurent()) + "|" +
                        String(dataSend.getPower()) + "|" +
                        String(dataSend.getEnery()) + "|" +
                        String(dataSend.getFrequency()) + "|" +
                        String(dataSend.getPF()) + "|" +
                        String(dataSend.getTime());
                    Encryption.plainText = dataS;
#endif
#if MODE_LORA_SEND == MODE_LORA_SEND_TEST
                    Encryption.plainText = MESSAGE_TEST;
#endif
                    checkCypher = 0;
                    if(Encryption.mode == MODE_NODE) {
                        setKeyLoraEncrypt(&Encryption);
                        encryptLora(&Encryption);
                        checkCypher = 1;
                    }
                    if(Encryption.mode == MODE_AES) {
                        if(Encryption.key == "") {
                            xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                            Serial.println("----------------------------------Encrypt key AES--------------------------");
                            Serial.println("Not key");
                            Serial.println("---------------------------------------------------------------------------");
                            xSemaphoreGive(mutex.Serial);
                        } else {
                            setKeyLoraEncrypt(&Encryption);
                            encryptLora(&Encryption);
                            checkCypher = 1;
                        }
                    }
                    if(Encryption.mode == MODE_AES_RSA) {
                        if(Encryption.setKey == 0 && Encryption.key != "") {
                            sendMassageLora(&lora, REQUEST_KEY, Encryption.cypher, 0, macS,
                                    ConfigSystem.MacDes);
                            xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                            Serial.println("----------------------------------Encrypt key AES--------------------------");
                            Serial.println("Send Request key");
                            Serial.println("MAC sender: " + macS);
                            Serial.println("MAC des: " + ConfigSystem.MacDes);
                            Serial.println("---------------------------------------------------------------------------");
                            xSemaphoreGive(mutex.Serial);
                        } else {
                            setKeyLoraEncrypt(&Encryption);
                            encryptLora(&Encryption);
                            checkCypher = 1;
                        }
                    }
                    
                    if(checkCypher) {
                        xSemaphoreTake(mutex.LORA_, portMAX_DELAY);
                        sendMassageLora(&lora, Encryption.mode, Encryption.cypher,
                            Encryption.plainText.length(), macS, ConfigSystem.MacDes);
                        xSemaphoreGive(mutex.LORA_);
                        xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                        Serial.println("-------------------------Send-------------------------");
                        Serial.print("DataLen: ");
                        Serial.println(Encryption.plainText.length());
                        Serial.println("MAC: " + macS);
                        Serial.println("send lora message: " + Encryption.plainText);
                        Serial.print("Cypher: ");
                        for(int i = 0; i < SIZE_SEND_LORA; i++) {
                            Serial.printf("%d ", Encryption.cypher[i]);
                        }
                        Serial.print("\n");
                        Serial.println("-------------------------Send-------------------------");
                        xSemaphoreGive(mutex.Serial);
                    }
                }
#endif
                vTaskDelay(1000);
                json.clear();
            }
        } while (numsData>0);

        // vTaskDelete(NULL);
        
        vTaskSuspend(NULL);
    }

}

void TaskStoreDataSD(void *pvParameter) {
    Data data_store;
    EventBits_t flag;
    FirebaseJson json;
    String data;
    File file;

    vTaskDelay(100/portTICK_PERIOD_MS);
    vTaskSuspend(NULL);

    while (true)
    {
        xSemaphoreTake(mutex.flag_hardware,portMAX_DELAY);
        flag = xEventGroupGetBits(flag_hardware);
        xSemaphoreGive(mutex.flag_hardware);

        

        if(flag & (1<<FLAG_HARD_SD)) {

            xQueueReceive(queue.data_store, (void*)&data_store, 0);
            json.setFloatDigits(2);
            json.add("Volt", data_store.getPower());
            json.add("Current", data_store.getCurent());
            json.add("Power", data_store.getPower());
            json.add("Energy", data_store.getEnery());
            json.add("Frequency", data_store.getFrequency());
            json.add("PF", data_store.getPF());
            json.add("Time", data_store.getTime());
            json.add("MAC", data_store.getMac());
           
            json.toString(data);
            // xSemaphoreTake(mutex.Serial, portMAX_DELAY);
            // Serial.println(data);
            // xSemaphoreGive(mutex.Serial);
            xSemaphoreTake(mutex.LAN_SD, portMAX_DELAY);
            file = SD.open("/data/data.txt", FILE_APPEND);
            if(!file) {
                if(SD.mkdir("/data")) {
                    file = SD.open("/data/data.txt", FILE_WRITE);
                }
            }

            if(file) {
                file.println(data);
            }
            file.close();
            xSemaphoreGive(mutex.LAN_SD);
            json.clear();
        }
        

        // vTaskDelete(NULL);
        vTaskSuspend(NULL);
    }
    
}

void TaskDisplayHome(void *pvParameter) {
    EventBits_t flag_hard;
    Data data_display;
    bool wifi = 0, eth = 0, _sd = 0;
    RtcDateTime now;
    // int numsData = 0;
    bool getData = false;
    char hstring[10];
    char dstring[15];
    vTaskDelay(100/portTICK_PERIOD_MS);

    xSemaphoreTake(mutex.TFT_LCD, portMAX_DELAY);
    tft.fillScreen(BACKGROUND_HOME);
    displayIcon(tft,wifi,eth, _sd);
    xSemaphoreGive(mutex.TFT_LCD);
    
    while (true)
    {
        xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
        flag_hard = xEventGroupGetBits(flag_hardware);
        xSemaphoreGive(mutex.flag_hardware);

        if((flag_hard & (1<<FLAG_HARD_WIFI)) != wifi || (flag_hard &(1<<FLAG_HARD_ETHERNET)) != eth ||(flag_hard&(1<<FLAG_HARD_SD)) != _sd) {
            wifi = flag_hard & (1<<FLAG_HARD_WIFI);
            eth = flag_hard &(1<<FLAG_HARD_ETHERNET);
            _sd = flag_hard&(1<<FLAG_HARD_SD);
            xSemaphoreTake(mutex.TFT_LCD, portMAX_DELAY);
            displayIcon(tft,wifi,eth,_sd);
            xSemaphoreGive(mutex.TFT_LCD);
        }

        xSemaphoreTake(mutex.RTC, portMAX_DELAY);
        now = Rtc.GetDateTime();
        xSemaphoreGive(mutex.RTC);

        snprintf_P(hstring, 
            countof(hstring),
            PSTR("%02u:%02u"),
            now.Hour(),
            now.Minute());
        snprintf_P(dstring, 
            countof(dstring),
            PSTR("%02u/%02u/%04u"),
            now.Month(),
            now.Day(),
            now.Year());
        
        xSemaphoreTake(mutex.TFT_LCD, portMAX_DELAY);
        displayTime(tft, (String)hstring, (String)dstring);
        xSemaphoreGive(mutex.TFT_LCD);

        getData = false;
        xSemaphoreTake(mutex.queue_data_display, portMAX_DELAY);
        if(xQueueReceive(queue.data_display, (void*)&data_display, 0)) {
            getData = true;
        }
        xSemaphoreGive(mutex.queue_data_display);
        if(getData) {
            xSemaphoreTake(mutex.TFT_LCD, portMAX_DELAY);
            displayData(tft, data_display.getVolt(), data_display.getCurent(), data_display.getPower(), data_display.getEnery());
            xSemaphoreGive(mutex.TFT_LCD);
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    
}

void TaskGateWayLora(void *pvParameter) {
    String data;
    String MACSender;
    uint8_t cmd;
    uint16_t lenData;
    Data dataSensor;
    EventBits_t flag;
    FirebaseJson json;
    FirebaseJsonData jsonData;
    int check;
    uint16_t dataReceiver[128];

    vTaskDelay(100/portTICK_PERIOD_MS);
    xSemaphoreTake(mutex.flag_hardware, portMAX_DELAY);
    flag = xEventGroupGetBits(flag_hardware);
    xSemaphoreGive(mutex.flag_hardware);

    while (true)
    {
        if(flag & 1 << FLAG_HARD_LORA) {
            check  = 0;
            xSemaphoreTake(mutex.LORA_, portMAX_DELAY);
            check = onReceive(&lora,cmd, dataReceiver, lenData, macS, ConfigSystem.list_mac_receive, ConfigSystem.len_list_mac_receive, MACSender);
            xSemaphoreGive(mutex.LORA_);
            if(check) {

                if(cmd < 50) {
#if MODE_SYSTEM_LORA == MODE_GATEWAY
                    memcpy(Decryption.cypher, dataReceiver, 128);
                    xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                    Serial.printf("-----------------------data lora receive --------------------------\n");
                    Serial.print("lenCypher: ");
                    Serial.println(lenData);
                    Serial.print("Cypher: ");
                    for(int i = 0; i < SIZE_SEND_LORA; i++) {
                        Serial.printf("%d ",Decryption.cypher[i]);
                    }
                    Serial.println();
                    setKeyLoraDecrypt(&Decryption);
                    decryptLora(&Decryption);
                    data = Decryption.decryptedText;
                    data = data.substring(0,lenData);
                    Serial.println("Data: " + data);
                    Serial.printf("------------------------------------------------------------------\n");

                    xSemaphoreGive(mutex.Serial);
#endif
                } else {
#if MODE_SYSTEM_LORA == MODE_GATEWAY
                    if(cmd == REQUEST_KEY) {
                        dataReceiver[0] = Encryption.public_key;
                        dataReceiver[1] = Encryption.n;
                        vTaskDelay(1/portTICK_PERIOD_MS);
                        xSemaphoreTake(mutex.LORA_, portMAX_DELAY);
                        sendMassageLora(&lora, SEND_PUBLIC_KEY, dataReceiver, 2, macS, MACSender);
                        xSemaphoreGive(mutex.LORA_);
                        xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                        Serial.printf("-----------------------Receive request key--------------------------\n");
                        Serial.printf("Send public key: %d\n", dataReceiver[0]);
                        Serial.printf("Send n: %d\n", dataReceiver[1]);
                        Serial.printf("-------------------------------------------------------------------\n");
                        xSemaphoreGive(mutex.Serial);
                    }
#endif
#if MODE_SYSTEM_LORA == MODE_NODE
                    if(cmd == SEND_PUBLIC_KEY) {
                        // Decryption.public_key = dataReceiver[0];
                        // Decryption.n = dataReceiver[1];
                        Serial.printf("Nhan public key\n");
                        uint16_t cypherKeyAES[128];
                        encoderRSA(cypherKeyAES, Encryption.key, dataReceiver[0], dataReceiver[1]);
                        vTaskDelay(1/portTICK_PERIOD_MS);
                        xSemaphoreTake(mutex.LORA_, portMAX_DELAY);
                        sendMassageLora(&lora, SEND_CYPHER_KEY_AES, cypherKeyAES,
                                Encryption.key.length(), macS, MACSender);
                        xSemaphoreGive(mutex.LORA_);
                        xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                        Serial.printf("-----------------------Send cypher key--------------------------\n");
                        Serial.printf("Key: ", Encryption.key);
                        Serial.printf("public key: %d\n", dataReceiver[0]);
                        Serial.printf("n: %d\n", dataReceiver[1]);
                        Serial.printf("Key AES Cypher: ");
                        for(int i = 0;  i < 16; i++) printf("%d ", cypherKeyAES[i]);
                        Serial.printf("\n-------------------------------------------------------------------\n");
                        xSemaphoreGive(mutex.Serial);
                    }
#endif
#if MODE_SYSTEM_LORA == MODE_GATEWAY
                    if(cmd == SEND_CYPHER_KEY_AES) {
                        Decryption.key = decoderRSA(dataReceiver, lenData,
                                     Encryption.private_key, Encryption.n);
                        vTaskDelay(1/portTICK_PERIOD_MS);
                        xSemaphoreTake(mutex.LORA_, portMAX_DELAY);
                        sendMassageLora(&lora, SEND_OK_SET_KEY, dataReceiver,
                                0, macS, MACSender);
                        xSemaphoreGive(mutex.LORA_);
                        Decryption.setKey = 1;
                        xSemaphoreTake(mutex.Serial, portMAX_DELAY);
                        Serial.printf("-----------------------Set key AES--------------------------\n");
                        Serial.printf("Key AES Cypher: ");
                        for(int i = 0;  i < 16; i++) printf("%d ", dataReceiver[i]);
                        Serial.println("\nKey AES: " + Decryption.key);
                        Serial.printf("\n-------------------------------------------------------------------\n");
                        xSemaphoreGive(mutex.Serial);
                    }
#endif
#if MODE_SYSTEM_LORA == MODE_NODE
                    if(cmd == SEND_OK_SET_KEY) {
                        Encryption.setKey = 1;
                    }
#endif
                }
                
#if MODE_LORA_SEND == MODE_LORA_SEND_DATA
                // if(cmd > 0) {
                    

                //     json.setJsonData(data);
                //     json.get(jsonData, "Volt");
                //     dataSensor.setVolt(jsonData.floatValue);
                //     json.get(jsonData, "Current");
                //     dataSensor.setCurent(jsonData.floatValue);
                //     json.get(jsonData, "Power");
                //     dataSensor.setPower(jsonData.floatValue);
                //     json.get(jsonData, "Energy");
                //     dataSensor.setEnery(jsonData.floatValue);
                //     json.get(jsonData, "Frequency");
                //     dataSensor.setFrequency(jsonData.floatValue);
                //     json.get(jsonData, "PF");
                //     dataSensor.setPF(jsonData.floatValue);
                //     json.get(jsonData, "Time");
                //     dataSensor.setTime(jsonData.stringValue);
                //     json.get(jsonData, "MAC");
                //     dataSensor.setMac(jsonData.stringValue);

                //     xSemaphoreTake(mutex.queue_data_send, portMAX_DELAY);
                //     xQueueSend(queue.data_send, (void*)&dataSensor, 0);
                //     xSemaphoreGive(mutex.queue_data_send);
                //     // xTaskCreatePinnedToCore(TaskSendData, "send data", 16384, NULL, 2, &sendData, 1);
                //     vTaskResume(sendData);
                // }
                
#endif
            }
            
        }
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}