#ifndef MIDI_BUFFER_H
#define MIDI_BUFFER_H

#include <Arduino.h>


//MIDI STATUSES
#define MIDI_NOTE_OFF 8
#define MIDI_NOTE_ON 9
#define MIDI_POLY_AFTERTOUCH 10
#define MIDI_CONTROL_CHANGE 11
#define MIDI_PROG_CHANGE 12
#define MIDI_CHANNEL_AFTERTOUCH 13
#define MIDI_PITCH_BEND 14
#define MIDI_SYSTEM_EXCLUSIVE 15

#ifndef MIDI_MESSAGE_STRUCT
#define MIDI_MESSAGE_STRUCT
struct MIDIMessage{
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
};
#endif


class MidiBuffer {
public:
  MidiBuffer(size_t size);
  ~MidiBuffer();

  bool push(const MIDIMessage &message);
  bool isEmpty() const;
  MIDIMessage getMsg();

private:
  size_t bufferSize;
  MIDIMessage *buffer;
  size_t head;
  size_t tail;
  size_t count;

  bool isFull() const;
};

#endif // MIDI_BUFFER_H