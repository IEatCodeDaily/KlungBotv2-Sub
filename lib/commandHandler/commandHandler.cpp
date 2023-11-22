#include "commandHandler.h"


void handleCommand(const String &message) {
  // Split the message into command and arguments
  int spaceIndex = message.indexOf(' ');
  String command = message.substring(0, spaceIndex);
  String argsStr = message.substring(spaceIndex + 1);
  argsStr.trim();

  const int maxArgs = MAX_ARGS;
  String args[maxArgs];
  int argsCount = 0;
  splitArgs(argsStr, args, argsCount);

  // Check if the command exists and process it

  // if (command.equalsIgnoreCase("log")) { //DEPRECATED
  //   set_log(args, argsCount);
  // } 

  // get mac function
  if (command == "getmac") {
    String macAddress = getMacAddress();
    Serial.println("MAC Address: " + macAddress);
  }
  else if (command == "peer") {
    cmdPeer(args);
  }
  else if (command == "midi") {
    cmdMidi(args);
  }
  else if (command == "note" || command == "notes"){
    cmdNote(args);
  }
  else if (command == "printconfig" || command == "printcfg") {
    printConfig();
  }
  else {
    Serial.println("Unknown command");
  }
}

void splitArgs(const String &argsStr, String args[], int &argsCount) {
  int start = 0;
  int spaceIndex = argsStr.indexOf(' ');
  while (spaceIndex != -1) {
    args[argsCount++] = argsStr.substring(start, spaceIndex);
    start = spaceIndex + 1;
    spaceIndex = argsStr.indexOf(' ', start);
  }
  args[argsCount++] = argsStr.substring(start);
}


//================NETWORK RELATED===============

//Return this device MAC address
String getMacAddress() {
  uint8_t mac[6];
  char macStr[18] = {0};

  // Get the MAC address
  WiFi.macAddress(mac);

  // Convert the MAC address to a string
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return String(macStr);
}

// check if it's a valid mac address
bool isValidMacAddress(const String &macAddr) {
  if (macAddr.length() != 17) {
    return false;
  }

  for (int i = 0; i < 17; i++) {
    char c = macAddr[i];
    if ((i % 3) == 2) {
      if (c != ':') {
        return false;
      }
    } else {
      if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
        return false;
      }
    }
  }

  return true;
}

//============MIDI Commands==============

void cmdMidi(const String args[MAX_ARGS]) {
  String subCommand = args[0];

  if (subCommand == "list") {
    listMidiPins();
  }
  else if (subCommand == "set" || subCommand == "modify") {
    uint8_t midiOutputPin = args[1].toInt();
    uint8_t gpioPin = args[2].toInt();
    modifyMidiPin(midiOutputPin, gpioPin);
  }
  else if (subCommand == "clear") {
    clearMidiPins();
  }
  else if (subCommand == "save") {
    saveMidiPins();
  }
  else if (subCommand == "load") {
    loadMidiPins();
  }
  else if (subCommand == "init"){
    initializeMidiPins();
  }
  else {
    Serial.println("Unknown MIDI command");
    Serial.println("Available commands:\nlist\nset <midiPin> <GPIO>\nmodify <midiPin> <GPIO>\nsave\nload\nclear\n");
  
  }
}

void listMidiPins() {
  Serial.println("MIDI Pin and Note list");
  for (int i = 0; i < 16; i++) {
    Serial.print("P");
    Serial.print(i);
    Serial.print(": GPIO ");
    Serial.print(midiPins[i]);
    Serial.print(" - MIDI Note: 0x");
    Serial.println(midiNotes[i], HEX);
  }
}

void modifyMidiPin(uint8_t midiOutputPin, uint8_t gpioPin) {
  midiPins[midiOutputPin] = gpioPin;
  Serial.printf("GPIO %d set for MIDI Pin %d\n", gpioPin, midiOutputPin);
}

void clearMidiPins() {
  for (int i = 0; i < 16; i++) {
    midiPins[i] = 0;
  }
}

void saveMidiPins() {
  // Load the existing configuration
  DynamicJsonDocument doc = loadConfig();
  JsonArray midiPinsJson = doc.createNestedArray("midiPins");

  // Update the midiPins array in the JSON document
  for (int i = 0; i < 16; i++) {
    midiPinsJson.add(midiPins[i]);
  }

  // Save the updated configuration
  saveConfig(doc);
}

void loadMidiPins() {
  // Load the configuration
  DynamicJsonDocument doc = loadConfig();
  JsonArray midiPinsJson = doc["midiPins"];

  // Update the midiPins array from the JSON document
  for (int i = 0; i < 16; i++) {
    midiPins[i] = midiPinsJson[i].as<uint8_t>();
  }
}

void initializeMidiPins() {
  for (int i = 0; i < 16; i++) {
    uint8_t pin = midiPins[i];
    if (pin != 0) {
      ESP_LOGD(TAG_SYSTEM,"midiPins[%d]: %d",i,pin);
      pinMode(pin, OUTPUT);
      // Set up a PWM channel for the pin
      ledcSetup(i, PWM_FREQUENCY, PWM_RESOLUTION);
      // Attach the pin to the PWM channel
      ledcAttachPin(pin, i);
      // Initially set the PWM duty cycle to 0 (off)
      ledcWrite(i, 0);
    }
  }
}

//============note commands==============

void cmdNote(const String args[MAX_ARGS]) {
  String subCommand = args[0];

  if (subCommand == "list") {
    listNotes();
  } else if (subCommand == "modify" || subCommand == "set") {
    int pin = Serial.parseInt();
    String note = Serial.readStringUntil('\n');
    modifyNote(args[1].toInt(), args[2]);
  } else if (subCommand == "clear") {
    clearNotes();
  } else if (subCommand == "save") {
    saveNotes();
  } else if (subCommand == "load") {
    loadNotes();
  } else {
    Serial.println("Invalid note command.");
  }
}

void listNotes() {
  Serial.println("MIDI Pin and Note list");
  for (int i = 0; i < 16; i++) {
    Serial.print("P");
    Serial.print(i);
    Serial.print(": GPIO ");
    Serial.print(midiPins[i]);
    Serial.print(" - MIDI Note: 0x");
    Serial.println(midiNotes[i], HEX);
  }
}

void modifyNote(int pin, String note) {
  if (pin >= 0 && pin < 16) {
    uint8_t midiNote = strtoul(note.c_str(), nullptr, 16);
    midiNotes[pin] = midiNote;
    Serial.print("Note for P");
    Serial.print(pin);
    Serial.print(" set to: 0x");
    Serial.println(midiNote, HEX);
  } else {
    Serial.println("Invalid pin number.");
  }
}

void clearNotes() {
  for (int i = 0; i < 16; i++) {
    midiNotes[i] = 0;
  }
  Serial.println("MIDI notes cleared.");
}

void saveNotes() {
  DynamicJsonDocument doc = loadConfig();
  JsonArray newNotesArray = doc.createNestedArray("midiNotes");
  
  for (int i = 0; i < 16; i++) {
    newNotesArray.add(midiNotes[i]);
  }

  if (saveConfig(doc)) {
    Serial.println("MIDI notes saved successfully.");
  } else {
    Serial.println("Failed to save MIDI notes.");
  }
}

void loadNotes() {
  DynamicJsonDocument doc = loadConfig();
  JsonArray notesArray = doc["midiNotes"];

  for (int i = 0; i < 16; i++) {
    midiNotes[i] = notesArray[i];
  }

  Serial.println("MIDI notes loaded.");
}

//============Peer Commands==============
void cmdPeer(const String args[MAX_ARGS]) {
  String subCommand = args[0];

  if (subCommand == "list") {
    if (args[1] == "-c") {
      listPeers();
    } else {
      peerManager.listPeers();
    }
  }
  else if (subCommand == "clear"){
    clearPeers();
    Serial.println("Peers cleared from Peer Manager.");
  }
  else if (subCommand == "add") {
    if (isValidMacAddress(args[2])) {
      addPeer(args[1].toInt(), args[2]);
    } else {
      Serial.println("Invalid MAC address format.");
    }
  }
  else if (subCommand == "remove") {
    if (isValidMacAddress(args[1])) {
      removePeerByMac(args[1]);
    } else {
      int id = args[1].toInt();
      removePeerById(id);
    }
  }
  else if (subCommand == "save") {
    savePeer();
    Serial.println("Peers saved to the config file.");
  }
  else if (subCommand == "load") {
    loadPeer();
    espNow_peerInit();
    Serial.println("Peers loaded from the config file.");
  }
  else {
    Serial.println("Invalid peer sub-command");
    Serial.println("Available commands:\nlist\nadd <id> <macAddr>\nremove <id/macAddr>\nsave\nload\nclear\n");
  }
}

void clearPeers(){
  peerManager.clear();
}

void listPeers() {
  DynamicJsonDocument doc = loadConfig();
  JsonArray peers = doc["peers"];
  Serial.println("Peers from config file:");
  
  if (peers.size() == 0) {
    Serial.println("No peers found.");
    return;
  }

  Serial.println("List of peers:");
  for (JsonObject peer : peers) {
    Serial.print("ID: ");
    Serial.print(peer["id"].as<int>());
    Serial.print(", MAC Addr: ");
    Serial.println(peer["macAddr"].as<String>());
  }
}

void savePeer() {
  DynamicJsonDocument doc = loadConfig();
  JsonArray peers = doc.createNestedArray("peers");
  for (int i = 0; i < peerManager.getPeerCount(); ++i) {
    JsonObject peer = peers.createNestedObject();
    Peer p = peerManager.getPeerByIndex(i);
    peer["id"] = p.id;
    peer["macAddr"] = macToString(p.macAddr);
  }
  saveConfig(doc);
}

void loadPeer(){
  DynamicJsonDocument doc = loadConfig();
  JsonArray peers = doc["peers"];

  if (peers.isNull()) {
    ESP_LOGW(TAG_NETWORK,"No peers in config file");
    return;
  }

  peerManager.clear();
  for (JsonObject peer : peers) {
    Peer p;
    p.id = peer["id"];
    String macAddr = peer["macAddr"].as<String>();
    macStringToByteArray(macAddr, p.macAddr);
    peerManager.addPeer(p);
  }

  ESP_LOGI(TAG_NETWORK,"Loaded peers from config file to Peer Manager");
}

void addPeer(int id, const String &macAddr) {
  // Check if the ID is already in peerManager
  if (peerManager.getPeerById(id) != nullptr) {
    Serial.println("ID already exists in peerManager.");
    return;
  }

  // Convert the string MAC address to a byte array
  uint8_t macByteArray[6];
  macStringToByteArray(macAddr, macByteArray);

  // Create a Peer object with the given ID and MAC address
  Peer newPeer{id, {macByteArray[0], macByteArray[1], macByteArray[2], macByteArray[3], macByteArray[4], macByteArray[5]}};

  // Add the new peer to the peerManager
  if (espNow_addPeer(macAddr)) {
    Serial.println("ESP-NOW peer added successfully.");

    // Add the peer to the ESP-NOW network
    if (peerManager.addPeer(newPeer)) {
      Serial.println("Peer added to Peer Manager successfully.");
    } else {
      Serial.println("Failed to add ESP-NOW peer.");
    }
  } 
  else {
    Serial.println("Failed to add peer to ESP-NOW");
  }
}
  

// Remove peer by MAC ADDRESS
void removePeerByMac(const String &macAddr) {
  int id = peerManager.getIdByMacAddress(macAddr);
  if (id != -1) {
    bool success = espNow_removePeer(macAddr);
    if (success) {
      // Remove peer from the peerManager and save the updated config
      peerManager.removePeer(id);
    }
  } else {
    Serial.println("Peer not found.");
  }
}

// Remove peer by ID
void removePeerById(int id) {
  String macAddr = peerManager.getMacAddressById(id);
  if (macAddr != "") {
    bool success = espNow_removePeer(macAddr);
    if (success) {
      // Remove peer from the peerManager and save the updated config
      peerManager.removePeer(id);
    }
  } else {
    Serial.println("Peer not found.");
  }
}

void removePeer(const String &macOrID) { //DEPRECATED
  DynamicJsonDocument doc = loadConfig();
  JsonArray peers = doc["peers"];

  for (int i = 0; i < peers.size(); i++) {
    JsonObject peer = peers[i];
    if (peer["id"].as<String>() == macOrID || peer["macAddr"].as<String>() == macOrID) {
      peers.remove(i);
      saveConfig(doc);
      Serial.println("Peer removed successfully.");
      return;
    }
  }

  Serial.println("Peer not found.");
}


//============SET LOG LEVEL=============== //SCRAPPED -- DOESN'T WORK ON ARDUINO FRAMEWORK FUCKKKKK
void set_log(String args[], int argsCount) {
  if (argsCount != 2) {
    Serial.println("Invalid number of arguments for log command");
    print_tags();
    return;
  }

  String tag = args[0];
  String logLevel = args[1];

  if (!isValidTag(tag)) {
    Serial.println("Invalid log tag");
    print_tags();
    return;
  }

  esp_log_level_t level;
  if (logLevel.equalsIgnoreCase("NONE")) {
    level = ESP_LOG_NONE;
  } else if (logLevel.equalsIgnoreCase("ERROR") || logLevel == "1") {
    level = ESP_LOG_ERROR;
  } else if (logLevel.equalsIgnoreCase("WARN") || logLevel == "2") {
    level = ESP_LOG_WARN;
  } else if (logLevel.equalsIgnoreCase("INFO") || logLevel == "3") {
    level = ESP_LOG_INFO;
  } else if (logLevel.equalsIgnoreCase("DEBUG") || logLevel == "4") {
    level = ESP_LOG_DEBUG;
  } else if (logLevel.equalsIgnoreCase("VERBOSE") || logLevel == "5") {
    level = ESP_LOG_VERBOSE;
  } else {
    Serial.println("Invalid log level");
    return;
  }

  esp_log_level_set(tag.c_str(), level);
  Serial.printf("Log level for tag \"%s\" set to \"%d\"\n", tag.c_str(), esp_log_level_get(tag.c_str()));
  
}


void print_tags(){
    Serial.println("Available Log Tags:");
    for (size_t i = 0; i < NUM_LOG_TAGS; i++) {
        Serial.printf("%s\n", LOG_TAGS[i]);
    }
}

bool isValidTag(const String &tag) {
  for (size_t i = 0; i < NUM_LOG_TAGS; i++) {
    if (tag.equalsIgnoreCase(LOG_TAGS[i])) {
      return true;
    }
  }
  return false;
}