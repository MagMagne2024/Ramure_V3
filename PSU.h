/**
 * @file
 *
 * @brief TENMA PSU.
 *
 * @author Patrick Lafarguette
 *
 * @date 18/06/2025
 *
 * Detailed description, etc.
 */

#ifndef PSU_H_
#define PSU_H_

#include <Arduino.h>

/**
 * @class PSU
 * @brief A class to handle TENMA PSU.
 *
 * TENMA PSU uses RS-232 connection
 *
 */
class PSU {
public:
    /**
     * @enum Status
     * @brief
     *
     */
    enum Status : uint8_t {
        CV = (1 << 0),      /**< CV */
        Output = (1 << 6)   /**< Output */
    };

    /**
     * @enum State
     * @brief
     *
     */
    enum State : uint8_t {
        Off,    /**< Off */
        On      /**< On */
    };

    /**
     * @brief
     *
     */
    PSU() : _serial(nullptr) {
    }

    /**
     * @brief
     *
     */
    virtual ~PSU() {
        _serial->end();
    }

    /**
     * @brief
     *
     * @param serial
     */
    void begin(HardwareSerial &serial) {
        _serial = &serial;
        _serial->begin(9600);
    }

    /**
     * @brief
     *
     * @param state
     */
    void lock(const uint8_t state) {
        if (state == State::On) {
            _serial->print("LOCK1");
        } else {
            _serial->print("LOCK0");
        }
        delay(500);
    }

    /**
     * @brief
     *
     * @param current
     */
    void iset(const double current) {
        _serial->print("ISET1:");
        _serial->print(current);
        delay(500);
    }

    /**
     * @brief
     *
     * @return
     */
    double iset() {
        return read("ISET1?");
    }

    /**
     * @brief
     *
     * @param voltage
     */
    void vset(const double voltage) {
        _serial->print("VSET1:");
        _serial->print(voltage);
        delay(500);
    }

    /**
     * @brief
     *
     * @return
     */
    double vset() {
        return read("VSET1?");
    }

    /**
     * @brief
     *
     * @return
     */
    double iout() {
        return read("IOUT1?");
    }

    /**
     * @brief
     *
     * @return
     */
    double vout() {
        return read("VOUT1?");
    }

    /**
     * @brief Turns on or off the output.
     *
     * @param state
     */
    void out(const uint8_t state) {
        if (state == State::On) {
            _serial->print("OUT1");
        } else {
            _serial->print("OUT0");
        }
    }

    /**
     * @brief
     *
     * @return
     */
    const uint8_t status() {
        _serial->print("STATUS?");
        return _serial->readString()[0];
    }

    /**
     * @brief Identifies the PSU.
     *
     * ```
     * TENMA 72-2710 V2.5
     * ```
     *
     * @return
     */
    String idn() {
        _serial->print("*IDN?");
        delay(500);
        return _serial->readString();
    }

private:
    HardwareSerial *_serial;    /**< */

    /**
     * @brief
     *
     * @param command
     * @return
     */
    double read(const String &command) {
        _serial->print(command);
        delay(500);
        return _serial->readString().toDouble();
    }
};

#endif /* PSU_H_ */
