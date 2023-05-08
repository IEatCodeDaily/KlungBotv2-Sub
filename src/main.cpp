#include "params.h"
#include <esp_log.h>
#include <Arduino.h>
#include <freertos\FreeRTOS.h>
#include "commandHandler.h"


// Task handles
TaskHandle_t TaskHandleMIDI;
TaskHandle_t TaskSerial;



//===============RTOS TASKS=================
void Task_MIDI(void *parameter) {
  extern MidiBuffer midiBuffer;
  while (1) {
    while(!midiBuffer.isEmpty()){
      MIDIMessage midiMsg = midiBuffer.getMsg();
      uint8_t statusDiv = midiMsg.status/16;
    if (statusDiv == MIDI_NOTE_OFF || statusDiv == MIDI_NOTE_ON) { // Note on or note off
      // Find the corresponding pin in midiNotes array
      for (int i = 0; i < 16; i++) {
        if (midiNotes[i] == midiMsg.data1) {
          // Set the corresponding GPIO pin based on the status byte
          if (statusDiv == 9) { // Note on
            ledcWrite(i, 255); // 100% duty cycle
            ESP_LOGD(TAG_SYSTEM,"Set Note 0x%02x: GPIO %d ON", midiNotes[i], midiPins[i]);
          } else if (statusDiv == 8) { // Note off
            ledcWrite(i, 0); // 0% duty cycle
            ESP_LOGD(TAG_SYSTEM,"Set Note 0x%02x: GPIO %d OFF", midiNotes[i], midiPins[i]);
          }
          break;
        }
      }
    }
    }
    
    // Yield the task, allowing other tasks to run
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void Task_Serial(void *parameter) {
  String command;
  while (1) {
    if (Serial.available()) {
      command = Serial.readStringUntil('\n');
      command.trim();
      
      // Process command
      handleCommand(command);
    }
    // Yield the task, allowing other tasks to run
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true); //Allow ESP_LOG debugging
  
  // Initialize SPIFFS
  initConfig();
  
  // Initialize ESP-NOW
  espNow_init();
  loadMidiPins();
  loadNotes();
  initializeMidiPins();

  // Set Default log level - USELESS
  esp_log_level_set("*", ESP_LOG_INFO);
  
  // Create tasks
  xTaskCreatePinnedToCore(Task_MIDI, "TaskHandleMIDI", 4096, NULL, 3, &TaskHandleMIDI, 1);
  xTaskCreatePinnedToCore(Task_Serial, "TaskSerial", 4096, NULL, 1, &TaskSerial, 1);
}

void loop() {
  // Intentionally left empty, as tasks are handled by RTOS.
}