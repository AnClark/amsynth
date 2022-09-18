/*
 *  mainwindow.cpp - Maintains the main editor panel window
 *
 *  Copyright (c) 2021 AnClark Liu
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "editor_pane.h"

void ImguiEditor::_AmsynthWindow_Main()
{
    /** ==============================================================================
     *  Main Window Section
     *  ==============================================================================
     *
     * Notes:
     * - ImGui widgets may use label name to identify each other. Keep them unique.
     *   To create widgets with same label, append "##%d" (%d means a unique integer).
     *   For example:
     *       ImGui::Button("Test Button##1");
     *       ImGui::Button("Test Button##2");
     *
     * - Do not use static variables to store current values, especially for widgets
     *   using int / enum values (DropDown, SliderInt, etc.).
     *   Otherwise, you will not be able to get value from host when setting the value
     *   on host side, or loading programs.
     *
     *   Call _onParamChange() instead. Parameter synchronization between DSP and
     *   editor will do the right things.
     */

    // Fullscreen window parameters
    static ImGuiWindowFlags flagsMainWindow = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    // Common buffers
    char buttonLabel[20]; // Buffer for creating unique button labels

    ImGui::Begin("Amsynth Main Window", (bool *)true, flagsMainWindow);

    // Piano
    if (1)
    {
        // ImGui::BeginChild("Piano", ImVec2(800, 150), true);
        // ImGui::Piano piano;   // Use the global ImGui::Piano definition in ImguiEditor. Bypass.
        piano.drawKeyboard(ImVec2(700, 150), true);

        if (ImGui::Button("Press Key #29"))
        {
            piano.down(29, 65);
        }

        if (ImGui::Button("Send Note On C4"))
        {
            unsigned char buffer[3] = {MIDI_STATUS_NOTE_ON, 0x3c, 0x7f}; // Buffer format: {status, data1, data2}
            synthInstance->getMidiController()->HandleMidiData(buffer, 3);
        }

        if (ImGui::Button("Send Note Off C4"))
        {
            unsigned char buffer[3] = {MIDI_STATUS_NOTE_OFF, 0x3c, 0}; // Buffer format: {status, data1, data2}
            synthInstance->getMidiController()->HandleMidiData(buffer, 3);
        }

        piano.drawDebugTable();
        // ImGui::EndChild();
    }

    // Piano from midiosc
    if (0)
    {
        // draw a keyboard
        static int keydown[128] = {};
        static int xpos[16] = {0}, ypos[16] = {0}, spiciness[16] = {0};
        static int topnote = 127;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        float pw = ImGui::GetWindowContentRegionWidth();
        float ph = 64.f;
        float kw = pw / 75.f;
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const ImVec2 q(p.x + pw, p.y + ph);
        draw_list->AddRectFilled(ImVec2(q.x - 64.f, p.y - 66.f), ImVec2(q.x, p.y - 2.f), 0xff808080, 2.f);
        draw_list->AddRectFilled(ImVec2(q.x - 70.f, p.y - 66.f), ImVec2(q.x - 66.f, p.y - 2.f), 0xff808080, 2.f);
        draw_list->AddRectFilled(ImVec2(q.x - 70.f, p.y - 2.f - spiciness[0] * 0.5f), ImVec2(q.x - 66.f, p.y - 2.f),
                                 0xff0000ff, 2.f);
        float xx = q.x - 64.f + xpos[0] * 0.5f, yy = p.y - 2.f - ypos[0] * 0.5f;
        draw_list->AddLine(ImVec2(xx, p.y - 66.f), ImVec2(xx, p.y - 2.f), 0x80000000, 1.f);
        draw_list->AddLine(ImVec2(q.x - 64.f, yy), ImVec2(q.x, yy), 0x80000000, 1.f);
        draw_list->AddRectFilled(p, q, 0xff000000);
        draw_list->AddCircleFilled(ImVec2(xx + 0.5f, yy + 0.5f), 2.f, 0xff0000ff, 8);
        const ImVec2 m = ImGui::GetIO().MousePos;
        int mb = ImGui::GetIO().MouseDown[0];
        static int mousekey = -1;
        if (!mb && mousekey >= 0)
        {
            // ProcessMidiEvent(0x80 + (mousekey << 8), true);
            mousekey = -1;
        }
        int keytab[2][7] = {{0, 2, 4, 5, 7, 9, 11}, {1, 3, -10000, 6, 8, 10, -10000}};
        for (int blackwhite = 0; blackwhite < 2; ++blackwhite)
        {
            for (int i = 0; i < 75; ++i)
            {
                int midinote = keytab[blackwhite][i % 7] + 12 * (i / 7);
                if (midinote < 0 || midinote > 127)
                    continue;

                ImU32 col = 0xffeeeeee;
                if (midinote > topnote)
                    col = 0x80eeeeee;
                if (blackwhite)
                    col ^= 0xdddddd;
                if (midinote == topnote)
                    col = 0xffffee00;
                ImVec2 a, b;
                float blacky = p.y + ph * 0.6f;
                if (blackwhite)
                    a = ImVec2(p.x + kw * i + kw * 0.66f, p.y), b = ImVec2(p.x + kw * i + kw * 1.33f, blacky);
                else
                    a = ImVec2(p.x + kw * i, p.y), b = ImVec2(p.x + kw * i + kw - 1.f, q.y);
                if (m.x >= a.x && m.x <= b.x && m.y >= a.y && m.y <= b.y && (m.y < blacky) == blackwhite)
                {
                    if (mb)
                    {
                        if (mousekey >= 0 && mousekey != midinote)
                        {
                            // ProcessMidiEvent(0x80 + (mousekey << 8), true);
                            mousekey = -1;
                        }
                        if (mousekey != midinote)
                        {
                            mousekey = midinote;
                            // ProcessMidiEvent(0x90 + (mousekey << 8) + (127 << 16), true);
                        }
                    }
                    col ^= 0x4040;
                }
                if (keydown[midinote])
                    col ^= 0x8080;
                draw_list->AddRectFilled(a, b, col, 2.f);
            }
        }
        ImGui::Dummy(ImVec2(pw, ph));
    }

    // Section 01: OSC1
    {
        ImGui::BeginChild("OSC1", ImVec2(200, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("OSC 1");
            ImGui::EndMenuBar();
        }

        // Placeholder
        ImGui::Text("");
        ImGui::SameLine(0, 5);

        _AmsynthControl_Oscillator1Waveform(); // Waveform Switch
        ImGui::SameLine(0, 20);

        _AmsynthControl_Oscillator1Pulsewidth(); // Shape (Pulse Width)

        ImGui::EndChild();
    }

    ImGui::SameLine();
    // TODO: Put OSC Mix together with AMP? -> "OSC/Master Mix"
    // Section 05: OSC Mix
    {
        ImGui::BeginChild("OSC Mix", ImVec2(200, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("OSC Mix");
            ImGui::EndMenuBar();
        }

        _AmsynthControl_OscillatorMix();
        ImGui::SameLine(0, 20);

        _AmsynthControl_OscillatorMixRingMod();

        ImGui::EndChild();
    }

    // Section 01.5: Oscilloscope
    ImGui::SameLine();
    _AmsynthControl_Oscilloscope();

    // Section 02: OSC2
    {
        ImGui::BeginChild("OSC2", ImVec2(400, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("OSC 2");

            // "Sync to OSC1" switch
            _AmsynthControl_Oscillator2Sync();
            ImGui::EndMenuBar();
        }

        // Placeholder
        ImGui::Text("");
        ImGui::SameLine(0, 5);

        // Waveform Switch
        _AmsynthControl_Oscillator2Waveform();
        ImGui::SameLine(0, 20);

        // Shape (Pulse Width)
        _AmsynthControl_Oscillator2Pulsewidth();
        ImGui::SameLine(0, 20);

        // Octave
        _AmsynthControl_Oscillator2Octave();
        ImGui::SameLine(0, 20);

        // Semitone
        _AmsynthControl_Oscillator2Pitch();
        ImGui::SameLine(0, 20);

        // Detune
        _AmsynthControl_Oscillator2Detune();

        ImGui::EndChild();
    }

    // Section 04: AMP Envelope
    {
        ImGui::BeginChild("AMP_Envelope", ImVec2(400, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("AMP Envelope");
            ImGui::EndMenuBar();
        }

        _AmsynthControl_AmpEnvAttack();
        ImGui::SameLine(0, 20);

        _AmsynthControl_AmpEnvDecay();
        ImGui::SameLine(0, 20);

        _AmsynthControl_AmpEnvSustain();
        ImGui::SameLine(0, 20);

        _AmsynthControl_AmpEnvRelease();
        ImGui::EndChild();
    }

    // ImGui::SameLine();

    ImGui::SameLine();

    // Section 06: AMP Volume / Drive
    {
        ImGui::BeginChild("AMP", ImVec2(200, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("AMP");
            ImGui::EndMenuBar();
        }

        _AmsynthControl_MasterVolume();
        ImGui::SameLine();

        _AmsynthControl_AmpDistortion();

        ImGui::EndChild();
    }

    // Section: LFO
    {
        ImGui::BeginChild("LFO", ImVec2(600, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("LFO");
            ImGui::SameLine(0, 100);

            // LFO OSC selector
            ImGui::Text("Apply to → ");
            _AmsynthControl_LFOOscillatorSelect();

            ImGui::EndMenuBar();
        }

        // LFO Waveform Switch
        _AmsynthControl_LFOWaveform();
        ImGui::SameLine(0, 40);

        // Frequency
        _AmsynthControl_LFOFreq();
        ImGui::SameLine(0, 40);

        // Freq Mod Amount
        _AmsynthControl_LFOToOscillators();
        ImGui::SameLine(0, 40);

        _AmsynthControl_LFOToFilterCutoff();
        ImGui::SameLine(0, 40);

        _AmsynthControl_LFOToAmp();

        ImGui::EndChild();
    }

    // Section: Reverb
    {
        ImGui::BeginChild("Reverb", ImVec2(400, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("Reverb");
            ImGui::EndMenuBar();
        }

        _AmsynthControl_ReverbWet();
        ImGui::SameLine(0, 40);

        _AmsynthControl_ReverbRoomsize();
        ImGui::SameLine(0, 40);

        _AmsynthControl_ReverbDamp();
        ImGui::SameLine(0, 40);

        _AmsynthControl_ReverbWidth();

        ImGui::EndChild();
    }

    // ImGui::SameLine();

    // Section: Filter
    {
        ImGui::BeginChild("Filter", ImVec2(600, 300), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("Filter");

            _AmsynthControl_FilterSlope();

            ImGui::EndMenuBar();
        }

        // ------ Filter option selectors ------
        _AmsynthControl_FilterType();
        ImGui::SameLine(0, 40);

        // ------ Filter basic options ------
        _AmsynthControl_FilterResonance();
        ImGui::SameLine(0, 40);

        _AmsynthControl_FilterCutoff();
        ImGui::SameLine(0, 40);

        _AmsynthControl_FilterKeyTrackAmount();
        ImGui::SameLine(0, 40);

        _AmsynthControl_FilterEnvAmount();

        // ------ Filter ADSR ------

        _AmsynthControl_FilterEnvAttack();
        ImGui::SameLine(0, 40);

        _AmsynthControl_FilterEnvDecay();
        ImGui::SameLine(0, 40);

        _AmsynthControl_FilterEnvSustain();
        ImGui::SameLine(0, 40);

        _AmsynthControl_FilterEnvRelease();

        ImGui::EndChild();
    }

    // Section: Keyboard Options
    {
        ImGui::BeginChild("Keyboard Options", ImVec2(700, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("Keyboard Options");
            ImGui::EndMenuBar();
        }

        _AmsynthControl_PortamentoTime();
        ImGui::SameLine(0, 40);

        _AmsynthControl_PortamentoMode();
        ImGui::SameLine(0, 40);

        _AmsynthControl_KeyboardMode();
        ImGui::SameLine(0, 40);

        // -------- Velocity strategy - How to process velocity --------
        _AmsynthControl_FilterKeyVelocityAmount();
        ImGui::SameLine(0, 40);

        _AmsynthControl_AmpVelocityAmount();

        ImGui::EndChild();
    }

#if 0
    ImGui::SameLine();

    // Section: Velocity strategy - How to process velocity
    {
        ImGui::BeginGroup();
        ImGui::Text("Velocity Strategy");

        _AmsynthControl_FilterKeyVelocityAmount();
        ImGui::SameLine();

        _AmsynthControl_AmpVelocityAmount();
        ImGui::EndGroup();
    }
#endif

    // Section: Option buttons
    // TODO: Must move to elsewhere properly
    {
        if (ImGui::Button("Presets", ImVec2(90, 0)))
            ImguiEditor::showPresetWindow = !ImguiEditor::showPresetWindow;

        ImGui::SameLine();

        if (ImGui::Button("Randomise", ImVec2(90, 0)))
            synthInstance->getPresetController()->randomiseCurrentPreset();

        ImGui::SameLine();

        if (ImGui::Button("Panic", ImVec2(90, 0)))
            panic();
    }

    ImGui::End();

    /** ==============================================================================
     *  Preset Window Section
     *  ==============================================================================
     *
     * TODO: Sync with host's program viewer (Memorymoon behavior)
     */
    if (showPresetWindow)
        _AmsynthWindow_Preset();
}
