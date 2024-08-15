#include "lora.hpp"

byte localAddress = 0xFF;     // address of this device
byte destination = 0x00;      // destination to send to

// uint8_t Lora_init(LoRaClass *lora, SPIClass &spi) {
//     spi.begin(LORA_SCK, LORA_MISO, LORA_MISO);
//     lora->setSPI(spi);
//     lora->setPins(LORA_CS, LORA_RST, LORA_IRQ);
//     return lora->begin(frequency);
// }

void read2Byte(LoRaClass *lora, uint16_t &val) {
  val = 0;
  uint8_t a = 0;
  if(lora->available()) {
    a = lora->read();
    val |= a;
    val = val << 8;
  }
  
  // Serial.print(a);
  if(lora->available()) {
    a = lora->read();
    val |= a;
  }
  // Serial.print(a);
  // Serial.print(" ");
  // val = val << 8;
  // Serial.printf("%d ", val);
}

void send2Byte(LoRaClass *lora, uint16_t val) {
  uint8_t a = 0;
  
  a = val >> 8;
  lora->write(a);
  // Serial.print(a);
  a = val;
  lora->write(a);
  // Serial.print(a);
  // Serial.print(" ");
  // Serial.printf("%d ", val);
}


void LoRa_rxMode(LoRaClass *lora){
  lora->enableInvertIQ();                // active invert I and Q signals
  lora->receive();                       // set receive mode
}

void LoRa_txMode(LoRaClass *lora){
  lora->idle();                          // set standby mode
  lora->enableInvertIQ();                // active invert I and Q signals
}

void sendMassageLora(LoRaClass *lora, uint8_t cmd, uint16_t* data, 
        uint16_t lenData,  String sender, String des) {
  // lora->idle();                          // set standby mode
  // lora->enableInvertIQ(); 

  lora->beginPacket();                   // start packet
  
  // Serial.println(sender);
  // Serial.println(des);
  lora->write(des.length());
  lora->print(des);              // add destination address
  lora->write(sender.length());
  lora->print(sender);             // add sender address
  lora->write(cmd);
  // lora->write((byte)(lenData & 0xFF));        // add payload length
  // lora->write((byte)((lenData >> 8) & 0xFF));

  send2Byte(lora, lenData);
  // Serial.println(lenData);
  // if(cmd != REQUEST_KEY) {
    for(int i = 0; i < 96; i++) {
      // lora->write(data[i]);
      send2Byte(lora, data[i]);
    }
  // }
  
  //send2Byte(lora, 0);
  lora->endPacket();                     // finish packet and send it
  // lora->enableInvertIQ();                // active invert I and Q signals
  // lora->receive(512); 
}

bool onReceive(LoRaClass *lora, uint8_t &cmd, uint16_t *data, uint16_t &lenData, 
                String MAC, String *list_receive, int len_list_receive, String &MACSend) {
  
  if (lora->parsePacket() == 0) return 0;          // if there's no packet, return
  //memset(data, 0, 256);
  MACSend = "";
  String des = "";
  
  cmd = 0;

  int check = 0;
  // read packet header bytes:
  Serial.println("------------------------HEADER------------------------");
  Serial.println("MAC: " + MAC);
  byte recipientlen = lora->read();          // recipient address
  for(int i = 0; i < recipientlen; i++) {
    des += (char)lora->read();
  }
  Serial.print("Des: "+ des +"\n");
  byte senderlen = lora->read();            // sender address
  for(int i = 0; i < senderlen; i++) {
    MACSend += (char)lora->read();
  }
  Serial.print("Sen: " + MACSend + "\n");
  cmd = lora->read();
  Serial.printf("CMD: %d\n", cmd);
  Serial.println("-----------------------------------------------------");
  read2Byte(lora, lenData);
  if(MAC == des) {
   for(int i = 0; i < len_list_receive; i++) {
      // Serial.println("lis: " + list_receive[i]);
      if(list_receive[i] == MACSend & MAC != MACSend) {
        
            // incoming msg length
        if(1) {
          for(int j = 0; j < SIZE_SEND_LORA; j++) {
          // data[j] = lora->read();
          read2Byte(lora, data[j]);
          // Serial.printf("%d ", data[j]);
          }
        }
        // if(cmd == REQUEST_KEY);
        // if(cmd == SEND_PUBLIC_KEY) {
        //   read2Byte(lora, data[0]);
        //   read2Byte(lora, data[1]);
        // }
        // if(cmd == SEND_CYPHER_KEY_AES) {
        //   for(int i = 0; i < lenData; i++) {
        //     read2Byte(lora, data[i]);
        //   }
        // }
        
        check = 1;
        break;
      }
    } 
  }

  if(check) return pass;
  else return false;
}