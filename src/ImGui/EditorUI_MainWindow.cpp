#include "EditorUI.h"

void EditorUI::amsynthWindow_Main()
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

    const float width = fUI->getWidth();
    const float height = fUI->getHeight();
    const float margin = 20.0f * fUI->getScaleFactor();

    // ImGui::SetNextWindowPos(ImVec2(margin, margin));
    // ImGui::SetNextWindowSize(ImVec2(width - 2 * margin, height - 2 * margin));
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    // Common buffers
    char buttonLabel[20]; // Buffer for creating unique button labels

    ImGui::Begin("Amsynth Main Window", (bool*)true, flagsMainWindow);

    // Section 01: OSC1
    {
        ImGui::BeginChild("OSC1", ImVec2(200, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar()) {
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
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("OSC Mix");
            ImGui::EndMenuBar();
        }

        _AmsynthControl_OscillatorMix();
        ImGui::SameLine(0, 20);

        _AmsynthControl_OscillatorMixRingMod();

        ImGui::EndChild();
    }

    // Section 01.5: Oscilloscope
#if 0 // TODO: A better way to let the DSP side export sample data to UI?
    ImGui::SameLine();
    _AmsynthControl_Oscilloscope();
#endif

    // Section 02: OSC2
    {
        ImGui::BeginChild("OSC2", ImVec2(400, 150), true, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar()) {
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
        if (ImGui::BeginMenuBar()) {
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
        if (ImGui::BeginMenuBar()) {
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
        if (ImGui::BeginMenuBar()) {
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
        if (ImGui::BeginMenuBar()) {
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
        if (ImGui::BeginMenuBar()) {
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
        if (ImGui::BeginMenuBar()) {
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
            this->showPresetWindow = !this->showPresetWindow;

        ImGui::SameLine();

#if 0 // TODO: A better way to handle randomisation or panic?
        if (ImGui::Button("Randomise", ImVec2(90, 0)))
            synthInstance->getPresetController()->randomiseCurrentPreset();

        ImGui::SameLine();

        if (ImGui::Button("Panic", ImVec2(90, 0)))
            panic();
#endif
    }

    ImGui::End();

    /** ==============================================================================
     *  Preset Window Section
     *  ==============================================================================
     *
     * TODO: Sync with host's program viewer (Memorymoon behavior)
     */
    //if (showPresetWindow)
    //    _AmsynthWindow_Preset();
}
