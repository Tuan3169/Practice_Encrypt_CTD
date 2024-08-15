#include <encrypt\rsa\rsa.h>
// #include <Arduino.h>

#include <math.h>
#include <numeric>

int prime[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 
                67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 
                139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 
                223, 227, 229};



void genKeysRSA(uint16_t &private_key, uint16_t &public_key, uint16_t &n)
{
    uint16_t prime1 = prime[esp_random()%50];          // first prime number
    uint16_t prime2 = prime[esp_random()%50];          // second prime number
    while (prime1 == prime2)
    {
        uint16_t prime2 = prime[esp_random()%50];
    }
    
    // to check the prime numbers selected
    // cout<<prime1<<" "<<prime2<<endl;
    n = prime1 * prime2;
    int fi = (prime1 - 1) * (prime2 - 1);
    int e = 2;
    while (1) {
        if (std::__gcd(e, fi) == 1)
            break;
        e++;
    } // d = (k*Φ(n) + 1) / e for some uint16_teger k
    public_key = e;
    uint16_t d = 2;
    while (1) {
        if ((d * e) % fi == 1)
            break;
        d++;
    }
    private_key = d;
}
// to encrypt the given number
uint16_t encryptRSA(uint16_t message, uint16_t public_key, uint16_t &n)
{
    uint32_t encrpyted_text = 1;
    while (public_key--) {
        encrpyted_text *= message;
        encrpyted_text %= n;
    }
    return encrpyted_text;
}
// to decrypt the given number
uint16_t decryptRSA(uint16_t encrpyted_text, uint16_t private_key, uint16_t &n)
{
    
    uint32_t decrypted = 1;
    while (private_key--) {
        decrypted *= encrpyted_text;
        decrypted %= n;
    }
    return decrypted;
}

void encoderRSA(uint16_t *cypher, String message, uint16_t public_key, uint16_t n)
{
    
    // calling the encrypting function in encoding function
    for (int i = 0; i < message.length(); i++)
        cypher[i] = encryptRSA((char)message[i], public_key, n);
    // return cypher;
}

String decoderRSA(uint16_t *cypher, uint16_t len, uint16_t private_key, uint16_t n)
{
    String s;
    // calling the decrypting function decoding function
    for (int i = 0; i < len; i++)
        s += (char) decryptRSA((uint16_t) cypher[i], private_key, n);
    return s;
}