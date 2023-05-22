#ifndef __ESP_FUNCTIONS_H__
#define __ESP_FUNCTIONS_H__


//-------------------------------------------------------------------------------
#define ESP_STA_CONFIG_FILE						"/staConfig"
#define ESP_AP_CONFIG_FILE						"/apConfig"
#define ESP_CONFIG_SSID_MAX_LEN					32
#define ESP_CONFIG_KEY_MAX_LEN					32
#define ESP_FIRMWARE_FILENAME					"firmware.bin"
#define ESP_FIRMWARE_FILEPATH					"/firmware.bin"
#define ESP_AUTOUPDATE_FILENAME					"/autoupdate"
#define ESP_FIRMWARE_VERSION_FILENAME			"/version"
#define USER_SETTINGS_FILE						"/settings.dat"
#define SYSTEM_LOGIN							"admin"
#define SYSTEM_PASSWORD							"admin"
#define ESP_AUTH_REALM							"Dr.Smyrke TECH"
#ifndef DEFAULT_UPDATE_KEY
	#define DEFAULT_UPDATE_KEY					""
#endif
#ifndef DEFAULT_AP_KEY
	#define DEFAULT_AP_KEY						"1234567890"
#endif
#ifndef UPDATE_SIZE_UNKNOWN
	#define UPDATE_SIZE_UNKNOWN					0xFFFFFFFF
#endif

//-------------------------------------------------------------------------------
#include <stdint.h>

#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WebServer.h>
#endif

//-------------------------------------------------------------------------------
#ifdef DEBUG_ESP
	#ifdef DEBUG_ESP_PORT
		#define ESP_DEBUG(fmt, ...) DEBUG_ESP_PORT.printf_P( (PGM_P)PSTR(fmt), ##__VA_ARGS__ )
	#endif
#else
	#define ESP_DEBUG(...) asm("nop")
#endif

//-------------------------------------------------------------------------------
namespace esp {
	struct Mode{
		enum{
			UNKNOWN,
			STA,
			AP,
			NO_WIFI,
		};
	};
	typedef struct {
		unsigned char captivePortal: 1;
		unsigned char captivePortalAccess: 1;
		unsigned char autoUpdate: 1;
		unsigned char useFS: 1;
		unsigned char updateError: 1;
		unsigned char updateFirmware: 1;
		unsigned char updateFile: 1;
	} Flags;
	extern Flags flags;
	extern int8_t countNetworks;
	extern const char* pageTop;
	extern const char* pageEndTop;
	extern const char* pageBottom;
	extern char* pageBuff;
	extern char tmpVal[ 11 ];
	extern uint8_t firstVersion;
	extern uint8_t secondVersion;
	extern uint16_t thridVersion;
	
	/**
	 * read ssid and key from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t readConfig(char *ssid, char *key);
	/**
	 * save ssid and key from spi fs
	 * @param {char*} ssid
	 * @param {char*} key
	 * @return {uint8_t} result ( 1 - success, 0 - error )
	 */
	uint8_t saveConfig(const char *ssid, const char *key);
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
	 * remove from spi fs
	 * @param {char*} filepath
	 * @return none
	 */
	void removeFile(const char* file);
	/**
	 * set redirect to / from web
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void setWebRedirect(ESP8266WebServer *webServer, const String &target = "");
#elif defined(ARDUINO_ARCH_ESP32)
	void setWebRedirect(WebServer *webServer, const String &target = "");
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
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void addWebServerPages(ESP8266WebServer *webServer, bool wifiConfig = true, bool notFound = true);
#elif defined(ARDUINO_ARCH_ESP32)
	void addWebServerPages(WebServer *webServer, bool wifiConfig = true, bool notFound = true);
#endif
	/**
	 * add web server update logic
	 * @param {WebServer*} pointer
	 * @param {const char*} key (default: #define DEFAULT_UPDATE_KEY)
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void addWebUpdate(ESP8266WebServer *webServer, const char* key = DEFAULT_UPDATE_KEY);
#elif defined(ARDUINO_ARCH_ESP32)
	void addWebUpdate(WebServer *webServer, const char* key = DEFAULT_UPDATE_KEY);
#endif
	/**
	 * Activate web server Captive Portal logic
	 * @param {WebServer*} pointer
	 * @param {HandlerFunction} hanler function to Captive portal page (default: nullptr)
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void activateCaptivePortal(ESP8266WebServer *webServer, const char* captiveRedirectTarget = "/", ESP8266WebServer::THandlerFunction cp_handler = nullptr);
#elif defined(ARDUINO_ARCH_ESP32)
	void activateCaptivePortal(WebServer *webServer, const char* captiveRedirectTarget = "/", WebServer::THandlerFunction cp_handler = nullptr);
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
	uint8_t webSendFile(ESP8266WebServer *webServer, const char* fileName, const char* mimeType, const uint16_t code = 200);
#elif defined(ARDUINO_ARCH_ESP32)
	uint8_t webSendFile(WebServer *webServer, const char* fileName, const char* mimeType, const uint16_t code = 200);
#endif
	/**
	 * Checking for updates 
	 * @param {char*} repository url (http://example.com/folder)
	 * @param {uint16_t} number of version
	 * @return {uint32_t} available version number
	 */
	uint32_t checkingUpdate(const char *repoURL, const uint16_t version);
	/**
	 * Download new files
	 * @param {char*} repository url (http://example.com/folder)
	 * @param {char*} file (/firmware.bin)
	 * @return {uint8_t} 0 if error, 1 if success
	 */
	uint8_t downloadUpdate(const char *repoURL, const char *file);
	/**
	 * Update firmware from FS
	 * @return {uint8_t} 0 if error, 1 if success
	 */
	uint8_t updateFromFS(void);
	/**
	 * Checking file in memory 
	 * @param {char*} file path
	 * @return {bool} available version number
	 */
	bool isFileExists(const char *filepath);
	/**
	 * Printing all files in memory
	 * @param {HardwareSerial} Serial object
	 * @return {none}
	 */
	void printAllFiles(HardwareSerial &SerialPort);
	/**
	 * Initialize for library methods
	 * @param {boolean} useFS (default: true)
	 * @return {none}
	 */
	void init(bool useFS = true);
	/**
	 * Change MAC (!!! use after Connection initialize)
	 * @return {none}
	 */
	void changeMAC(const uint8_t *mac);
	/**
	 * Captive portal page
	 * @return {none}
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void captivePortalPage(ESP8266WebServer *webServer, ESP8266WebServer::THandlerFunction cp_handler = nullptr);
#elif defined(ARDUINO_ARCH_ESP32)
	void captivePortalPage(WebServer *webServer, WebServer::THandlerFunction cp_handler = nullptr);
#endif
	/**
	 * HTTP Put request
	 * @param {String &} URL
	 * @param {String &} playload
	 * @param {String &} response
	 * @return {int} http response code
	 */
	int http_put(const String &url, const String &playload, String &response);
	/**
	 * Save settings at file from SPI FS
	 * @param {const uint8_t*} data buffer (default: nullptr)
	 * @param {uint8_t} length data (default: 0)
	 * @param {const char*} settingsFile - filepath (default: USER_SETTINGS_FILE)
	 * @return {none}
	 */
	void saveSettings(const uint8_t* data = nullptr, uint8_t length = 0, const char* settingsFile = USER_SETTINGS_FILE);
	/**
	 * Read settings at file from SPI FS
	 * @param {uint8_t*} data buffer (default: nullptr)
	 * @param {size_t} length data (default: 0)
	 * @param {const char*} settingsFile - filepath (default: USER_SETTINGS_FILE)
	 * @return {uint8_t} reading length
	 */
	uint8_t loadSettings(uint8_t* data = nullptr, size_t size = 0, const char* settingsFile = USER_SETTINGS_FILE);
	/**
	 * Set system user and password
	 * @param {const char*} login (default: nullptr)
	 * @param {const char*} password (default: nullptr)
	 * @return {none}
	 */
	void changeSystemUserPassword(const char* login = nullptr, const char* password = nullptr);
	/**
	 * Update process function
	 * @return {none}
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void updateProcess(ESP8266WebServer *webServer);
#elif defined(ARDUINO_ARCH_ESP32)
	void updateProcess(WebServer *webServer);
#endif
	/**
	 * Set application version
	 * @param {const uint8_t} first version <FIRST>.1.256
	 * @param {const uint8_t} second version 0.<SECOND>.256
	 * @param {const uint16_t} thrid version 0.1.<THRID>
	 * @return {none}
	 */
	void setVersion(const uint8_t first = 0, const uint8_t second = 1, const uint16_t thrid = 0);
	/**
	 * Update mode value (internal method !!!)
	 * @return {none}
	 */
	void updateMode(void);
	/**
	 * Get mode value
	 * @return {uint8_t} esp::Mode enum
	 */
	uint8_t getMode(void);
	/**
	 * Set mode value
	 * @param {const uint8_t} value must be lower at esp::Mode enum
	 * @return {none}
	 */
	void setMode(const uint8_t value);
}

//-------------------------------------------------------------------------------

#endif /* __ESP_FUNCTIONS_H__ */
