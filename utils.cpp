
#include "utils.h"


String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}


// Forcibly mount the SPIFFS. Formatting the SPIFFS if needed.
//
// Returns:
//   A boolean indicating success or failure.
bool mountSpiffs(void)
{
    //Serial.println("Mounting SPIFFS...");
    if (SPIFFS.begin()){
        //Serial.println("SPIFFS mounted successfully.");
        return true; // We mounted it okay.
    }
    // We failed the first time.
    Serial.println("Failed to mount SPIFFS!\nFormatting SPIFFS and trying again...");
    SPIFFS.format();
    if (!SPIFFS.begin()){ // Did we fail?
        Serial.println("DANGER: Failed to mount SPIFFS even after formatting!");
        delay(1000); // Make sure the debug message doesn't just float by.
        return false;
    }
    return true; // Success!
}

/**
 * Returns contents of the file or empty string on fail.
 */
String fileGetContents(const char * filename)
{
    String contents = "";
    if (mountSpiffs()) {
        if (SPIFFS.exists(filename)) {
            File theFile = SPIFFS.open(filename, "r");
            if (theFile) {
                size_t size = theFile.size();
                // Allocate a buffer to store contents of the file.
                char buf[size+1];
                
                int readedLen = theFile.readBytes(buf, size);
                buf[readedLen] = 0;
                contents = contents + buf;
                theFile.close();
            }
        } else {
            Serial.println(String("File '") + filename + "' doesn't exist!");
        }
        SPIFFS.end();
    }
    return contents;
}


bool loadConfig(const char * filename, std::function<void(DynamicJsonDocument)> onLoadCallback)
{
    String text = fileGetContents(filename);
    Serial.println(text);
    Serial.println("xxx");

    if(text == "") 
        return false;

    const int JSON_SIZE = 2048;

    DynamicJsonDocument json(JSON_SIZE);
    DeserializationError error = deserializeJson(json, text);
    if (!error) {
        // run onLoadCallback
        onLoadCallback(json);
        return true;
    } else {
        Serial.println("Failed to load json config");
        return false;
    }    

}



bool saveConfig(const char * filename, DynamicJsonDocument json) 
{
    bool success = false;

    if (mountSpiffs()) {
        File configFile = SPIFFS.open(filename, "w");
        if (!configFile) {
            Serial.println("Failed to open config file for writing.");
        } else {
            serializeJson(json, configFile);
            configFile.close();
            success = true;
        }
    }
    return success;
}

bool saveConfig(const char * filename, String jsonStr) 
{
    bool success = false;

    if (mountSpiffs()) {
        File configFile = SPIFFS.open(filename, "w");
        if (!configFile) {
            Serial.println("Failed to open config file for writing.");
        } else {
            configFile.write(jsonStr.c_str());
            configFile.close();
            success = true;
        }
        SPIFFS.end();
    }
    return success;
}

