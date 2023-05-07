//------------ Includes -----------------------------------------------------------------
#include "esp_functions.h"

#if defined(ARDUINO_ARCH_ESP8266)
	#include <LittleFS.h>
	#include <ESP8266WiFi.h>
	#include <ESP8266HTTPClient.h>
	#include <WiFiClient.h>
#elif defined(ARDUINO_ARCH_ESP32)
	#include <SPIFFS.h>
	#include <HTTPClient.h>
	#include <Update.h>
	#include <esp_wifi.h>
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
	char tmpVal[ 11 ];
	char systemLogin[ 16 ];
	char systemPassword[ 16 ];
	char updateKey[ 32 ];
	uint8_t firstVersion;
	uint8_t secondVersion;
	uint16_t thridVersion;
	File updateFile;

	//-------------------------------------------------------------------------------
	uint8_t readAPconfig(char *ssid, char *key)
	{
		uint8_t result = 0;
		uint8_t ssidLen = 0;
		uint8_t keyLen = 0;

		if( esp::isFileExists( ESP_AP_CONFIG_FILE ) ){
#if defined(ARDUINO_ARCH_ESP8266)
			File f = LittleFS.open( ESP_AP_CONFIG_FILE, "r");
#elif defined(ARDUINO_ARCH_ESP32)
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
		if( !esp::flags.useFS ) return result;
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
		return esp::isFileExists( ESP_STA_CONFIG_FILE ) ? 1 : 0;
	}

	//-------------------------------------------------------------------------------
	uint8_t readSTAconfig(char *ssid, char *key)
	{
		uint8_t result = 0;
		uint8_t ssidLen = 0;
		uint8_t keyLen = 0;

		if( esp::isFileExists( ESP_STA_CONFIG_FILE ) ){
#if defined(ARDUINO_ARCH_ESP8266)
			File f = LittleFS.open( ESP_STA_CONFIG_FILE, "r");
#elif defined(ARDUINO_ARCH_ESP32)
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
	void removeFile(const char* file)
	{
		if( !esp::flags.useFS ) return;
		if( esp::isFileExists( file ) ){
#if defined(ARDUINO_ARCH_ESP8266)
			LittleFS.remove( file );
#elif defined(ARDUINO_ARCH_ESP32)
			SPIFFS.remove( file );
#endif
		}
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void setWebRedirect(ESP8266WebServer *webServer, const String &target)
#elif defined(ARDUINO_ARCH_ESP32)
	void setWebRedirect(WebServer *webServer, const String &target)
#endif
	{
		String location = "http://" + target;
		ESP_DEBUG( "WEB Redirect to [%s]\n", location.c_str() );

		if( target == "" ) return;

		// String location = "http://" + webServer->client().localIP().toString();
		webServer->sendHeader( "Location", location, true );
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

		WiFi.disconnect();
		WiFi.hostname( hostname );
		WiFi.mode( WiFiMode_t::WIFI_AP );
		WiFi.persistent( false );
		
		bool res = WiFi.softAP( hostname, "1234567890" );
#if defined(ARDUINO_ARCH_ESP8266)
	
#elif defined(ARDUINO_ARCH_ESP32)
		WiFi.softAPConfig( ip, gateway, mask );
#endif

		delay( 600 );

		esp::flags.ap_mode = 1;

		ESP_DEBUG( "wifi_AP_init IP: %s SSID: %s HOSTNAME: %s\n", WiFi.softAPIP().toString().c_str(), WiFi.softAPSSID().c_str(), hostname );

		return res;
	}

	//-------------------------------------------------------------------------------
	bool wifi_STA_init(const char* hostname)
	{
		ESP_DEBUG( "STA MODE INIT...\n" );

		char ssid[ ESP_CONFIG_SSID_MAX_LEN ];
		char skey[ ESP_CONFIG_KEY_MAX_LEN ];

		WiFi.softAPdisconnect( true );
		WiFi.mode( WiFiMode_t::WIFI_STA );
		WiFi.setAutoReconnect( true );
		WiFi.setAutoConnect( false );
		WiFi.persistent( false );

		if( esp::readSTAconfig( ssid, skey ) ){
			ESP_DEBUG( "Connect to %s\n", ssid );
			WiFi.begin( ssid, skey );
		}else{
			esp::removeFile( ESP_STA_CONFIG_FILE );
			ESP.restart();
			return false;
		}

		WiFi.hostname( hostname );

		ESP_DEBUG( "WiFi connecting...\n" );
		uint8_t i = 0;
		while( !WiFi.isConnected() && i++ < 50 ){
			delay( 100 );
			ESP_DEBUG( "." );
		}

		esp::flags.ap_mode = 0;
		
		if( !esp::isWiFiConnection() ){
			// ESP.restart();
			ESP_DEBUG( "ERROR\n" );
			return false;
		}
		
		ESP_DEBUG( "\n" );

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
	void addWebServerPages(ESP8266WebServer *webServer, bool wifiConfig, bool notFound, bool captivePortal, ESP8266WebServer::THandlerFunction cp_handler, const String &captiveRedirectTarget)
#elif defined(ARDUINO_ARCH_ESP32)
	void addWebServerPages(WebServer *webServer, bool wifiConfig, bool notFound, bool captivePortal, WebServer::THandlerFunction cp_handler, const String &captiveRedirectTarget)
#endif
	{
		if( wifiConfig ){
			webServer->on( "/wifi", [ webServer ](void){
				ESP_DEBUG( "WEB GET /wifi\n" );
				esp::handleWebConfigPage( webServer );
			} );
		}
		if( notFound ){
			webServer->onNotFound( [ webServer, captiveRedirectTarget ](void){
				String uri = webServer->uri();
				ESP_DEBUG( "WEB 404 [%s]\n", uri.c_str() );

				if( esp::flags.captivePortal && !esp::flags.captivePortalAccess ){
					String target = ( captiveRedirectTarget.charAt( 0 ) == '/' ) ? WiFi.softAPSSID() + ".lan" + captiveRedirectTarget : captiveRedirectTarget;
					target.toLowerCase();
					esp::setWebRedirect( webServer, target );
					return;
				}
				esp::handleWeb404Page( webServer );
			} );
		}

		webServer->on( "/sysinfo", [ webServer ](void){
			if( pageBuff == nullptr ){
				webServer->send ( 200, "text/html", "pageBuff is nullptr" );
				return;
			}


			strcpy( esp::pageBuff, "{ \"cpu_freq\": " );
			utoa( ESP.getCpuFreqMHz(), esp::tmpVal, 10 ); strcat( esp::pageBuff, esp::tmpVal );

			if( esp::flags.useFS ){
				strcat( esp::pageBuff, ",\"fs_total\": " );
#if defined(ARDUINO_ARCH_ESP8266)
				FSInfo64 info;
				bool resInfo = LittleFS.info64( info );
				if( resInfo ){
					itoa( info.totalBytes, esp::tmpVal, 10 );
				}else{
					itoa( -1, esp::tmpVal, 10 );
				}
				strcat( esp::pageBuff, esp::tmpVal );
#elif defined(ARDUINO_ARCH_ESP32)
				utoa( SPIFFS.totalBytes(), esp::tmpVal, 10 ); strcat( esp::pageBuff, esp::tmpVal );
#endif
				strcat( esp::pageBuff, ",\"fs_used\": " );
#if defined(ARDUINO_ARCH_ESP8266)
				if( resInfo ){
					itoa( info.usedBytes, esp::tmpVal, 10 );
				}else{
					itoa( -1, esp::tmpVal, 10 );
				}
				strcat( esp::pageBuff, esp::tmpVal );
#elif defined(ARDUINO_ARCH_ESP32)
				utoa( SPIFFS.usedBytes(), esp::tmpVal, 10 ); strcat( esp::pageBuff, esp::tmpVal );
#endif
			}

			strcat( esp::pageBuff, ",\"version\": [" );
			itoa( esp::firstVersion, esp::tmpVal, 10 );strcat( esp::pageBuff, esp::tmpVal );
			strcat( esp::pageBuff, "," );itoa( esp::secondVersion, esp::tmpVal, 10 );strcat( esp::pageBuff, esp::tmpVal );
			strcat( esp::pageBuff, "," );itoa( esp::thridVersion, esp::tmpVal, 10 );strcat( esp::pageBuff, esp::tmpVal );
			strcat( esp::pageBuff, "]" );

			strcat( esp::pageBuff, "}" );

			webServer->send ( 200, "application/json", esp::pageBuff );
		} );

		webServer->on( "/favicon.ico", [ webServer, captivePortal, cp_handler ](void){
			if( esp::flags.ap_mode && captivePortal ){
				captivePortalPage( webServer, cp_handler );
			}else{
				esp::webSendFile( webServer, "/favicon.ico", "image/x-icon" );
			}
		} );
		
		webServer->on( "/index.css", [ webServer ](void){
			esp::webSendFile( webServer, "/index.css", "text/css" );
		} );
		webServer->on( "/index.js", [ webServer ](void){
			esp::webSendFile( webServer, "/index.js", "text/javascript" );
		} );

		webServer->on( "/format", [ webServer ](void){
			if( esp::checkWebAuth( webServer, esp::systemLogin, esp::systemPassword, ESP_AUTH_REALM, "access denied" ) ){
#if defined(ARDUINO_ARCH_ESP8266)
				bool res = LittleFS.format();
#elif defined(ARDUINO_ARCH_ESP32)
				bool res = SPIFFS.format();
#endif
				if( res ){
					webServer->send ( 200, "application/json", "{ \"result\": \"OK\" }" );
				}else{
					webServer->send ( 500, "application/json", "{ \"result\": \"ERROR\" }" );
				}
			}else{
				webServer->send ( 403, "application/json", "{ \"result\": \"access denied\" }" );
			}
		} );

		if( captivePortal ){
			webServer->on( "/fwlink", [ webServer, cp_handler ](void){
				captivePortalPage( webServer, cp_handler );
			} );
			webServer->on( "/generate_204", [ webServer, cp_handler ](void){
				ESP_DEBUG( "WEB GET [404]: %s\n", webServer->header( "Location" ).c_str() );
				captivePortalPage( webServer, cp_handler );
			} );

			esp::flags.captivePortal = 1;
		}
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void addWebUpdate(ESP8266WebServer *webServer, const char* key)
#elif defined(ARDUINO_ARCH_ESP32)
	void addWebUpdate(WebServer *webServer, const char* key)
#endif
	{
		strcpy( esp::updateKey, key );
		webServer->on( "/update", HTTP_POST, [ webServer ](void){
			webServer->sendHeader( "Connection", "close" );
    		webServer->send( 200, "text/plain", ( Update.hasError() ) ? "FAIL" : "OK" );
			if( !esp::flags.updateError && esp::flags.updateFirmware ){
				delay( 1000 );
				ESP.restart();
			}
		}, [ webServer ](void){
			// webServer->sendHeader( "Access-Control-Allow-Origin", "*" );
			// if( esp::checkWebAuth( webServer, esp::systemLogin, esp::systemPassword, ESP_AUTH_REALM, "access denied" ) ){
				esp::updateProcess( webServer );
			// }else{
				// webServer->send( 403, "text/plain", "Access denied" );
			// }
		} );
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
				if( webServer->hasArg( "format" ) ){
					if( webServer->arg( "format" ) == "on" ){
						ESP_DEBUG( "Format Filesystem...\n" );
#if defined(ARDUINO_ARCH_ESP8266)
						LittleFS.format();
#elif defined(ARDUINO_ARCH_ESP32)
						SPIFFS.format();
#endif
					}
				}
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
						strcat( pageBuff, "<input type='text' name='key'>" );
					strcat( pageBuff, "</td>" );
				strcat( pageBuff, "</tr>" );
				strcat( pageBuff, "<tr>" );
					strcat( pageBuff, "<td>FORMAT FS:</td>" );
					strcat( pageBuff, "<td>" );
						strcat( pageBuff, "<input type='checkbox' name='format'>" );
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
		if( !esp::webSendFile( webServer, "/404.html", "text/html", 0 ) && !esp::webSendFile( webServer, "/index.html", "text/html", 0 ) ){
			if( pageBuff == nullptr ){
				webServer->send ( 200, "text/html", "pageBuff is nullptr" );
				return;
			}
			esp::pageBuff[ 0 ] = '\0';
			if( esp::pageTop != nullptr ) strcpy( esp::pageBuff, esp::pageTop );
			strcat( esp::pageBuff, "<title>Not found</title>" );
			if( esp::pageEndTop != nullptr ) strcat( esp::pageBuff, esp::pageEndTop );
			strcat( esp::pageBuff, "<h1>404 Not found</h1>" );
			strcat( esp::pageBuff, webServer->uri().c_str() );
			if( esp::pageBottom != nullptr ) strcat( esp::pageBuff, esp::pageBottom );
			webServer->send ( 200, "text/html", esp::pageBuff );
		}
	}

	//-------------------------------------------------------------------------------
	#if defined(ARDUINO_ARCH_ESP8266)
	uint8_t webSendFile(ESP8266WebServer *webServer, const char* fileName, const char* mimeType, const uint16_t code)
#elif defined(ARDUINO_ARCH_ESP32)
	uint8_t webSendFile(WebServer *webServer, const char* fileName, const char* mimeType, const uint16_t code)
#endif
	{
		ESP_DEBUG( "ESPF: Http send file [%s] %s\n", fileName, mimeType );
		if( esp::isFileExists( fileName ) ){
#if defined(ARDUINO_ARCH_ESP8266)
			File f = LittleFS.open( fileName, "r");
#elif defined(ARDUINO_ARCH_ESP32)
			File f = SPIFFS.open( fileName, "r");
#endif
			if( f ){
#if defined(ARDUINO_ARCH_ESP8266)
				webServer->send( code, mimeType, f, f.size() );
#elif defined(ARDUINO_ARCH_ESP32)
				webServer->streamFile( f, mimeType, code );
#endif
				f.close();
			}else{
				webServer->send( 500, "text/html", "File not open :(" );
			}
			// webServer->setContentLength(f.size());
			// webServer->send( code, mimeType, );
			// while( f.available() ){
				// int8_t len = f.read( (uint8_t*)esp::tmpVal, sizeof( esp::tmpVal ) );
				// if( len ) webServer->client().write( esp::tmpVal, len );
			// }
			// f.close();
			// webServer->client().stop();
		}else{
			if( code ) webServer->send( ( code == 200 ) ? 404 : code, "text/html", "File not found :(");
			return 0;
		}

		return 1;
	}

	//-------------------------------------------------------------------------------
	uint32_t checkingUpdate(const char *repoURL, const uint16_t version)
	{
		uint32_t res = 0;
		if( !esp::flags.useFS ) return res;
		HTTPClient http;
#if defined(ARDUINO_ARCH_ESP8266)
		WiFiClient client;
		http.begin( client, String( repoURL ) + String( ESP_FIRMWARE_VERSION_FILENAME ) );
#elif defined(ARDUINO_ARCH_ESP32)
		http.begin( String( repoURL ) + String( ESP_FIRMWARE_VERSION_FILENAME ) );
#endif
		int httpCode = http.GET();
		
		if( httpCode == HTTP_CODE_OK ){
			res = http.getString().toInt();
		}

		http.end();

		return res;
	}

	//-------------------------------------------------------------------------------
	uint8_t downloadUpdate(const char *repoURL, const char *file)
	{
		uint8_t res = 0;
		if( !esp::flags.useFS ) return res;
		HTTPClient http;
#if defined(ARDUINO_ARCH_ESP8266)
		WiFiClient client;
		http.begin( client, String( repoURL ) + String( file ) );
#elif defined(ARDUINO_ARCH_ESP32)
		http.begin( String( repoURL ) + String( file ) );
#endif
		int httpCode = http.GET();
		if( httpCode == HTTP_CODE_OK ){		
#if defined(ARDUINO_ARCH_ESP8266)
			File f = LittleFS.open( file, "w");
#elif defined(ARDUINO_ARCH_ESP32)
			File f = SPIFFS.open( file, "w");
#endif
			if( f ){
				ESP_DEBUG( "%s:%d[HTTP] Downloading [%s%s]...\n", __FILE__, __LINE__, repoURL, file );
				WiFiClient* stream = http.getStreamPtr();
				uint8_t buff[ 128 ] = { 0 };
				int totalLength = http.getSize();
				int len = totalLength;
				while( http.connected() && ( len > 0 || len == -1 ) ){
					size_t size = stream->available();
					if( size ){
						int c = stream->readBytes( buff, ((size > sizeof(buff)) ? sizeof(buff) : size) );
						f.write( buff, c );
						if( len > 0 ) len -= c;
					}
					delay(1);
				}

				// http.writeToStream( &f );
				res = 1;
			}else{
				ESP_DEBUG( "%s:%d failed to open %s\n", __FILE__, __LINE__, file );
			}
		}else{
			ESP_DEBUG( "%s:%d[HTTP] GET... failed, error: %s\n", __FILE__, __LINE__, http.errorToString( httpCode ).c_str() );
		}
		http.end();
		return res;
	}

	//-------------------------------------------------------------------------------
	uint8_t updateFromFS(void)
	{
		uint8_t res = 0;
		if( !esp::flags.useFS ) return res;

		if( esp::isFileExists( ESP_FIRMWARE_FILEPATH ) ){
#if defined(ARDUINO_ARCH_ESP8266)
			File f = LittleFS.open( ESP_FIRMWARE_FILEPATH, "r");
#elif defined(ARDUINO_ARCH_ESP32)
			File f = SPIFFS.open( ESP_FIRMWARE_FILEPATH, "r");
#endif
			if( f ){
				if( f.isDirectory() ){
					ESP_DEBUG( "%s:%d Error, %s is not a file\n", __FILE__, __LINE__, ESP_FIRMWARE_FILEPATH );
					f.close();
					return res;
				}

				size_t fileSize = f.size();
				if( fileSize > 0 ){
					ESP_DEBUG( "%s:%d Trying to start update\n", __FILE__, __LINE__ );
					if( Update.begin( fileSize ) ){
						size_t written = Update.writeStream( f );
						if( written == fileSize ){
							ESP_DEBUG( "%s:%d Written : %d successfully\n", __FILE__, __LINE__, written );
						}else{
							ESP_DEBUG( "%s:%d Written only: %d / %d. Retry?\n", __FILE__, __LINE__, written, fileSize );
						}

						if( Update.end() ){
							ESP_DEBUG( "%s:%d OTA done!?\n", __FILE__, __LINE__ );
							if( !Update.isFinished() ){
								ESP_DEBUG( "%s:%d Update not finished? Something went wrong!\n", __FILE__, __LINE__ );
							}else{
								res = 1;
							}
						}else{
							ESP_DEBUG( "%s:%d Error Occurred. Error #: %d\n", __FILE__, __LINE__, Update.getError() );
						}
					}else{
							ESP_DEBUG( "%s:%d Not enough space to begin OTA\n", __FILE__, __LINE__ );
					}
				}else{
					ESP_DEBUG( "%s:%d Error, file is empty\n", __FILE__, __LINE__ );
				}
				f.close();
			}
		}else{
			ESP_DEBUG( "%s:%d Could not load %s from spiffs root\n", __FILE__, __LINE__, ESP_FIRMWARE_FILEPATH );
		}

		return res;
	}

	//-------------------------------------------------------------------------------
	bool isFileExists(const char *filepath)
	{
		if( !esp::flags.useFS ) return false;
#if defined(ARDUINO_ARCH_ESP8266)
		return LittleFS.exists( filepath );
#elif defined(ARDUINO_ARCH_ESP32)
		return SPIFFS.exists( filepath );
#endif
	}

	//-------------------------------------------------------------------------------
	void printAllFiles(HardwareSerial &SerialPort)
	{
		if( !esp::flags.useFS ) return;
#if defined(ARDUINO_ARCH_ESP8266)
		Dir root = LittleFS.openDir( "/" );
		while( root.next() ){
			File file = root.openFile("r");
			SerialPort.print( ": " );
			SerialPort.println( root.fileName() );
			file.close();
		}
#elif defined(ARDUINO_ARCH_ESP32)
		File root = SPIFFS.open( "/" );
		File file = root.openNextFile();
		while( file ){
			SerialPort.print( ": " );
			SerialPort.println( file.name() );
			file = root.openNextFile();
		}
#endif
	}

	//-------------------------------------------------------------------------------
	void init(bool useFS)
	{
		esp::flags.useFS							= 0;

		if( useFS ){
			bool fs_init_res = false;
#if defined(ARDUINO_ARCH_ESP8266)
			fs_init_res = LittleFS.begin();
#elif defined(ARDUINO_ARCH_ESP32)
			fs_init_res = SPIFFS.begin( true );
#endif
			delay( 500 );
			ESP_DEBUG( "FS Init...%s\n", ( ( fs_init_res ) ? "OK" : "ERROR" ) );
			esp::flags.useFS						= ( fs_init_res ) ? 1 : 0;
		}
		
		esp::flags.ap_mode							= 0;
		esp::flags.captivePortal					= 0;
		esp::flags.captivePortalAccess				= 0;
		esp::flags.autoUpdate						= 0;

		if( esp::isFileExists( ESP_AUTOUPDATE_FILENAME ) ) esp::flags.autoUpdate = 1;

		pageTop = "<!DOCTYPE HTML><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><meta charset=\"utf-8\"/><script type=\"text/javascript\" src=\"/index.js\"></script><link rel=\"stylesheet\" type=\"text/css\" href=\"/index.css\"/>";
		pageEndTop = "</head><body><hr size=\"1\">";
		pageBottom = "<hr size=\"1\" class=\"red\"></body></html>";

		strcpy( esp::systemLogin, SYSTEM_LOGIN );
		strcpy( esp::systemPassword, SYSTEM_PASSWORD );
	}

	//-------------------------------------------------------------------------------
	void changeMAC(const uint8_t *mac)
	{
		ESP_DEBUG( "OLD ESP MAC: %s\n", WiFi.macAddress().c_str() );
#if defined(ARDUINO_ARCH_ESP8266)
		wifi_set_macaddr( 0, const_cast<uint8*>(mac) );
#elif defined(ARDUINO_ARCH_ESP32)
		esp_wifi_set_mac( WIFI_IF_STA, mac );
#endif
		ESP_DEBUG( "NEW ESP MAC: %s\n", WiFi.macAddress().c_str() );
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void captivePortalPage(ESP8266WebServer *webServer, ESP8266WebServer::THandlerFunction cp_handler)
#elif defined(ARDUINO_ARCH_ESP32)
	void captivePortalPage(WebServer *webServer, WebServer::THandlerFunction cp_handler)
#endif
	{
		//------------------------------------------------------------------------
		if( webServer->hasArg( "getAccess" ) ) esp::flags.captivePortalAccess = 1;
		//------------------------------------------------------------------------
		if( pageBuff == nullptr ){
			webServer->send ( ( !esp::flags.captivePortalAccess ) ? 200 : 204, "text/html", "pageBuff is nullptr" );
			return;
		}

		if( esp::pageTop != nullptr ) strcpy( esp::pageBuff, esp::pageTop );
		strcat( esp::pageBuff, "<title>Captive portal</title>" );
		if( esp::pageEndTop != nullptr ) strcat( esp::pageBuff, esp::pageEndTop );
		strcat( esp::pageBuff, "<h1>ESP Captive portal</h1>" );
		strcat( esp::pageBuff, "<br>" );
		strcat( esp::pageBuff, webServer->header( "Location" ).c_str() );
		strcat( esp::pageBuff, "<br>" );
		strcat( esp::pageBuff, webServer->uri().c_str() );
		strcat( esp::pageBuff, "<br>" );
		if( cp_handler != nullptr ) cp_handler();
		if( esp::pageBottom != nullptr ) strcat( esp::pageBuff, esp::pageBottom );

		webServer->send ( ( !esp::flags.captivePortalAccess ) ? 200 : 204, "text/html", esp::pageBuff );
	}

	//-------------------------------------------------------------------------------
	int http_put(const String &url, const String &playload, String &response)
	{
		HTTPClient http;
#if defined(ARDUINO_ARCH_ESP8266)
		WiFiClient client;
		http.begin( client, url );
#elif defined(ARDUINO_ARCH_ESP32)
		http.begin( url );
#endif
		String request = "{ \"user\": \"DrSmyrke\", \"message\": \"IR Reciever starting...\", \"key\": \"0f7d848c56094bf722ca34701d38938e\" }";
		int httpCode = http.PUT( playload );
		
		// if( httpCode == HTTP_CODE_OK ){
			response = http.getString();
		// }

		http.end();

		return httpCode;
	}

	//-------------------------------------------------------------------------------
	void saveSettings(const uint8_t* data, uint8_t length, const char* settingsFile)
	{
		if( length <= 0 || data == nullptr || settingsFile == nullptr || !esp::flags.useFS ) return;
#if defined(ARDUINO_ARCH_ESP8266)
		File f = LittleFS.open( settingsFile, "w");
#elif defined(ARDUINO_ARCH_ESP32)
		File f = SPIFFS.open( settingsFile, "w");
#endif
		if( f ){
			f.write( data, length );
			f.close();
		}
	}

	//-------------------------------------------------------------------------------
	uint8_t loadSettings(uint8_t* data, size_t size, const char* settingsFile)
	{
		uint8_t res = 0;
		if( size <= 0 || data == nullptr || settingsFile == nullptr || !esp::flags.useFS ) return res;

		if( esp::isFileExists( settingsFile ) ){
#if defined(ARDUINO_ARCH_ESP8266)
			File f = LittleFS.open( settingsFile, "r");
#elif defined(ARDUINO_ARCH_ESP32)
			File f = SPIFFS.open( settingsFile, "r");
#endif
			if( f ){
				res = f.read( data, size );
				f.close();
			}
		}
		return res;
	}
	//-------------------------------------------------------------------------------
	void changeSystemUserPassword(const char* login, const char* password)
	{
		if( login == nullptr || password == nullptr ) return;
		if( strlen( login ) > sizeof( esp::systemLogin ) ) return;
		if( strlen( password ) > sizeof( esp::systemPassword ) ) return;

		strcpy( esp::systemLogin, login );
		strcpy( esp::systemPassword, password );
	}

	//-------------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP8266)
	void updateProcess(ESP8266WebServer *webServer)
#elif defined(ARDUINO_ARCH_ESP32)
	void updateProcess(WebServer *webServer)
#endif
	{
		HTTPUpload& upload = webServer->upload();
		if( upload.status == UPLOAD_FILE_START ){
			esp::flags.updateError = 0;
			esp::flags.updateFirmware = 0;
			esp::flags.updateFile = 0;
			ESP_DEBUG( "Update: %s [ %s ]\n", upload.filename.c_str(), upload.name.c_str() );

			if( webServer->hasArg( "sdf" ) ){
				ESP_DEBUG( "[%s/%s]\n", webServer->arg( "sdf" ).c_str(), esp::updateKey );
				if( strcmp( webServer->arg( "sdf" ).c_str(), esp::updateKey ) == 0 ){
					if( upload.name == "firmware" && upload.filename == ESP_FIRMWARE_FILENAME ){
						ESP_DEBUG( "Update begin\n" );
						esp::flags.updateFirmware = 1;

						if( !Update.begin( UPDATE_SIZE_UNKNOWN ) ){ //start with max available size
							Update.printError( Serial );
							webServer->send ( 500, "text/html", "update begin fs error" );
							esp::flags.updateError = 1;
							esp::flags.updateFirmware = 0;
						}
					}else if( upload.name == "file" ){
						String path = "/" + upload.filename;
#if defined(ARDUINO_ARCH_ESP8266)
						updateFile = LittleFS.open( path, "w" );
#elif defined(ARDUINO_ARCH_ESP32)
						updateFile = SPIFFS.open( path, "w" );
#endif
						if( updateFile ){
							esp::flags.updateFile = 1;
						}else{
							esp::flags.updateError = 1;
						}
					}else{
						ESP_DEBUG( "Unknown update\n" );
						webServer->send ( 500, "text/html", "Unknown update: error" );
						esp::flags.updateError = 1;
					}
				}else{
					ESP_DEBUG( "Unknown key\n" );
					webServer->send ( 500, "text/html", "Update: Unknown key :(" );
					esp::flags.updateError = 1;
				}
			}else{
				ESP_DEBUG( "Unknown key\n" );
				webServer->send ( 500, "text/html", "Update: Unknown key :(" );
				esp::flags.updateError = 1;
			}
		}else if( upload.status == UPLOAD_FILE_WRITE ){
			// flashing firmware to ESP
			if( !esp::flags.updateError && esp::flags.updateFirmware ){
				if( Update.write( upload.buf, upload.currentSize ) != upload.currentSize ){
					Update.printError( Serial );
					webServer->send ( 500, "text/html", "update error" );
					esp::flags.updateError = 1;
				}
			}else if( !esp::flags.updateError && esp::flags.updateFile ){
				if( updateFile ){
					updateFile.write( upload.buf, upload.currentSize );
				}else{
					esp::flags.updateError = 1;
				}
			}
		}else if( upload.status == UPLOAD_FILE_END ){
			// finish flashing firmware to ESP
			if( !esp::flags.updateError && esp::flags.updateFirmware ){
				if( Update.end( true ) ){ //true to set the size to the current progress
					ESP_DEBUG( "Update Success: %u\nRebooting...\n", upload.totalSize );
				}else{
					Update.printError( Serial );
					webServer->send ( 500, "text/html", "update error 2" );
					esp::flags.updateError = 1;
				}
			}else if( !esp::flags.updateError && esp::flags.updateFile ){
				if( updateFile ){
					// if( upload.currentSize ) updateFile.write( upload.buf, upload.currentSize );
					updateFile.close();
				}
			}
		}else if( upload.status == UPLOAD_FILE_ABORTED ){
			Update.end();
			ESP_DEBUG( "Update was aborted\n" );
			webServer->send ( 500, "text/html", "update aborted" );
			esp::flags.updateError = 1;
		}

		
		/*
		HTTPUpload& upload = webServer->upload();

		ESP_DEBUG( "updateProcess [%s][%s] %d %d/%d\n", upload.name, upload.filename, upload.status, upload.currentSize, upload.totalSize );

		if( upload.status == UPLOAD_FILE_START ){
			esp::flags.updateError = 0;
			if( upload.name == "filesystem" ){
#if defined(ARDUINO_ARCH_ESP8266)
				FSInfo64 info;
				bool resInfo = LittleFS.info64( info );
				size_t fsSize = info.totalBytes - info.usedBytes;
				LittleFS.end();
				//start with max available size
				if( !Update.begin( fsSize, U_FS ) ){
#elif defined(ARDUINO_ARCH_ESP32)
				size_t fsSize = SPIFFS.totalBytes() - SPIFFS.usedBytes();
				SPIFFS.end();
				//start with max available size
				if( !Update.begin( fsSize, U_SPIFFS ) ){
#endif
					webServer->send ( 500, "text/html", "update begin fs error" );
					esp::flags.updateError = 1;
				}
			}else if( upload.name == "firmware" && upload.filename == ESP_FIRMWARE_FILENAME ){
				ESP_DEBUG( "Update begin\n" );
				uint32_t maxSketchSpace = ( ESP.getFreeSketchSpace() - 0x1000 ) & 0xFFFFF000;
				//start with max available size
				if( !Update.begin( maxSketchSpace, U_FLASH ) ){
					webServer->send ( 500, "text/html", "update begin firmware error" );
					esp::flags.updateError = 1;
				}
			}
		}else if( upload.status == UPLOAD_FILE_WRITE && !esp::flags.updateError ){
			ESP_DEBUG( "." );
			if( Update.write( upload.buf, upload.currentSize ) != upload.currentSize ){
				webServer->send ( 500, "text/html", "update error" );
			}
		}else if( upload.status == UPLOAD_FILE_END && !esp::flags.updateError ){
			//true to set the size to the current progress
			if( Update.end( true ) ){
				ESP_DEBUG( "Update Success: %zu\nRebooting...\n", upload.totalSize );
			} else {
				webServer->send ( 500, "text/html", "update error" );
			}
		}else if( upload.status == UPLOAD_FILE_ABORTED ){
			Update.end();
			ESP_DEBUG( "Update was aborted\n" );
			webServer->send ( 500, "text/html", "update aborted" );
		}
		*/
	}

	//-------------------------------------------------------------------------------
	void setVersion(const uint8_t first, const uint8_t second, const uint16_t thrid)
	{
		esp::firstVersion = first;
		esp::secondVersion = second;
		esp::thridVersion = thrid;
	}

	//-------------------------------------------------------------------------------
}
