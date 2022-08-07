#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"

const char * ssid = "OpenWrt";
const char * password = "andrew01";


String FirmwareVer = {
  "2.1"
};
#define URL_fw_Version "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/bin_version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/fw.bin"

//#define URL_fw_Version "http://cade-make.000webhostapp.com/version.txt"
//#define URL_fw_Bin "http://cade-make.000webhostapp.com/firmware.bin"

void connect_wifi();
void firmwareUpdate();
int FirmwareVersionCheck();

unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long previousMillis_2 = 0;
const long interval = 60000;
const long mini_interval = 1000;
void repeatedCall() {
  static int num=0;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (FirmwareVersionCheck()) {
      firmwareUpdate();
    }
  }
  if ((currentMillis - previousMillis_2) >= mini_interval) {
    previousMillis_2 = currentMillis;
    Serial.print("idle loop...");
    Serial.print(num++);
    Serial.print(" Active fw version:");
    Serial.println(FirmwareVer);
   if(WiFi.status() == WL_CONNECTED) 
   {
       Serial.println("wifi connected");
   }
   else
   {
    connect_wifi();
   }
  }
}

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button_boot = {
  0,
  0,
  false
};
/*void IRAM_ATTR isr(void* arg) {
    Button* s = static_cast<Button*>(arg);
    s->numberKeyPresses += 1;
    s->pressed = true;
}*/

void IRAM_ATTR isr() {
  button_boot.numberKeyPresses += 1;
  button_boot.pressed = true;
}


// ------------------------------------------------------------------------------------------

// If you're using the full breakout...
Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();
// If you're using the FeatherWing version
//Adafruit_IS31FL3731_Wing ledmatrix = Adafruit_IS31FL3731_Wing();


// The lookup table to make the brightness changes be more visible
uint8_t sweep[] = {1, 2, 3, 4, 6, 8, 10, 15, 20, 30, 40, 60, 60, 40, 30, 20, 15, 10, 8, 6, 4, 3, 2, 1};

int smile[7][11] ={{0,0,0,0,0,0,0,0,0,0,0},
                   {0,0,1,1,0,0,0,1,1,0,0},
                   {0,0,1,1,0,0,0,1,1,0,0},
                   {0,0,0,0,0,0,0,0,0,0,0},
                   {0,0,1,0,0,0,0,0,1,0,0},
                   {0,0,0,1,1,1,1,1,0,0,0},
                   {0,0,0,0,0,0,0,0,0,0,0}
                  };

int sad[7][11] =  {{0,0,0,0,0,0,0,0,0,0,0},
                   {0,0,1,1,0,0,0,1,1,0,0},
                   {0,0,1,1,0,0,0,1,1,0,0},
                   {0,0,0,0,0,0,0,0,0,0,0},
                   {0,0,0,1,1,1,1,1,0,0,0},
                   {0,0,1,0,0,0,0,0,1,0,0},
                   {0,0,0,0,0,0,0,0,0,0,0}
                  };

int logo[7][11] = {{0,0,0,0,0,0,0,0,0,0,0},
                   {0,0,0,1,1,0,0,1,1,2,0},
                   {0,0,0,1,1,0,2,1,1,0,0},
                   {0,0,0,0,0,0,1,1,2,0,0},
                   {0,0,0,0,0,2,1,1,0,0,0},
                   {0,0,0,0,0,1,1,2,0,0,0},
                   {0,0,0,0,2,1,1,0,0,0,0}
                  };

unsigned long timerPeriod = 50; 
unsigned long startMillis; 
unsigned long currentMillis;
int brightness = 0;
bool pulse = false;

void aliasBitmap(int width, int height, int brightness, int data[7][11])
{
    int x;
    int y;
    int b;
    int x1;
    int y1;

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
         //   b = brightness * data[y][x];              
            if (data[y][x] == 1)
                b = 200;
            else if (data[y][x] == 2)
                b = 64;
            else
                b = 0;
                
            if (x > 5)
            {
                x1 = x - 6;
                y1 = y + 1;
            }
            else
            {
                x1 = x;
                y1 = y + 9;
            }
            ledmatrix.drawPixel(x1, y1, b);
        }
    }
}


void flashBitmap(int width, int height, int brightness, int data[7][11])
{
    int x;
    int y;
    int b;
    int x1;
    int y1;

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            b = brightness * data[y][x];              
                
            if (x > 5)
            {
                x1 = x - 6;
                y1 = y + 1;
            }
            else
            {
                x1 = x;
                y1 = y + 9;
            }
            ledmatrix.drawPixel(x1, y1, b);
        }
    }
}

// ------------------------------------------------------------------------------------------


void setup() {
  pinMode(button_boot.PIN, INPUT);
  attachInterrupt(button_boot.PIN, isr, RISING);
  Serial.begin(115200);
  Serial.print("Active firmware version:");
  Serial.println(FirmwareVer);
//  pinMode(LED_BUILTIN, OUTPUT);
  connect_wifi();

  aliasBitmap(11, 7, brightness, logo);
  
}
void loop() {
  if (button_boot.pressed) { //to connect wifi via Android esp touch app 
    Serial.println("Firmware update Starting..");
    firmwareUpdate();
    button_boot.pressed = false;
  }
  repeatedCall();
}

void connect_wifi() {
  Serial.println("Waiting for WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
//  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}
int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client) 
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }
      
  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}
