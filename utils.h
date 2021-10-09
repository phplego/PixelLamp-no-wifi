#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>


String getContentType(String filename);



// Forcibly mount the SPIFFS. Formatting the SPIFFS if needed.
//
// Returns:
//   A boolean indicating success or failure.
bool mountSpiffs(void);

/**
 * Returns contents of the file or empty string on fail.
 */
String fileGetContents(const char * filename);


bool loadConfig(const char * filename, std::function<void(DynamicJsonDocument)> onLoadCallback);
bool saveConfig(const char * filename, DynamicJsonDocument json) ;
bool saveConfig(const char * filename, String jsonStr);

