/*
 * Ramure.h
 *
 *  Created on: 18 juin 2025
 *      Author: Patrick
 */

#ifndef RAMURE_V3_H_
#define RAMURE_V3_H_

#include "Arduino.h"

#include <string.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
// #include <MCP24LC512.h>

#ifndef RAMURE_LVGL
#define RAMURE_LVGL 1       /**< 1 to enable LVGL, 0 to disable */
#endif
#define RAMURE_SERIAL 1     /**< 1 to enable serial, 0 to disable */
#define RAMURE_WATCHDOG 1   /**< 1 to enable watchdog, 0 to disable */
#define RAMURE_BENCH 1      /**< 1 to enable bench, 0 to disable */

#define RAMURE_FAN  0   /* Fan non utilisé */
#define SYSTEM_SN  0       /*   LPCA /MOD 0 systeme spare_LPC , 1 systeme Modane  */


#if RAMURE_WATCHDOG
#include <IWatchdog.h>
#endif

#include <PSU.h>

#include <ArduinoJson.h>
#include <STM32SD.h>

#include <NTPClient.h>

#include <STM32RTC.h>


/**
 * @enum
 * @brief
 *
 */
enum : uint8_t {
    CLIENT_MQTT,    /**< CLIENT_MQTT */
    CLIENT_SMS,     /**< CLIENT_SMS */
    CLIENT_COUNT,   /**< CLIENT_COUNT */
};

#include <STM32Ethernet.h>
#include <Dns.h>
#include <PubSubClient.h>
#include <SMS.h>

#include <lvgl.h>


#include <SDRAM.h>
#include <LTDC.h>
#include <FT5336.h>

/**
 * | Pin | Color | Function |
 * |:---:|:------|:---------|
 * | 1   | Red   | +5V      |
 * | 2   | Green | GND      |
 * | 3   | Blue  | DATA     |
 */
#include <DS18B20.h>

constexpr uint8_t DS18B20_DELAY = 10;
constexpr uint8_t DS18B20_COUNT = 5;
constexpr uint8_t DS18B20_PINS[DS18B20_COUNT] = { D2, D3, D4, D5, D6 };

constexpr uint8_t NSL19M51_PIN = A0;

//constexpr uint8_t SN_PIN =  A1;// 15 ; //

constexpr uint8_t RELAYS_COUNT = 6;
constexpr uint8_t RELAYS_PINS[RELAYS_COUNT] = { D11, D12, D9, D10, D7, D8 };

constexpr uint8_t POWER_COUNT = 2;


String path_mqtt_publish ="";
String path_mqtt_subscribe ="";
String msg_alarm_int ="";
String  msg_alarm_ext ="";
String UTC_time = "";

// constexpr uint8_t SYSTEM_SN = 0; /*   LPCA /MOD 0 systeme spare_LPC , 1 systeme Modane  */
/**
 * @struct Voltage
 * @brief
 *
 */
struct Voltage {
    double set;
    double get;
    double out;
};

/**
 * @struct Current
 * @brief
 *
 */
struct Current {
    double set;
    double get;
    double out;
};

/**
 * @enum
 * @brief
 *
 */
enum : uint8_t {
    SERVICE_MQTT,/**< SERVICE_MQTT */
    SERVICE_NTP, /**< SERVICE_NTP */
    SERVICE_COUNT/**< SERVICE_COUNT */
};

#define RAMURE_JSON "ramure.json"

constexpr uint32_t RAMURE_MQTT_DELAY = 1;
constexpr double RAMURE_TARGET = 22.0;

class Host {
public:
    enum : uint8_t {
        None,
        Name,
        Address
    };

    Host(const String host) : _flag(None) {
        if (_address.fromString(host)) {
            _flag = Address;
        } else {
            _flag = Name;
        }
        _name = host;
    }

    IPAddress address() {
        if (_flag == Name) {
            DNSClient dns;
            dns.begin(Ethernet.dnsServerIP());
            dns.getHostByName(_name.c_str(), _address);
        }
        return _address;
    }
    
    String name() {
        return _name;
    }
private:
    uint8_t _flag = None;
    String _name;
    IPAddress _address;
};

/**
 * @struct NTP
 * @brief
 *
 */
struct NTP {
    IPAddress server;
    uint16_t port;
};

/**
 * @struct MQTT
 * @brief
 *
 */
struct MQTT {
    IPAddress server;
    uint16_t port;
    String topic;
    String topic2;
    bool authentication;
    String username;
    String password;
};

/**
 * @struct Network
 * @brief
 *
 */
struct Network {
    bool dhcp;
    IPAddress local;
    IPAddress subnet;
    IPAddress gateway;
    IPAddress dns;
    NTP ntp;
    MQTT mqtt;
};

constexpr double ALARM_INTERNAL = 30.0; // a remettre a 19 pour test reel
constexpr double ALARM_EXTERNAL = 30.0;

constexpr uint32_t ALARM_DELAY = 600; // temps en secondes

/**
 * @struct Alarm
 * @brief
 *
 */
struct Alarm {
    enum : uint8_t {
        Internal,
        External,
        Count
    };
    bool active;
    bool sms;
    uint32_t last;
};


struct Time {
	uint8_t annee;
	uint8_t mois;
	uint8_t jour;
	uint8_t heure;
	uint8_t minutes;
	uint8_t secondes;
};

/**
 * @struct Worker
 * @brief
 *
 */
struct Worker {
    uint8_t relays;
    uint8_t probes;
    uint32_t light;
 //   uint32_t sn;
    double temperatures[DS18B20_COUNT];
    uint8_t status;
    bool regulate;
    uint32_t delay;
    double target;
    Voltage voltage;
    Current current;
    Network network;
    String mobile;
    Time  time ;
    bool store;
    uint32_t last;
    Alarm alarms[Alarm::Count];

};



#endif /* RAMURE_V3_H_ */
