#ifndef _ENCRYPTLORA_
#define _ENCRYPTLORA_

#include <Arduino.h>
#include <config.hpp>
#include <AES.h>
#include <SHA256.h>
#include <encrypt\rsa\rsa.h>



struct _EncryptFunction {
    AES128 aes;
    
};
extern _EncryptFunction EncryptFunction;

struct _Encryption {
    int mode;
    //  RSA
    uint16_t public_key;
    uint16_t private_key;
    uint16_t n;
    //  AES
    int setKey;
    String key;
    String plainText;
    uint16_t cypher[128];
};
extern _Encryption Encryption;
struct _Decryption {
    int mode;
    //  RSA
    uint16_t public_key;
    uint16_t private_key;
    uint16_t n;
    //  AES
    int setKey;
    String key;
    uint16_t cypher[128];
    String decryptedText;
};
extern _Decryption Decryption;

String byte_array_to_string(byte* bytes);
PROJECT_LOG setKeyLoraEncrypt(IN _Encryption *encryption);
PROJECT_LOG setKeyLoraDecrypt(IN _Decryption *decryption);
PROJECT_LOG encryptLora(IN _Encryption *encryption);
PROJECT_LOG decryptLora(IN _Decryption *decryption);

#endif