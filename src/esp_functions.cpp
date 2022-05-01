/* Includes ------------------------------------------------------------------*/
#include "esp_functions.h"


#if defined(ARDUINO_ARCH_ESP8266)
	#include <LittleFS.h>
#elif defined(ARDUINO_ARCH_ESP32)
	#include <SPIFFS.h>
#endif

//-------------------------------------------------------------------------------
namespace esp {
	//-------------------------------------------------------------------------------
	uint8_t readAPconfig(char *ssid, char *key)
	{
		uint8_t result = 0;
		uint8_t ssidLen = 0;
		uint8_t keyLen = 0;

#if defined(ARDUINO_ARCH_ESP8266)
		if( LittleFS.exists( ESP_AP_CONFIG_FILE ) ){	
			File f = LittleFS.open( ESP_AP_CONFIG_FILE, "r");
#elif defined(ARDUINO_ARCH_ESP32)
		if( SPIFFS.exists( ESP_AP_CONFIG_FILE ) ){	
			File f = SPIFFS.open( ESP_AP_CONFIG_FILE, "r");
#endif
			if( f ){
				bool first = true;
				while( f.available() ){
					char sym;
					f.readBytes( &sym, 1 );
					if( first ){
						if( sym == '\n' ){
							first = false;
							ssid[ ssidLen ] = '\0';
							continue;
						}
						if( ssidLen >= ESP_CONFIG_SSID_MAX_LEN ) break;
						ssid[ ssidLen++ ] = sym;
					}else{
						if( sym == '\n' ){
							key[ keyLen ] = '\0';
							break;
						}
						if( keyLen >= ESP_CONFIG_KEY_MAX_LEN ) break;
						key[ keyLen++ ] = sym;
					}
				}
				f.close();
			}
		}

		if( ssidLen > 2 && keyLen >= 8 ) result = 1;

		return result;
	}

	//-------------------------------------------------------------------------------
	uint8_t saveConfig(const char *ssid, const char *key, uint8_t mode)
	{
		uint8_t result = 0;
#if defined(ARDUINO_ARCH_ESP8266)
		File f = LittleFS.open( ( mode == esp::AP_MODE ) ? ESP_AP_CONFIG_FILE : ESP_STA_CONFIG_FILE, "w");
#elif defined(ARDUINO_ARCH_ESP32)
		File f = SPIFFS.open( ( mode == esp::AP_MODE ) ? ESP_AP_CONFIG_FILE : ESP_STA_CONFIG_FILE, "w");
#endif
		if( f ){
#if defined(ARDUINO_ARCH_ESP8266)
			f.write( ssid );
			f.write( '\n' );
			f.write( key );
#elif defined(ARDUINO_ARCH_ESP32)
			f.write( (uint8_t*)ssid, strlen( ssid ) );
			f.write( '\n' );
			f.write( (uint8_t*)key, strlen( key ) );
#endif
			f.write( '\n' );
			f.close();
			result = 1;
		}

		return result;
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	uint8_t checkWebAuth(ESP8266WebServer *webServer, const char *user, const char *password, const char *realm, const char *failMess)
#elif defined(ARDUINO_ARCH_ESP32)
	uint8_t checkWebAuth(WebServer *webServer, const char *user, const char *password, const char *realm, const char *failMess)
#endif
	{
		delay( 1000 );

		if( !webServer->authenticate( user, password ) ){
			webServer->requestAuthentication( DIGEST_AUTH, realm, failMess );
			return 0;
		}

		return 1;
	}

	//-------------------------------------------------------------------------------
	uint8_t isClient()
	{
#if defined(ARDUINO_ARCH_ESP8266)
		return ( LittleFS.exists( ESP_STA_CONFIG_FILE ) ) ? 1 : 0;
#elif defined(ARDUINO_ARCH_ESP32)
		return ( SPIFFS.exists( ESP_STA_CONFIG_FILE ) ) ? 1 : 0;
#endif
	}

	//-------------------------------------------------------------------------------
	uint8_t readSTAconfig(char *ssid, char *key)
	{
		uint8_t result = 0;
		uint8_t ssidLen = 0;
		uint8_t keyLen = 0;

#if defined(ARDUINO_ARCH_ESP8266)
		if( LittleFS.exists( ESP_STA_CONFIG_FILE ) ){	
			File f = LittleFS.open( ESP_STA_CONFIG_FILE, "r");
#elif defined(ARDUINO_ARCH_ESP32)
		if( SPIFFS.exists( ESP_STA_CONFIG_FILE ) ){	
			File f = SPIFFS.open( ESP_STA_CONFIG_FILE, "r");
#endif
			if( f ){
				bool first = true;
				while( f.available() ){
					char sym;
					f.readBytes( &sym, 1 );
					if( first ){
						if( sym == '\n' ){
							first = false;
							ssid[ ssidLen ] = '\0';
							continue;
						}
						if( ssidLen >= ESP_CONFIG_SSID_MAX_LEN ) break;
						ssid[ ssidLen++ ] = sym;
					}else{
						if( sym == '\n' ){
							key[ keyLen ] = '\0';
							break;
						}
						if( keyLen >= ESP_CONFIG_KEY_MAX_LEN ) break;
						key[ keyLen++ ] = sym;
					}
				}
				f.close();
			}
		}

		if( ssidLen > 2 && keyLen >= 8 ) result = 1;

		return result;
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void setWebRedirect(ESP8266WebServer *webServer)
#elif defined(ARDUINO_ARCH_ESP32)
	void setWebRedirect(WebServer *webServer)
#endif
	{
		webServer->sendHeader( "Location", String( "http://" ) + webServer->client().localIP().toString(), true);
		webServer->send ( 302, "text/plain", "" );
		webServer->client().stop();
	}

	//-------------------------------------------------------------------------------
	uint8_t isWiFiConnection()
	{
		uint8_t state = 0;
		if( WiFi.status() == WL_CONNECTED ){
			state = ( WiFi.localIP().toString() == "0.0.0.0" ) ? 0 : 1 ;
		}
		return state;
	}

	//-------------------------------------------------------------------------------
	uint32_t getMyID()
	{
		#if defined(ARDUINO_ARCH_AVR)
			uint32_t _id =
				(uint32_t)boot_signature_byte_get(0x13) << 24 |
				(uint32_t)boot_signature_byte_get(0x15) << 16 |
				(uint32_t)boot_signature_byte_get(0x16) << 8 |
				(uint32_t)boot_signature_byte_get(0x17);
			return _id;
		#elif defined(ARDUINO_ARCH_ESP8266)
			return ESP.getChipId();
		#elif defined(ARDUINO_ARCH_ESP32)
			uint32_t _id = (uint32_t)((uint64_t)ESP.getEfuseMac() >> 16);
			return ((((_id)&0xff000000) >> 24) | (((_id)&0x00ff0000) >> 8) | (((_id)&0x0000ff00) << 8) | (((_id)&0x000000ff) << 24)); // swap bits
		#endif
		return 0;
	}

	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
}
