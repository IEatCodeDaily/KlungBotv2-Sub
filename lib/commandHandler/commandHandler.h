#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "ESPNowHandler.hpp"
#include "params.h"

#define MAX_ARGS 5

#ifndef PEER_MANAGER
#define PEER_MANAGER
PeerManager peerManager;
#endif
extern PeerManager peerManager;
extern uint8_t midiPins[16];
extern uint8_t midiNotes[16];


// Command handler
void handleCommand(const String &message);
void splitArgs(const String &argsStr, String args[], int &argsCount);

//Get MAC
String getMacAddress();
bool isValidMacAddress(const String &macAddr);

//Midi Command
const uint32_t PWM_FREQUENCY = 5000; // 5 kHz
const uint8_t PWM_RESOLUTION = 8; // 8-bit resolution, values from 0 to 255

void cmdMidi(const String args[MAX_ARGS]);
void listMidiPins();
void modifyMidiPin(uint8_t midiPinIndex, uint8_t gpioPin);
void clearMidiPins();
void saveMidiPins();
void loadMidiPins();
void initializeMidiPins();

//Note Command
void cmdNote(const String args[MAX_ARGS]);
void listNotes();
void modifyNote(int pin, String note);
void clearNotes();
void saveNotes();
void loadNotes();

// Peers Command
void cmdPeer(const String args[MAX_ARGS]);

void listPeers();
void clearPeers();

void savePeer();
void loadPeer();

void addPeer(int id, const String &macAddr);

void removePeerByMac(const String &macAddr);
void removePeerById(int id);
void removePeer(const String &macOrID);


// Log Command
void set_log(String args[], int argsCount);
void print_tags();
bool isValidTag(const String &tag);

#endif