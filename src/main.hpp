#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <Wire.h>
#include <RtcDS1307.h>
#include <EEPROM.h>


#include <Firebase_ESP_Client.h>
#include <PZEM004Tv30.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <LoRa.h>


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include "time.h"
#include <driver/sensor/data.hpp>
#include <driver/sensor/driver_sensor.hpp>
#include <driver/sdcard/driver_sdcard.hpp>
#include <driver/tft/tft1_8.h>
#include <driver/lora/lora.hpp>
#include <config.hpp>
#include <encrypt\encryptLora.h>

struct _Queue {
    QueueHandle_t data_send;
    QueueHandle_t data_serial;
    QueueHandle_t data_display;
    QueueHandle_t data_store;
} queue;

struct _Mutex {
    SemaphoreHandle_t queue_data_send;
    SemaphoreHandle_t queue_data_display;
    SemaphoreHandle_t queue_data_serial;
    // SemaphoreHandle_t queue_data_store;

    SemaphoreHandle_t Serial;
    SemaphoreHandle_t LAN_SD;
    SemaphoreHandle_t RTC;
    SemaphoreHandle_t TFT_LCD;
    SemaphoreHandle_t WIFI;
    SemaphoreHandle_t SENSOR;
    SemaphoreHandle_t LORA_;

    SemaphoreHandle_t firebasem;
    SemaphoreHandle_t flag_hardware;
} mutex;

struct _ConfigSystem {
    _Encryption *encryptConfig;
    _Decryption *decryptConfig;

    String list_mac_receive[10];
    int len_list_mac_receive;
    String MacDes;

    String ssidWifi;
    String passwordWifi;

    int timeDelayReadSensor;
} ConfigSystem;

void init_EEPROM();
void init_Config(_ConfigSystem *configSystem);
void init_wifi();
void init_ethernet();
void init_hardware();
void setupFirebase();
void init_flag();
void init_queue();
void init_mutex();
void TaskInitSensor(void *pvParameter);
void init_RTC();
void init_SD();
void init_LORA();
void networkConnection_eth();
void networkStatusRequestCallback_eth();
void networkConnection_wifi();
void networkStatusRequestCallback_wifi();

void TaskSerialRTC(void *pvParameter);
void TaskUpdateTimeRTC(void *pvParameter);
void TaskUpdateStatusHardware(void *pvPrameter);
void TaskReadSensor(void *pvParameter);
void TaskResetSensor(void *pvParameter);
void TaskSerialData(void *pvParameter);
void TaskSendData(void *pvParameter);
void TaskDisplayHome(void *pvParameter);
void TaskStoreDataSD(void *pvParameter);
void TaskGateWayLora(void *pvParameter);

#endif