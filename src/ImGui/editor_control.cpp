#include "editor_pane.h"

#define KNOB_PARAMS(index) &paramList[index], paramMinValues[index], paramMaxValues[index]

void ImguiEditor::_AmsynthControl_Oscilloscope()
{
    ImGui::Oscilloscope("##Oscilloscope", currentSample, numCurrentSample, 180, ImVec2(0, 80.0f));
}

void ImguiEditor::_AmsynthControl_Oscillator1Waveform()
{
    const unsigned char OSC1_WAVEFORM_COUNT = 5;
    static const char *osc1_waveformOptions[OSC1_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample (Hold)"};
    unsigned char osc1_waveformSelected = (int)paramList[kAmsynthParameter_Oscillator1Waveform];
    if (ImGui::ComboButton("OSC1 Waveform", osc1_waveformSelected, osc1_waveformOptions, OSC1_WAVEFORM_COUNT, ImVec2(70, 0), "OSC1 Waveform"))
    {
        paramList[kAmsynthParameter_Oscillator1Waveform] = (float)osc1_waveformSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_Oscillator1Pulsewidth()
{
    if (ImGui::Knob("Shape", KNOB_PARAMS(kAmsynthParameter_Oscillator1Pulsewidth), ImVec2(40, 40), "Shape"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_Oscillator2Waveform()
{
    const unsigned char OSC2_WAVEFORM_COUNT = 5;
    static const char *osc2_waveformOptions[OSC2_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample (Hold)"};
    unsigned char osc2_waveformSelected = (int)paramList[kAmsynthParameter_Oscillator2Waveform];

    if (ImGui::ComboButton("OSC2 Waveform", osc2_waveformSelected, osc2_waveformOptions, OSC2_WAVEFORM_COUNT, ImVec2(70, 0), "OSC2 Waveform"))
    {
        paramList[kAmsynthParameter_Oscillator2Waveform] = (float)osc2_waveformSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_Oscillator2Pulsewidth()
{
    if (ImGui::Knob("Shape 2", KNOB_PARAMS(kAmsynthParameter_Oscillator2Pulsewidth), ImVec2(40, 40), "Shape"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_Oscillator2Sync()
{
    bool osc2_syncWithOSC1Toggle = paramList[kAmsynthParameter_Oscillator2Sync] >= 1.0f ? true : false;
    if (ImGui::Checkbox("Sync to OSC1", &osc2_syncWithOSC1Toggle))
    {
        paramList[kAmsynthParameter_Oscillator2Sync] = osc2_syncWithOSC1Toggle ? 1.0f : 0.0f;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_Oscillator2Octave()
{
    int osc2_octaveValue = (int)paramList[kAmsynthParameter_Oscillator2Octave]; // NOTICE: Also avoid using static var
    if (ImGui::SliderInt("OSC2 Octave", &osc2_octaveValue, -3, 4))
    {
        paramList[kAmsynthParameter_Oscillator2Octave] = (float)osc2_octaveValue;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_Oscillator2Detune()
{
    if (ImGui::Knob("Detune", &paramList[kAmsynthParameter_Oscillator2Detune], paramMinValues[kAmsynthParameter_Oscillator2Detune], paramMaxValues[kAmsynthParameter_Oscillator2Detune], ImVec2(90, 40), "Detune"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_Oscillator2Pitch()
{
    // TODO: Create ImGui::KnobInt
    if (ImGui::Knob("Pitch", &paramList[kAmsynthParameter_Oscillator2Pitch], paramMinValues[kAmsynthParameter_Oscillator2Pitch], paramMaxValues[kAmsynthParameter_Oscillator2Pitch], ImVec2(90, 40), "Semitone"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpEnvAttack()
{
    if (ImGui::Knob("Attack", KNOB_PARAMS(kAmsynthParameter_AmpEnvAttack), ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpEnvDecay()
{
    if (ImGui::Knob("Decay", KNOB_PARAMS(kAmsynthParameter_AmpEnvDecay), ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpEnvSustain()
{
    if (ImGui::Knob("Sustain", KNOB_PARAMS(kAmsynthParameter_AmpEnvSustain), ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpEnvRelease()
{
    if (ImGui::Knob("Release", KNOB_PARAMS(kAmsynthParameter_AmpEnvRelease), ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpDistortion()
{
    if (ImGui::Knob("Distortion", &paramList[kAmsynthParameter_AmpDistortion], paramMinValues[kAmsynthParameter_AmpDistortion], paramMaxValues[kAmsynthParameter_AmpDistortion], ImVec2(70, 40), "Distortion"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_MasterVolume()
{
    if (ImGui::Knob("Master Volume", &paramList[kAmsynthParameter_MasterVolume], paramMinValues[kAmsynthParameter_MasterVolume], paramMaxValues[kAmsynthParameter_MasterVolume], ImVec2(90, 40), "Master Volume"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_OscillatorMix()
{
    if (ImGui::Knob("OSC Balance", &paramList[kAmsynthParameter_OscillatorMix], paramMinValues[kAmsynthParameter_OscillatorMix], paramMaxValues[kAmsynthParameter_OscillatorMix], ImVec2(70, 40), "OSC Mix"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_OscillatorMixRingMod()
{
    if (ImGui::Knob("Ring Mod", &paramList[kAmsynthParameter_OscillatorMixRingMod], paramMinValues[kAmsynthParameter_OscillatorMixRingMod], paramMaxValues[kAmsynthParameter_OscillatorMixRingMod], ImVec2(80, 40), "Ring Mod"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOWaveform()
{
    const unsigned char LFO_WAVEFORM_COUNT = 7;
    static const char *lfo_waveformOptions[LFO_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample & Hold", "Sawtooth (up)", "Sawtooth (down)"};
    static const char *lfo_waveformIcon[LFO_WAVEFORM_COUNT]; // TODO: Add icon resource
    unsigned char lfo_waveformSelected = (int)paramList[kAmsynthParameter_LFOWaveform];

    if (ImGui::ComboButton("LFO Waveform", lfo_waveformSelected, lfo_waveformOptions, LFO_WAVEFORM_COUNT, ImVec2(70, 0), "LFO Waveform"))
    {
        paramList[kAmsynthParameter_LFOWaveform] = (float)lfo_waveformSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_LFOOscillatorSelect()
{
    const int LFO_OSC_SELECTION_COUNT = 3;
    unsigned char lfo_oscSelected = (int)paramList[kAmsynthParameter_LFOOscillatorSelect];
    const char *lfo_oscSelectorOptions[LFO_OSC_SELECTION_COUNT] = {"OSC 1+2", "OSC 1", "OSC2"};

    if (ImGui::ComboButton("OSC Selector", lfo_oscSelected, lfo_oscSelectorOptions, LFO_OSC_SELECTION_COUNT, ImVec2(70, 0), "OSC Selector"))
    {
        paramList[kAmsynthParameter_LFOOscillatorSelect] = (float)lfo_oscSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_LFOFreq()
{
    if (ImGui::Knob("Speed", &paramList[kAmsynthParameter_LFOFreq], paramMinValues[kAmsynthParameter_LFOFreq], paramMaxValues[kAmsynthParameter_LFOFreq], ImVec2(80, 40), "LFO Frequency"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOToOscillators()
{
    if (ImGui::Knob("Mod Amount", &paramList[kAmsynthParameter_LFOToOscillators], paramMinValues[kAmsynthParameter_LFOToOscillators], paramMaxValues[kAmsynthParameter_LFOToOscillators], ImVec2(90, 40), "LFO Mod Amount"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOToFilterCutoff()
{
    if (ImGui::Knob("To Filter", &paramList[kAmsynthParameter_LFOToFilterCutoff], paramMinValues[kAmsynthParameter_LFOToFilterCutoff], paramMaxValues[kAmsynthParameter_LFOToFilterCutoff], ImVec2(80, 40), "LFO to Filter"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOToAmp()
{
    if (ImGui::Knob("To Amp", &paramList[kAmsynthParameter_LFOToAmp], paramMinValues[kAmsynthParameter_LFOToAmp], paramMaxValues[kAmsynthParameter_LFOToAmp], ImVec2(80, 40), "LFO to Amp"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbWet()
{
    if (ImGui::Knob("Amount", &paramList[kAmsynthParameter_ReverbWet], paramMinValues[kAmsynthParameter_ReverbWet], paramMaxValues[kAmsynthParameter_ReverbWet], ImVec2(80, 40), "Reverb Amount"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbRoomsize()
{
    if (ImGui::Knob("Size", &paramList[kAmsynthParameter_ReverbRoomsize], paramMinValues[kAmsynthParameter_ReverbRoomsize], paramMaxValues[kAmsynthParameter_ReverbRoomsize], ImVec2(80, 40), "Room Size"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbDamp()
{
    if (ImGui::Knob("Damp", &paramList[kAmsynthParameter_ReverbDamp], paramMinValues[kAmsynthParameter_ReverbDamp], paramMaxValues[kAmsynthParameter_ReverbDamp], ImVec2(80, 40), "Damp"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbWidth()
{
    if (ImGui::Knob("Width", &paramList[kAmsynthParameter_ReverbWidth], paramMinValues[kAmsynthParameter_ReverbWidth], paramMaxValues[kAmsynthParameter_ReverbWidth], ImVec2(80, 40), "Reverb Width"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_PortamentoTime()
{
    if (ImGui::Knob("Portamento Time", &paramList[kAmsynthParameter_PortamentoTime], paramMinValues[kAmsynthParameter_PortamentoTime], paramMaxValues[kAmsynthParameter_PortamentoTime], ImVec2(100, 40), "Portamento Time"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_PortamentoMode()
{
    ImGui::BeginGroup();
    {
        const int PORTAMENTO_MODE_COUNT = 2;
        unsigned char portamento_modeSelected = (int)paramList[kAmsynthParameter_PortamentoMode];
        const char *portamento_modeOptions[PORTAMENTO_MODE_COUNT] = {"Always", "Legato"};

        if (ImGui::ComboButton("Portamento Mode", portamento_modeSelected, portamento_modeOptions, PORTAMENTO_MODE_COUNT, ImVec2(100, 0), nullptr))
        {
            paramList[kAmsynthParameter_PortamentoMode] = (float)portamento_modeSelected;
            _onParamChange(paramList, effInstance);
        }
    }
    ImGui::Text("Portamento Mode");
    ImGui::EndGroup();
}

void ImguiEditor::_AmsynthControl_KeyboardMode()
{
    ImGui::BeginGroup();
    {
        const int KEYBOARD_MODE_COUNT = 3;
        unsigned char keyboard_modeSelected = (int)paramList[kAmsynthParameter_KeyboardMode];
        const char *keyboard_modeOptions[KEYBOARD_MODE_COUNT] = {"Poly", "Mono", "Legato"};

        if (ImGui::ComboButton("Keyboard Mode", keyboard_modeSelected, keyboard_modeOptions, KEYBOARD_MODE_COUNT, ImVec2(100, 0), nullptr))
        {
            paramList[kAmsynthParameter_KeyboardMode] = (float)keyboard_modeSelected;
            _onParamChange(paramList, effInstance);
        }
    }
    ImGui::Text("Keyboard Mode");
    ImGui::EndGroup();
}

// ------ Filter option selectors ------

void ImguiEditor::_AmsynthControl_FilterType()
{
    const int FILTER_TYPE_COUNT = 5;
    static const char *filter_typeOptions[FILTER_TYPE_COUNT] = {"Low Pass", "High Pass", "Band Pass", "Notch", "Bypass"};
    unsigned char filter_typeSelected = (int)paramList[kAmsynthParameter_FilterType];

    if (ImGui::ComboButton("Filter Type", filter_typeSelected, filter_typeOptions, FILTER_TYPE_COUNT, ImVec2(70, 0), "Filter Type"))
    {
        paramList[kAmsynthParameter_FilterType] = (float)filter_typeSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_FilterSlope()
{
    const int FILTER_SLOPE_COUNT = 2;
    unsigned char filter_slopeSelected = (int)paramList[kAmsynthParameter_FilterSlope];
    const char *filter_slopeOptions[FILTER_SLOPE_COUNT] = {"12 dB", "24 dB"};

    if (ImGui::ComboButton("Filter Slope", filter_slopeSelected, filter_slopeOptions, FILTER_SLOPE_COUNT, ImVec2(70, 0), "Filter Slope"))
    {
        paramList[kAmsynthParameter_FilterSlope] = (float)filter_slopeSelected;
        _onParamChange(paramList, effInstance);
    }
}

// ------ Filter basic options ------

void ImguiEditor::_AmsynthControl_FilterResonance()
{
    if (ImGui::Knob("Reson", &paramList[kAmsynthParameter_FilterResonance], paramMinValues[kAmsynthParameter_FilterResonance], paramMaxValues[kAmsynthParameter_FilterResonance], ImVec2(90, 40), "Resonance"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterCutoff()
{
    if (ImGui::Knob("Cut Off", &paramList[kAmsynthParameter_FilterCutoff], paramMinValues[kAmsynthParameter_FilterCutoff], paramMaxValues[kAmsynthParameter_FilterCutoff], ImVec2(90, 40), "Cut Off"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterKeyTrackAmount()
{
    if (ImGui::Knob("Key Track", &paramList[kAmsynthParameter_FilterKeyTrackAmount], paramMinValues[kAmsynthParameter_FilterKeyTrackAmount], paramMaxValues[kAmsynthParameter_FilterKeyTrackAmount], ImVec2(90, 40), "Key Track"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvAmount()
{
    if (ImGui::Knob("Env Amt", &paramList[kAmsynthParameter_FilterEnvAmount], paramMinValues[kAmsynthParameter_FilterEnvAmount], paramMaxValues[kAmsynthParameter_FilterEnvAmount], ImVec2(90, 40), "Env Amount"))
        _onParamChange(paramList, effInstance);
}

// ------ Filter ADSR ------

void ImguiEditor::_AmsynthControl_FilterEnvAttack()
{
    if (ImGui::Knob("FLT Attack", &paramList[kAmsynthParameter_FilterEnvAttack], paramMinValues[kAmsynthParameter_FilterEnvAttack], paramMaxValues[kAmsynthParameter_FilterEnvAttack], ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvDecay()
{
    if (ImGui::Knob("FLT Decay", &paramList[kAmsynthParameter_FilterEnvDecay], paramMinValues[kAmsynthParameter_FilterEnvDecay], paramMaxValues[kAmsynthParameter_FilterEnvDecay], ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvSustain()
{
    if (ImGui::Knob("FLT Sustain", &paramList[kAmsynthParameter_FilterEnvSustain], paramMinValues[kAmsynthParameter_FilterEnvSustain], paramMaxValues[kAmsynthParameter_FilterEnvSustain], ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvRelease()
{
    if (ImGui::Knob("FLT Release", &paramList[kAmsynthParameter_FilterEnvRelease], paramMinValues[kAmsynthParameter_FilterEnvRelease], paramMaxValues[kAmsynthParameter_FilterEnvRelease], ImVec2(90, 40), ""))
        _onParamChange(paramList, effInstance);
}

// Velocity strategy - How to process velocity

void ImguiEditor::_AmsynthControl_FilterKeyVelocityAmount()
{
    if (ImGui::Knob("VEL -> FLT", &paramList[kAmsynthParameter_FilterKeyVelocityAmount], paramMinValues[kAmsynthParameter_FilterKeyVelocityAmount], paramMaxValues[kAmsynthParameter_FilterKeyVelocityAmount], ImVec2(100, 40), "Velocity to Filter Amount"))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpVelocityAmount()
{
    if (ImGui::Knob("VEL -> AMP", &paramList[kAmsynthParameter_AmpVelocityAmount], paramMinValues[kAmsynthParameter_AmpVelocityAmount], paramMaxValues[kAmsynthParameter_AmpVelocityAmount], ImVec2(100, 40), "Velocity to Amp Amount"))
        _onParamChange(paramList, effInstance);
}
