#ifndef _RSA_H_
#define _RSA_H_
// #include <vector>
// #include <string>
#include <Arduino.h>




void genKeysRSA(uint16_t &private_key, uint16_t &public_key, uint16_t &n);
// int encryptRSA(int message, int public_key, int n);
// int decryptRSA(int encrpyted_text, int private_key, int n);
void encoderRSA(uint16_t *cypher, String message, uint16_t public_key, uint16_t n);
String decoderRSA(uint16_t *cypher, uint16_t len, uint16_t private_key, uint16_t n);

#endif