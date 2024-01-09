#ifndef __ESP_FUNCTIONS_H__
#define __ESP_FUNCTIONS_H__


//-------------------------------------------------------------------------------
#define ESP_CONFIG_SSID_MAX_LEN					32
#define ESP_CONFIG_KEY_MAX_LEN					32
#define ESP_FIRMWARE_FILENAME					"firmware.bin"
#define ESP_FIRMWARE_FILEPATH					"/firmware.bin"
#define ESP_CAPTIVE_PORTAL_URL					"/portal"
#define ESP_AUTOUPDATE_FILENAME					"/autoupdate"
#define ESP_FIRMWARE_VERSION_FILENAME			"/version"
#define USER_SETTINGS_FILE						"/settings.dat"
#define ESP_SYSTEM_CONFIG_FILE					"/config.dat"
#define SYSTEM_LOGIN							"admin"
#define SYSTEM_PASSWORD							"admin"
#define ESP_AUTH_REALM							"Dr.Smyrke TECH"
#ifndef DEFAULT_UPDATE_KEY
	#define DEFAULT_UPDATE_KEY					""
#endif
#ifndef DEFAULT_AP_KEY
	#define DEFAULT_AP_KEY						"1234567890"
#endif
#ifndef DEFAULT_DEVICE_NAME
	#define DEFAULT_DEVICE_NAME					"ESP_DEVICE"
#endif
#ifndef UPDATE_SIZE_UNKNOWN
	#define UPDATE_SIZE_UNKNOWN					0xFFFFFFFF
#endif
#ifndef FIRMWARE_REVISION
	#define FIRMWARE_REVISION					0
#endif

#define PROMISCUOUS_MODE_CHANNEL				7

//-------------------------------------------------------------------------------
#include <stdint.h>

#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
	#include <WebServer.h>
	#include <esp_wifi.h>
	#include <driver/can.h>
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
		//NOTE: If you change this enum, you must be change options in wifi page! Find ": Wifi mode options" in .cpp file
		enum{
			UNKNOWN,
			STA,
			AP,
			NO_WIFI,
			PROMISCUOUS,
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
	typedef struct {
		uint8_t mode;
		char ap_ssid[ ESP_CONFIG_SSID_MAX_LEN ];
		char ap_key[ ESP_CONFIG_KEY_MAX_LEN ];
		char sta_ssid[ ESP_CONFIG_SSID_MAX_LEN ];
		char sta_key[ ESP_CONFIG_KEY_MAX_LEN ];
	} Data;
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
	 * @param {IPAddress} ip
	 * @param {IPAddress} gateway
	 * @param {IPAddress} mask
	 * @return {bool} true if correct
	 */
	bool wifi_init(const IPAddress &ip, const IPAddress &gateway, const IPAddress &mask);
	/**
	 * initialize wifi AP
	 * @param {IPAddress} ip
	 * @param {IPAddress} gateway
	 * @param {IPAddress} mask
	 * @return {bool} true if correct
	 */
	bool wifi_AP_init(const IPAddress &ip, const IPAddress &gateway, const IPAddress &mask);
	/**
	 * initialize wifi STA
	 * @return {bool} true if correct
	 */
	bool wifi_STA_init(void);
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
	 * @param {char*} url redirect to Captive portal page (default: nullptr)
	 * @param {HandlerFunction} hanler function to Captive portal page (default: nullptr)
	 * @return none
	 */
#if defined(ARDUINO_ARCH_ESP8266)
	void activateCaptivePortal(ESP8266WebServer *webServer, const char* captiveRedirectTarget = ESP_CAPTIVE_PORTAL_URL, ESP8266WebServer::THandlerFunction cp_handler = nullptr);
#elif defined(ARDUINO_ARCH_ESP32)
	void activateCaptivePortal(WebServer *webServer, const char* captiveRedirectTarget = ESP_CAPTIVE_PORTAL_URL, WebServer::THandlerFunction cp_handler = nullptr);
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
	 * @param {char*} name - Device Name
	 * @param {boolean} useFS (default: true)
	 * @return {none}
	 */
	void init(const char* deviceName = DEFAULT_DEVICE_NAME, bool useFS = true);
	/**
	 * Change MAC (!!! use after Connection initialize)
	 * @return {none}
	 */
	void changeMAC(const uint8_t *mac);
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
	 * @param {uint32_t} length data (default: 0)
	 * @param {const char*} settingsFile - filepath (default: USER_SETTINGS_FILE)
	 * @return {none}
	 */
	void saveSettings(const uint8_t* data = nullptr, uint32_t length = 0, const char* settingsFile = USER_SETTINGS_FILE);
	/**
	 * Read settings at file from SPI FS
	 * @param {uint8_t*} data buffer (default: nullptr)
	 * @param {uint32_t} length data (default: 0)
	 * @param {const char*} settingsFile - filepath (default: USER_SETTINGS_FILE)
	 * @return {uint32_t} reading length
	 */
	uint32_t loadSettings(uint8_t* data = nullptr, uint32_t size = 0, const char* settingsFile = USER_SETTINGS_FILE);
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
	 * Get mode value
	 * @return {uint8_t} esp::Mode enum
	 */
	uint8_t getMode(void);
	/**
	 * Set mode value
	 * @param {const uint8_t} value must be lower at esp::Mode enum
	 * @return none
	 */
	void setMode(const uint8_t value);
	/**
	 * Save system settings (internal method don`t use this method!!!)
	 * @return {none}
	 */
	void saveSystemSettings(void);
	/**
	 * Get reset reason
	 * @return {uint32_t}
	 */
	uint32_t getResetReason(void);
	/**
	 * Enable Promisc mode method
	 * @param {Callback function} (default: nullptr)
	 * @return none
	*/
	void enablePromiscMode(wifi_promiscuous_cb_t func = nullptr);
	/**
	 * Disable Promisc mode method
	 * @return none
	*/
	void disablePromiscMode(void);
	/**
	 * Callback to recieve RAW data
	 * @return none
	 */
	void promisc_rx_cb(uint8_t *buf, uint16_t len);
	
#if defined(ARDUINO_ARCH_ESP8266)

#elif defined(ARDUINO_ARCH_ESP32)
	/**
	 * Initialize CAN module
	 * @param {uint16_t} CAN Speed
	 * @param {gpio_num_t} tx pin
	 * @param {gpio_num_t} rx pin
	 * @param {can_mode_t} mode (default: CAN_MODE_NORMAL)
	 * @return none
	 */
	void CAN_Init(const uint16_t speed, const gpio_num_t tx_pin, const gpio_num_t rx_pin, const can_mode_t mode = CAN_MODE_NORMAL);
	/**
	 * Getter for CAN speed value
	 * @return {uint16_t} speed value
	 */
	uint16_t get_CAN_speed(void);
#endif
	/**
	 * Print data at HEX format
	 * @return {const uint8_t*} data pointer
	 * @return {size_t} data length
	 * @return none
	*/
	void printHexData(const uint8_t* data, size_t len);
}

//-------------------------------------------------------------------------------

#endif /* __ESP_FUNCTIONS_H__ */
