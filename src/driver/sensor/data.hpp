#ifndef DATA_H
#define DATA_H

#include <cmath>
#include <config.hpp>

class Data
{
private:
    
    float volt, current, power, energy, frequency, pf;
    String time, macAddress;
public:
    Data() {
        this->volt = this->current = this->power = this->energy = this->frequency = this->pf = -1;
        this->time = "1/1/2000 00:00:00";
        this->macAddress = "0:0:0:0:0";
    }
    Data(float volt,float current, float power,float energy,float frequency,float pf, String time, String macAddress) {
        if(std::isnan(volt) || std::isnan(current) || std::isnan(power) || std::isnan(energy) || std::isnan(frequency) || std::isnan(pf)) {
            volt = current = power = energy = frequency = pf = -1;
            return;
        }
        this->volt = volt;
        this->current = current;
        this->power = power;
        this->energy = energy;
        this->frequency = frequency;
        this->pf = pf;
        this->time = time;
        this->macAddress = macAddress;
    };
    
    ~Data() {};
    bool checkData() {
        if(this->volt==-1 || this->current==-1 || this->power==-1 || this->energy==-1 || this->frequency==-1 || this->pf==-1) {
            return fail;
        } else {
            return pass;
        }
    }
    bool setData(float volt,float current, float power,float energy,float frequency,float pf, String time, String macAddress) {
        if(std::isnan(volt) || std::isnan(current) || std::isnan(power) || std::isnan(energy) || std::isnan(frequency) || std::isnan(pf)) {
            volt = current = power = energy = frequency = pf = -1;
            return fail;
        }
        
        this->volt = volt;
        this->current = current;
        this->power = power;
        this->energy = energy;
        this->frequency = frequency;
        this->pf = pf;
        this->time = time;
        this->macAddress = macAddress;
        return pass;
    }
    // void set(float v) {this->volt = v;}
    void setTime(String time) {this->time = time;}
    void setMac(String macAddress) {this->macAddress = macAddress;}
    float getVolt(){return this->volt;}
    float getCurent() {return this->current;}
    float getPower() {return this->power;}
    float getEnery() {return this->energy;}
    float getFrequency() {return this->frequency;}
    float getPF() {return this->pf;}
    void setVolt(float volt) {this->volt = volt;}
    void setCurent(float current) {this->current = current;}
    void setPower(float power) {this->power = power;}
    void setEnery(float energy) {this->energy = energy;}
    void setFrequency(float frequency) {this->frequency = frequency;}
    void setPF(float pf) {this->pf = pf;} 
    String getTime() {return this->time;}
    String getMac() {return this->macAddress;}
};

#endif