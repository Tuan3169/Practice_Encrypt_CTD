#ifndef LORA
#define LORA

#include<config.hpp>
#include<SPI.h>
#include <LoRa.h>
#include <encrypt\encryptLora.h>


// uint8_t Lora_init(LoRaClass *lora, SPIClass &spi);
void LoRa_rxMode(LoRaClass *lora);
void LoRa_txMode(LoRaClass *lora);
void sendMassageLora(
    LoRaClass *lora, uint8_t cmd, uint16_t* data,
     uint16_t lenData, String sender, String des);
bool onReceive(
    LoRaClass *lora, uint8_t &cmd, uint16_t *data, 
    uint16_t &lenData, String MAC, String *list_receive, 
    int len_list_receive, String &MACSend);

#endif