#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include "params.h"

#ifndef TAG_SYSTEM
#define TAG_SYSTEM "SYSTEM"
#endif

#define CONFIG_FILE "/config.json"
#define JSON_DOC_SIZE 1024

void initConfig();
DynamicJsonDocument loadConfig();
bool saveConfig(const DynamicJsonDocument &doc);
void printConfig();

#endif // CONFIG_MANAGER_H