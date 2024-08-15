#ifndef SENSOR_H
#define SENSOR_H

#include <PZEM004Tv30.h>
#include <driver/sensor/data.hpp>
#include <config.hpp>

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif


class Sensor {
    PZEM004Tv30 *pzem;

    public:
    Sensor() {pzem = new PZEM004Tv30(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);}
    ~Sensor() {delete pzem;}

    uint8_t check() {if(pzem) return pass; else return fail;};
    uint8_t init() {
        if(!pzem)  {
            pzem = new PZEM004Tv30(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
        }
        if(pzem) {return pass;} else {return fail;}
    }
    uint8_t readAddress() {return pzem->getAddress();}
    bool resetEnergy() {return pzem->resetEnergy();}
    bool setAddress(uint8_t address) {return pzem->setAddress(address);}
    bool readData(Data *data) {

        if(
            data->setData(
                pzem->voltage(),
                pzem->current(),
                pzem->power(),
                pzem->energy(),
                pzem->frequency(),
                pzem->pf(),
                "",
                ""
            )) 
        {
            return pass;
        } else {
            return pass;
        }
    }
};

#endif