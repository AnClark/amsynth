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
    static ImGuiWindowFlags flagsMainWindow = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    // Common buffers
    char buttonLabel[20]; // Buffer for creating unique button labels

    ImGui::Begin("Amsynth Main Window", (bool *)true, flagsMainWindow);

    // Section 01: OSC1
    {
        ImGui::BeginGroup();
        ImGui::Text("OSC 1");

        _AmsynthControl_Oscillator1Waveform(); // Waveform Switch
        ImGui::SameLine();

        _AmsynthControl_Oscillator1Pulsewidth(); // Shape (Pulse Width)

        ImGui::EndGroup();
    }

    // Section 01.5: Oscilloscope
    ImGui::SameLine();
    _AmsynthControl_Oscilloscope();

    // Section 02: OSC2
    {
        ImGui::BeginGroup();
        ImGui::Text("OSC 2");

        // Waveform Switch
        _AmsynthControl_Oscillator2Waveform();
        ImGui::SameLine();

        // Shape (Pulse Width)
        _AmsynthControl_Oscillator2Pulsewidth();
        ImGui::SameLine();

        // "Sync to OSC1" switch
        _AmsynthControl_Oscillator2Sync();

        ImGui::EndGroup();
    }

    // Section 03: Tune for OSC 2
    {
        ImGui::BeginGroup();
        ImGui::Text("Tune for OSC 2");

        // Octave
        _AmsynthControl_Oscillator2Octave();
        ImGui::SameLine();

        // Semitone
        _AmsynthControl_Oscillator2Pitch();
        ImGui::SameLine();

        // Detune
        _AmsynthControl_Oscillator2Detune();

        ImGui::EndGroup();
    }

    // Section 04: AMP Envelope
    {
        ImGui::BeginGroup();
        ImGui::Text("AMP Envelope");

        _AmsynthControl_AmpEnvAttack();
        ImGui::SameLine();

        _AmsynthControl_AmpEnvDecay();
        ImGui::SameLine();

        _AmsynthControl_AmpEnvSustain();
        ImGui::SameLine();

        _AmsynthControl_AmpEnvRelease();
        ImGui::EndGroup();
    }

    ImGui::SameLine();

    // Section 05: OSC Mix
    {
        ImGui::BeginGroup();
        ImGui::Text("OSC Mix");

        _AmsynthControl_OscillatorMix();
        ImGui::SameLine();

        _AmsynthControl_OscillatorMixRingMod();

        ImGui::EndGroup();
    }

    ImGui::SameLine();

    // Section 06: AMP Volume / Drive
    {
        ImGui::BeginGroup();
        ImGui::Text("AMP");

        _AmsynthControl_MasterVolume();
        ImGui::SameLine();

        _AmsynthControl_AmpDistortion();

        ImGui::EndGroup();
    }

    // Section: LFO
    {
        ImGui::BeginGroup();
        ImGui::Text("LFO");
        ImGui::SameLine(0, 90);

        // LFO Waveform Switch
        _AmsynthControl_LFOWaveform();
        ImGui::SameLine(0, 40);

        // LFO OSC selector
        _AmsynthControl_LFOOscillatorSelect();

        // Frequency
        _AmsynthControl_LFOFreq();
        ImGui::SameLine();

        // Freq Mod Amount
        _AmsynthControl_LFOToOscillators();
        ImGui::SameLine();

        _AmsynthControl_LFOToFilterCutoff();
        ImGui::SameLine();

        _AmsynthControl_LFOToAmp();

        ImGui::EndGroup();
    }

    // Section: Reverb
    {
        ImGui::BeginGroup();
        ImGui::Text("Reverb");

        _AmsynthControl_ReverbWet();
        ImGui::SameLine();

        _AmsynthControl_ReverbRoomsize();
        ImGui::SameLine();

        _AmsynthControl_ReverbDamp();
        ImGui::SameLine();

        _AmsynthControl_ReverbWidth();
        ImGui::EndGroup();
    }

    ImGui::SameLine();

    // Section: Filter
    {
        ImGui::BeginGroup();
        ImGui::Text("Filter");
        ImGui::SameLine(0, 90);

        // ------ Filter option selectors ------
        _AmsynthControl_FilterType();
        ImGui::SameLine(0, 40);

        _AmsynthControl_FilterSlope();

        // ------ Filter basic options ------
        _AmsynthControl_FilterResonance();
        ImGui::SameLine();

        _AmsynthControl_FilterCutoff();
        ImGui::SameLine();

        _AmsynthControl_FilterKeyTrackAmount();
        ImGui::SameLine();

        _AmsynthControl_FilterEnvAmount();

        // ------ Filter ADSR ------

        _AmsynthControl_FilterEnvAttack();
        ImGui::SameLine();

        _AmsynthControl_FilterEnvDecay();
        ImGui::SameLine();

        _AmsynthControl_FilterEnvSustain();
        ImGui::SameLine();

        _AmsynthControl_FilterEnvRelease();
        ImGui::SameLine();

        ImGui::EndGroup();
    }

    // Section: Keyboard Options
    {
        ImGui::BeginGroup();
        ImGui::Text("Keyboard Options");

        _AmsynthControl_PortamentoTime();
        ImGui::SameLine();

        _AmsynthControl_PortamentoMode();
        ImGui::SameLine();

        _AmsynthControl_KeyboardMode();
        ImGui::EndGroup();
    }

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
