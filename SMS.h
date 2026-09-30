/*
 * SMS.h
 *
 *  Created on: 23 juin 2025
 *      Author: Patrick
 */

#ifndef SMS_H_
#define SMS_H_

#include <Client.h>

/**
 * @class SMS
 * @brief
 *
 */
class SMS {
public:
    /**
     * @brief
     *
     * @param client
     */
    SMS(Client& client) {
        _counter = 0;
        _client = &client;
    }

    /**
     * @brief
     *
     * @param experience
     * @return
     */
    bool begin(const char* experience) {
        if (_client->connect("sms.in2p3.fr", 4377)) {
            String command("ENR ");
            command += experience;
            _client->println(command);
            String response = _client->readString();
            if (response.startsWith("EOK")) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief
     *
     * @param experience
     * @return
     */
    bool begin(const String& experience) {
        return begin(experience.c_str());
    }

    /**
     * @brief
     *
     * @param mobile
     * @param message
     * @return
     */
    bool send(const char* mobile, const char* message) {
        String command("SMS ");
        command += mobile;
        command += " ";
        command += _counter++;
        command += " ";
        command += message;
        _client->println(command);
        String response = _client->readString();
        if (response.startsWith("SOK")) {
            return true;
        }
        return false;
    }

    /**
     * @brief
     *
     * @param mobile
     * @param message
     * @return
     */
    bool send(const String &mobile, const String &message) {
        return send(mobile.c_str(), message.c_str());
    }

    /**
     * @brief
     *
     */
    void end(const bool close = false) {
        _client->println("DEC");
        if (close) {
            _client->stop();
        }
    }

private:
    uint8_t _counter;   /**< */
    Client* _client;    /**< */
};

#endif /* SMS_H_ */
