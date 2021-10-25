#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <SSD1306.h>
#include "utils.h"



#define APP_VERSION         "0.1"
#define DEVICE_ID           "PixelLamp-nw"
#define LED_PIN             D3 // Lamp
#define BTN_PIN             D8 // Button

#define MODE_COUNT 0

CRGB leds[256] = {0};
#include "ledeffects.h"

const char *        gConfigFile = "/config.json";
byte                gBrightness = 40;

                    // SSD1306Wire(uint8_t _address, uint8_t _sda, uint8_t _scl, OLEDDISPLAY_GEOMETRY g = GEOMETRY_128_64)    
SSD1306             display(0x3c, D2 /*RX*/, D1 /*TX*/, GEOMETRY_128_32);



void saveTheConfig()
{
    DynamicJsonDocument json(4096);
    json["bright"] = gBrightness;
    json["mode"]   = gCurrentMode;

    JsonObject modesC = json.createNestedObject("modes");

    for(int i = 0; i < MODE_COUNT; i++){
        JsonObject item = modesC.createNestedObject(gModeStructs[i].name);
        item["scale"] = gModeConfigs[i].scale;
        item["speed"] = gModeConfigs[i].speed;
    }

    saveConfig(gConfigFile, json);
}

void loadTheConfig()
{
    loadConfig(gConfigFile, [](DynamicJsonDocument doc){
        gCurrentMode = doc["mode"]; 
        gBrightness  = doc["bright"]; 

        for(int i = 0; i < MODE_COUNT; i++){
            String key = gModeStructs[i].name;
            if(doc["modes"].containsKey(key)){
                gModeConfigs[i].scale = doc["modes"][key]["scale"];
                gModeConfigs[i].speed = doc["modes"][key]["speed"];

                if(!gModeConfigs[i].scale)
                    gModeConfigs[i].scale = 30;
                if(!gModeConfigs[i].speed)
                    gModeConfigs[i].speed = 30;
            }
        }

    });
}


void setup() 
{

    Serial.begin(74880/*, SERIAL_8N1, SERIAL_TX_ONLY*/);

    // Init display
    display.init();
    display.setContrast(1, 5, 0);
    display.flipScreenVertically();
    display.drawString(0, 0, "pixelLamp");
    display.display();


    // config loading
    loadTheConfig();


    // ЛЕНТА
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, 256);//.setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(gBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 5000);
    FastLED.clear();

    leds[0] = CRGB::Blue;
    leds[1] = CRGB::Magenta;
    leds[2] = CRGB::Red;
    leds[3] = CRGB::Green;
    leds[4] = CRGB::Blue;


    FastLED.show();
    delay(50);
    FastLED.show();


    Serial.println("*** end setup ***");
    
}

void printCurrentParams()
{
    char buf[150];
    sprintf(buf, "Mode: %2d. %-11s Scale: %3d  Speed: %3d",
        gCurrentMode,
        gModeStructs[gCurrentMode].name,
        gModeConfigs[gCurrentMode].scale,
        gModeConfigs[gCurrentMode].speed
    );
    Serial.println(buf);
}

void btnLoop()
{
    static long pressTime = 0;
    static bool lastState = false;

    bool pressed = digitalRead(BTN_PIN);

    if(pressed && !lastState && millis() - pressTime > 200){
        pressTime = millis();
        gCurrentMode ++;
        if(gCurrentMode >= MODE_COUNT)
            gCurrentMode = 0;
        printCurrentParams();
        saveTheConfig();
    }

}


void serialReadLoop()
{
    if(Serial.available() > 0){
        String str = Serial.readStringUntil('\n');
        //Serial.println(str);
        if(str.charAt(0) == 'm' && str.charAt(1) == 'o'){
            gCurrentMode = str.substring(2).toInt();
            saveTheConfig();
        }
        if(str.charAt(0) == 's' && str.charAt(1) == 'c'){
            int value = str.substring(2).toInt();
            gModeConfigs[gCurrentMode].scale = value;
            saveTheConfig();
        }
        if(str.charAt(0) == 's' && str.charAt(1) == 'p'){
            int value = str.substring(2).toInt();
            gModeConfigs[gCurrentMode].speed = value;
            saveTheConfig();
        }
        if(str.startsWith("conf")){
            int mode = gCurrentMode;
            for(int i=0; i < MODE_COUNT; i++){
                gCurrentMode = i;
                printCurrentParams();
            }
            gCurrentMode = mode;

            loadConfig(gConfigFile, [](DynamicJsonDocument doc){
                char buf[1024];
                serializeJson(doc, buf, 1024);
                Serial.println(buf);
            });
        }
        if(str.startsWith("restart")){
            ESP.restart();
        }

        printCurrentParams();
    }
}


void loop() 
{
    effectsLoop();
    btnLoop();
    serialReadLoop();

    display.clear();
    display.drawString(0, 0, String() + gCurrentMode);
    display.drawString(20, 0, gModeStructs[gCurrentMode].name);
    display.drawString(0, 10, String("scale: ") + gScale + " speed: " + gSpeed);
    display.display();
}