/* Includes ------------------------------------------------------------------*/
#include "esp_functions.h"

#include <LittleFS.h>

//-------------------------------------------------------------------------------
namespace esp {
	//-------------------------------------------------------------------------------
	uint8_t readAPconfig(char *ssid, char *key)
	{
		uint8_t result = 0;
		uint8_t ssidLen = 0;
		uint8_t keyLen = 0;

		if( LittleFS.exists( ESP_AP_CONFIG_FILE ) ){	
			File f = LittleFS.open( ESP_AP_CONFIG_FILE, "r");
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
	uint8_t saveAPconfig(const char *ssid, const char *key)
	{
		uint8_t result = 0;

		File f = LittleFS.open( ESP_AP_CONFIG_FILE, "w");
		if( f ){
			f.write( ssid );
			f.write( '\n' );
			f.write( key );
			f.write( '\n' );
			f.close();
			result = 1;
		}

		return result;
	}

	//-------------------------------------------------------------------------------
	uint8_t checkWebAuth(ESP8266WebServer *webServer, const char *user, const char *password, const char *realm, const char *failMess)
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
		return ( LittleFS.exists( ESP_STA_CONFIG_FILE ) ) ? 1 : 0;
	}

	//-------------------------------------------------------------------------------
	uint8_t readSTAconfig(char *ssid, char *key)
	{
		uint8_t result = 0;
		uint8_t ssidLen = 0;
		uint8_t keyLen = 0;

		if( LittleFS.exists( ESP_STA_CONFIG_FILE ) ){	
			File f = LittleFS.open( ESP_STA_CONFIG_FILE, "r");
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
	uint8_t saveSTAconfig(const char *ssid, const char *key)
	{
		uint8_t result = 0;

		File f = LittleFS.open( ESP_STA_CONFIG_FILE, "w");
		if( f ){
			f.write( ssid );
			f.write( '\n' );
			f.write( key );
			f.write( '\n' );
			f.close();
			result = 1;
		}

		return result;
	}

	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
}
