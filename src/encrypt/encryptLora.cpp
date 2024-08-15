#include <encrypt\encryptLora.h>


_EncryptFunction EncryptFunction;
_Encryption Encryption;
_Decryption Decryption;

String byte_array_to_string(uint16_t* bytes, int len) {
    String result;
    for (size_t i = 0; i < len; ++i) {
        result += (char)bytes[i];
    }
    return result;
}

PROJECT_LOG setKeyLoraEncrypt(IN _Encryption *encryption) {
    if(encryption->mode == MODE_NONE) {
        encryption->key = "";
        return pass;
    }
    if(encryption->mode == MODE_AES) {
        EncryptFunction.aes.setKey((uint8_t*) encryption->key.c_str(), encryption->key.length());
        return pass;
    }
    return fail;
}

PROJECT_LOG setKeyLoraDecrypt(IN _Decryption *decryption) {
    if(decryption->mode == MODE_NONE) {
        decryption->key = "";
        return pass;
    }
    if(decryption->mode == MODE_AES) {
        EncryptFunction.aes.setKey((uint8_t*)decryption->key.c_str(), decryption->key.length());
        return pass;
    }
    return fail;
}

PROJECT_LOG encryptLora(IN _Encryption *encryption) {
    if(encryption->mode == MODE_NONE) {
        // int i = 0;
        // while (encryption->plainText[i]!=0)
        // {
        //     encryption->cypher[i] = encryption->plainText[i];
        //     i++;
        // }
        // encryption->cypher[i]=0;
        memset(encryption->cypher, 0, 96);
        for(int i = 0; i < encryption->plainText.length(); i++) {
            encryption->cypher[i] = encryption->plainText[i];
        }
        return pass;
    }
    if(encryption->mode == MODE_AES || (encryption->mode == MODE_AES_RSA && encryption->key != "")) {
        int i = 0;
        uint8_t *plan = (uint8_t*) encryption->plainText.c_str();
        uint8_t cy[16];
        memset(encryption->cypher, 0, 128);
        while (i < encryption->plainText.length())
        {
            EncryptFunction.aes.encryptBlock(cy, &plan[i]);
            // memcpy(&encryption->cypher[i], cy, 16);
            for(int j = 0; j < 16; j++) {
                encryption->cypher[i+j] = cy[j];
            }
            i += 16;
        }
        return pass;
    }
    if(encryption->mode == MODE_RSA) {
        encoderRSA(encryption->cypher, encryption->plainText, encryption->public_key, encryption->n);
    }
    return fail;
}

PROJECT_LOG decryptLora(IN _Decryption *decryption) {
    if(decryption->mode == MODE_NONE) {
        decryption->decryptedText = byte_array_to_string(decryption->cypher, 96);
        return pass;
    }
    if(decryption->mode == MODE_AES || (decryption->mode == MODE_AES_RSA && decryption->key != "")) {
        byte a[16];
        byte b[16];
        int i = 0;
        decryption->decryptedText = "";
        while (i < 96)
        {
            for(int j = 0; j < 16; j++) {
                //Serial.print((char)a[j]);
                b[j] =  (byte) decryption->cypher[i + j];
            }
            EncryptFunction.aes.decryptBlock(a, b);
            for(int j = 0; j < 16; j++) {
                //Serial.print((char)a[j]);
                decryption->decryptedText += (char)a[j];
            }
            i += 16;
        }
        
        return pass;
    }   
    return fail;
}