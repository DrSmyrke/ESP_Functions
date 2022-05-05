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
	enum{
		STA_MODE,
		AP_MODE,
	};
	/**
	 * read ssid and key for AP from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t readAPconfig(char *ssid, char *key);
	/**
	 * save ssid and key from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @param {uint8_t} mode
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t saveConfig(const char *ssid, const char *key, uint8_t mode);
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
	uint8_t isClient(void);
	/**
	 * read ssid and key for STA from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t readSTAconfig(char *ssid, char *key);
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
	uint8_t isWiFiConnection(void);
	/**
	 * get esp chip ID
	 * @return {uint32_t} ID or 0 if unknown device
	 */
	uint32_t getMyID(void);
	/**
	 * initialize wifi subsystem AP or STA
	 * @param {char*} hostname
	 * @param {IPAddress} ip
	 * @param {IPAddress} gateway
	 * @param {IPAddress} mask
	 * @return {bool} true if correct
	 */
	bool wifi_init(const char* hostname, const IPAddress &ip, const IPAddress &gateway, const IPAddress &mask);
	/**
	 * initialize wifi AP
	 * @param {char*} hostname
	 * @param {IPAddress} ip
	 * @param {IPAddress} gateway
	 * @param {IPAddress} mask
	 * @return {bool} true if correct
	 */
	bool wifi_AP_init(const char* hostname, const IPAddress &ip, const IPAddress &gateway, const IPAddress &mask);
	/**
	 * initialize wifi STA
	 * @param {char*} hostname
	 * @return {bool} true if correct
	 */
	bool wifi_STA_init(const char* hostname);
	/**
	 * checking acces from web
	 * @param {WebServer*} pointer
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void setNoCacheContent(ESP8266WebServer *webServer);
#elif defined(ARDUINO_ARCH_ESP32)
	void setNoCacheContent(WebServer *webServer);
#endif
}

//-------------------------------------------------------------------------------

#endif /* __ESP_FUNCTIONS_H__ */
