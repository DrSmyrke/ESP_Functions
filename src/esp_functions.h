#ifndef __ESP_FUNCTIONS_H__
#define __ESP_FUNCTIONS_H__


//-------------------------------------------------------------------------------
#define ESP_STA_CONFIG_FILE						"/staConfig"
#define ESP_AP_CONFIG_FILE						"/apConfig"
#define ESP_CONFIG_SSID_MAX_LEN					32
#define ESP_CONFIG_KEY_MAX_LEN					32

//-------------------------------------------------------------------------------
#include <stdint.h>

#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WebServer.h>
#endif

//-------------------------------------------------------------------------------
namespace esp {
	/**
	 * read ssid and key for AP from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t readAPconfig(char *ssid, char *key);
	/**
	 * save ssid and key for AP from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t saveAPconfig(const char *ssid, const char *key);
	/**
	 * checking acces from web
	 * @param {WebServer*} user
	 * @param {char*} user
	 * @param {char*} password
	 * @param {char*} realm
	 * @param {char*} failMess
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	uint8_t checkWebAuth(ESP8266WebServer *webServer, const char *user, const char *password, const char *realm, const char *failMess);
#elif defined(ARDUINO_ARCH_ESP32)
	uint8_t checkWebAuth(WebServer *webServer, const char *user, const char *password, const char *realm, const char *failMess);
#endif
	/**
	 * checking STA or AP mode
	 * @return none
	 */
	uint8_t isClient();
	/**
	 * read ssid and key for STA from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t readSTAconfig(char *ssid, char *key);
	/**
	 * save ssid and key for STA mode from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t saveSTAconfig(const char *ssid, const char *key);
	/**
	 * set redirect to / from web
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void setWebRedirect(ESP8266WebServer *webServer);
#elif defined(ARDUINO_ARCH_ESP32)
	void setWebRedirect(WebServer *webServer);
#endif
	/**
	 * checking Connections for AP from STA mode
	 * @return none
	 */
	uint8_t isWiFiConnection();
	/**
	 * get esp chip ID
	 * @return {uint32_t} ID or 0 if unknown device
	 */
	uint32_t getMyID();
}

//-------------------------------------------------------------------------------

#endif /* __ESP_FUNCTIONS_H__ */
