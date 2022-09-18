#pragma once

#include <imgui.h>
#include <imgui_internal.h>

/** midi.h from amsynth */
#include "midi.h"

/** Start entry of key state index */
#define KEY_INDEX_START_WHITE 21
#define KEY_INDEX_START_BLACK 22

/** Special values of key state */
#define KEY_RELEASED -2
#define KEY_SUSTAIN -1

namespace ImGui
{

typedef void (*PianoMidiEventCallback)(char midi_status, char note_id, char velocity);

class IMGUI_API Piano
{
  private:
    int keyNum;
    inline bool has_black(int key);
    PianoMidiEventCallback _onPianoKeyPress;

  public:
    Piano(int keyNum = 61);
    int keyStates[256] = {0};
    void up(int key);
    void drawKeyboard(ImVec2 size, bool enable_input = false);
    void down(int key, int velocity);
    void reset();

    void setPianoKeyEventCallback(PianoMidiEventCallback func)
    {
        this->_onPianoKeyPress = func;
    }

    char *keyboardToMidi(int key_id, int velocity, bool note_on);

    // For debug
    void drawDebugTable();
};

static void default_piano_midi_event_callback(char midi_status, char note_id, char velocity);

} // namespace ImGui
