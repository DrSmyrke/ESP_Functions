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
	Flags flags;
	int8_t countNetworks;
	const char* pageTop = nullptr;
	const char* pageEndTop = nullptr;
	const char* pageBottom = nullptr;
	char* pageBuff = nullptr;

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
	uint8_t isClient(void)
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
	{	String location = "http://" + String( WiFi.getHostname() ) + ".lan/";
		// String location = "http://" + webServer->client().localIP().toString();
		webServer->sendHeader( "Location", location, true);
		webServer->send ( 302, "text/plain", "" );
		webServer->client().stop();
	}

	//-------------------------------------------------------------------------------
	uint8_t isWiFiConnection(void)
	{
		uint8_t state = 0;
		if( WiFi.status() == WL_CONNECTED ){
			state = ( WiFi.localIP().toString() == "0.0.0.0" ) ? 0 : 1 ;
		}
		return state;
	}

	//-------------------------------------------------------------------------------
	uint32_t getMyID(void)
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
	bool wifi_init(const char* hostname, const IPAddress &ip, const IPAddress &gateway, const IPAddress &mask)
	{
		if( esp::isClient() ){
			return wifi_STA_init( hostname );
		}else{
			return wifi_AP_init( hostname, ip, gateway, mask );
		}
	}

	//-------------------------------------------------------------------------------
	bool wifi_AP_init(const char* hostname, const IPAddress &ip, const IPAddress &gateway, const IPAddress &mask)
	{
		esp::countNetworks = WiFi.scanNetworks();

		WiFi.hostname( hostname );
		WiFi.disconnect();

		WiFi.mode( WiFiMode_t::WIFI_AP );
		bool res = WiFi.softAP( hostname, "1234567890" );
		delay( 50 );
		WiFi.softAPConfig( ip, gateway, mask );

		delay( 600 );

		esp::flags.ap_mode = 1;

		return res;
	}

	//-------------------------------------------------------------------------------
	bool wifi_STA_init(const char* hostname)
	{
		char ssid[ ESP_CONFIG_SSID_MAX_LEN ];
		char skey[ ESP_CONFIG_KEY_MAX_LEN ];

		WiFi.hostname( hostname );
		WiFi.softAPdisconnect( true );

		WiFi.mode( WiFiMode_t::WIFI_STA );

		if( esp::readSTAconfig( ssid, skey ) ){
			WiFi.begin( ssid, skey );
		}else{
			ESP.restart();
			return false;
		}

		delay( 5000 );

		if( !esp::isWiFiConnection() ){
			delay( 1000 );
			ESP.restart();
			return false;
		}

		esp::flags.ap_mode = 0;

		return true;
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void setNoCacheContent(ESP8266WebServer *webServer)
#elif defined(ARDUINO_ARCH_ESP32)
	void setNoCacheContent(WebServer *webServer)
#endif
	{
		webServer->sendHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
		webServer->sendHeader( "Pragma", "no-cache" );
		webServer->sendHeader( "Expires", "-1" );
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void addWebServerPages(ESP8266WebServer *webServer, bool wifiConfig, bool notFound, ESP8266WebServer::THandlerFunction cp_handler)
#elif defined(ARDUINO_ARCH_ESP32)
	void addWebServerPages(WebServer *webServer, bool wifiConfig, bool notFound, WebServer::THandlerFunction cp_handler)
#endif
	{
		if( wifiConfig ){
			webServer->on( "/wifi", [ webServer ](void){
				esp::handleWebConfigPage( webServer );
			} );
		}
		if( notFound ){
			webServer->onNotFound( [ webServer ](void){
				esp::handleWeb404Page( webServer );
			} );
		}

		if( cp_handler != nullptr ){
			webServer->on( "/fwlink", cp_handler );
			webServer->on( "/generate_204", cp_handler );
			webServer->on( "/favicon.ico", cp_handler );
		}
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void handleWebConfigPage(ESP8266WebServer *webServer)
#elif defined(ARDUINO_ARCH_ESP32)
	void handleWebConfigPage(WebServer *webServer)
#endif
	{
		//-------------------------------------------------------------
		if( webServer->hasArg( "sta_config" ) && webServer->hasArg( "ssid" ) && webServer->hasArg( "key" ) ){
			if( webServer->arg( "ssid" ).length() > 0 ){
				if( !esp::saveConfig( webServer->arg( "ssid" ).c_str(), webServer->arg( "key" ).c_str(), esp::STA_MODE ) ){
					webServer->client().write( "ERROR" );
					webServer->client().stop();
				}else{
					webServer->send ( 200, "text/html", "OK" );
					esp::flags.captivePortalAccess = 1;
					ESP.restart();
				}
				return;
			}
		}
		//-------------------------------------------------------------
		esp::setNoCacheContent( webServer );

		if( pageBuff == nullptr ){
			webServer->send ( 200, "text/html", "pageBuff is nullptr" );
			return;
		}

		pageBuff[ 0 ] = '\0';
		if( pageTop != nullptr ) strcpy( pageBuff, pageTop );
		strcat( pageBuff, "<title>Wi-Fi Settings</title>" );
		if( pageEndTop != nullptr ) strcat( pageBuff, pageEndTop );



		strcat( pageBuff, "<form action='/wifi' method='post'>" );
			strcat( pageBuff, "<input type='hidden' name='sta_config' value='1'>" );
			strcat( pageBuff, "<table style=\"width: 300px; margin: auto;\">" );
				strcat( pageBuff, "<tr>" );
					strcat( pageBuff, "<td>SSID:</td>" );
					strcat( pageBuff, "<td>" );
						strcat( pageBuff, "<select name='ssid'>" );
							for( uint8_t i = 0; i < esp::countNetworks; i++ ){
								strcat( pageBuff, "<option value=\"" );
								strcat( pageBuff, WiFi.SSID( i ).c_str() );
								strcat( pageBuff, "\">" );
								strcat( pageBuff, WiFi.SSID( i ).c_str() );
								strcat( pageBuff, "</option>" );
							}
						strcat( pageBuff, "</select>" );
					strcat( pageBuff, "</td>" );
				strcat( pageBuff, "</tr>" );
				strcat( pageBuff, "<tr>" );
					strcat( pageBuff, "<td>KEY:</td>" );
					strcat( pageBuff, "<td>" );
						strcat( pageBuff, "<input name='key'>" );
					strcat( pageBuff, "</td>" );
				strcat( pageBuff, "</tr>" );
				strcat( pageBuff, "<tr>" );
					strcat( pageBuff, "<td colspan='2' align='center'><input type='submit' value='Save & connect'></td>" );
				strcat( pageBuff, "</tr>" );
			strcat( pageBuff, "</table>" );
		strcat( pageBuff, "</form>" );

		

		if( pageBottom != nullptr ) strcat( pageBuff, pageBottom );

		webServer->send ( 200, "text/html", pageBuff );
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void handleWeb404Page(ESP8266WebServer *webServer)
#elif defined(ARDUINO_ARCH_ESP32)
	void handleWeb404Page(WebServer *webServer)
#endif
	{
		if( esp::flags.captivePortalAccess ){
			esp::setWebRedirect( webServer );
			return;
		}

		esp::pageBuff[ 0 ] = '\0';
		if( esp::pageTop != nullptr ) strcpy( esp::pageBuff, esp::pageTop );
		strcat( esp::pageBuff, "<title>Not found</title>" );
		if( esp::pageEndTop != nullptr ) strcat( esp::pageBuff, esp::pageEndTop );
		strcat( esp::pageBuff, "<h1>404 Not found</h1>" );
		strcat( esp::pageBuff, webServer->header( "Location" ).c_str() );
		strcat( esp::pageBuff, "<br>" );
		strcat( esp::pageBuff, webServer->uri().c_str() );
		if( esp::pageBottom != nullptr ) strcat( esp::pageBuff, esp::pageBottom );
		webServer->send ( 200, "text/html", esp::pageBuff );
	}

	//-------------------------------------------------------------------------------
	#if defined(ARDUINO_ARCH_ESP8266)
	uint8_t webSendFile(ESP8266WebServer *webServer, char* fileName, char* mimeType)
#elif defined(ARDUINO_ARCH_ESP32)
	uint8_t webSendFile(WebServer *webServer, char* fileName, char* mimeType)
#endif
	{
#if defined(ARDUINO_ARCH_ESP8266)
		if( LittleFS.exists( fileName ) ){	
			File f = LittleFS.open( fileName, "r");
#elif defined(ARDUINO_ARCH_ESP32)
		if( SPIFFS.exists( fileName ) ){	
			File f = SPIFFS.open( fileName, "r");
#endif
			webServer->setContentLength(f.size());
			webServer->send(200, mimeType, "");
			webServer->client().write(f);
			f.close();
			webServer->client().stop();
		}else{
			webServer->send(404, "text/html", "File not found :(");
			return 0;
		}

		return 1;
	}

	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
}
