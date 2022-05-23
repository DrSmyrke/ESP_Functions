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
	typedef struct {
		unsigned char ap_mode: 1;
		unsigned char captivePortalAccess: 1;
	} Flags;
	extern Flags flags;
	extern int8_t countNetworks;
	extern const char* pageTop;
	extern const char* pageEndTop;
	extern const char* pageBottom;
	extern char* pageBuff;
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
	/**
	 * add web server default pages callback
	 * @param {WebServer*} pointer
	 * @param {bool} wifi config page (default: true)
	 * @param {bool} not found page (default: true)
	 * @param {HandlerFunction} hanler function to Captive portal page (default: nullptr)
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void addWebServerPages(ESP8266WebServer *webServer, bool wifiConfig = true, bool notFound = true, ESP8266WebServer::THandlerFunction cp_handler = nullptr);
#elif defined(ARDUINO_ARCH_ESP32)
	void addWebServerPages(WebServer *webServer, bool wifiConfig = true, bool notFound = true, WebServer::THandlerFunction cp_handler = nullptr);
#endif
	/**
	 * web config page
	 * @param {WebServer*} pointer
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void handleWebConfigPage(ESP8266WebServer *webServer);
#elif defined(ARDUINO_ARCH_ESP32)
	void handleWebConfigPage(WebServer *webServer);
#endif
	/**
	 * web not found page
	 * @param {WebServer*} pointer
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void handleWeb404Page(ESP8266WebServer *webServer);
#elif defined(ARDUINO_ARCH_ESP32)
	void handleWeb404Page(WebServer *webServer);
#endif
	/**
	 * web send file to client or generate 404 error if file not found
	 * @param {WebServer*} pointer
	 * @param {char*} fileName
	 * @param {char*} mimeType
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	uint8_t webSendFile(ESP8266WebServer *webServer, char* fileName, char* mimeType);
#elif defined(ARDUINO_ARCH_ESP32)
	uint8_t webSendFile(WebServer *webServer, char* fileName, char* mimeType);
#endif
}

//-------------------------------------------------------------------------------

#endif /* __ESP_FUNCTIONS_H__ */
