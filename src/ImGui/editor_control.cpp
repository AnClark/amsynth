/*
 *  editor_control.h - Code of amsynth editor controllers
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

/** FontAudio icon font */
#include "IconsFontaudio.h" // FontAudio UTF-8 codepoints

#define KNOB_PARAMS(index) &paramList[index], paramMinValues[index], paramMaxValues[index]

void ImguiEditor::_AmsynthControl_Oscilloscope()
{
    ImGui::Oscilloscope("##Oscilloscope", currentSample, numCurrentSample, 180, ImVec2(0, 80.0f));
}

void ImguiEditor::_AmsynthControl_Oscillator1Waveform()
{
    const unsigned char OSC1_WAVEFORM_COUNT = 5;
    static const char *osc1_waveformOptions[OSC1_WAVEFORM_COUNT] = {
        ICON_FAD_MODSINE, ICON_FAD_MODSQUARE, ICON_FAD_MODTRI, ICON_FAD_MODRANDOM, ICON_FAD_WAVEFORM};
    static const char *osc1_waveformTooltips[OSC1_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise",
                                                                     "Noise / Sample (Hold)"};
    unsigned char osc1_waveformSelected = (char)paramList[kAmsynthParameter_Oscillator1Waveform];

    if (ImGui::SelectorPanel("osc1_waveform_selector", osc1_waveformOptions, osc1_waveformSelected,
                             OSC1_WAVEFORM_COUNT, osc1_waveformTooltips, ImVec2(50, 0), 1))
    {
        paramList[kAmsynthParameter_Oscillator1Waveform] = (float)osc1_waveformSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_Oscillator1Pulsewidth()
{
    if (ImGuiKnobs::Knob("Shape##OSC1", KNOB_PARAMS(kAmsynthParameter_Oscillator1Pulsewidth)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_Oscillator2Waveform()
{
    const unsigned char OSC2_WAVEFORM_COUNT = 5;
    static const char *osc2_waveformOptions[OSC2_WAVEFORM_COUNT] = {
        ICON_FAD_MODSINE, ICON_FAD_MODSQUARE, ICON_FAD_MODTRI, ICON_FAD_MODRANDOM, ICON_FAD_WAVEFORM};
    static const char *osc2_waveformTooltips[OSC2_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise",
                                                                     "Noise / Sample (Hold)"};
    unsigned char osc2_waveformSelected = (char)paramList[kAmsynthParameter_Oscillator2Waveform];

    if (ImGui::SelectorPanel("osc2_waveform_selector", osc2_waveformOptions, osc2_waveformSelected, OSC2_WAVEFORM_COUNT,
                             osc2_waveformTooltips, ImVec2(50, 0), 1))
    {
        paramList[kAmsynthParameter_Oscillator2Waveform] = (float)osc2_waveformSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_Oscillator2Pulsewidth()
{
    if (ImGuiKnobs::Knob("Shape##OSC2", KNOB_PARAMS(kAmsynthParameter_Oscillator2Pulsewidth)))
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
    int val_Oscillator2Octave = (int)paramList[kAmsynthParameter_Oscillator2Octave];
    if (ImGuiKnobs::KnobInt("Octave", &val_Oscillator2Octave, (int)paramMinValues[kAmsynthParameter_Oscillator2Octave],
                            (int)paramMaxValues[kAmsynthParameter_Oscillator2Octave], 0.0F, (const char *)__null,
                            ImGuiKnobVariant_Stepped))
    {
        paramList[kAmsynthParameter_Oscillator2Octave] = (float)val_Oscillator2Octave;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_Oscillator2Detune()
{
    if (ImGuiKnobs::Knob("Detune", KNOB_PARAMS(kAmsynthParameter_Oscillator2Detune)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_Oscillator2Pitch()
{
    // Use KnobInt from imgui-knobs
    // KnobInt only supports int, so I need to convert between float and int
    int val_Oscillator2Pitch = (int)paramList[kAmsynthParameter_Oscillator2Pitch]; // Using local var is enough. No
                                                                                   // need to use TLS
                                                                                   // (`thread_local static int`)
    if (ImGuiKnobs::KnobInt("Pitch", &val_Oscillator2Pitch, (int)paramMinValues[kAmsynthParameter_Oscillator2Pitch],
                            (int)paramMaxValues[kAmsynthParameter_Oscillator2Pitch], 0.0F, (const char *)__null,
                            ImGuiKnobVariant_Stepped))
    {
        paramList[kAmsynthParameter_Oscillator2Pitch] = (float)val_Oscillator2Pitch;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_AmpEnvAttack()
{
    if (ImGuiKnobs::Knob("Attack", KNOB_PARAMS(kAmsynthParameter_AmpEnvAttack)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpEnvDecay()
{
    if (ImGuiKnobs::Knob("Decay", KNOB_PARAMS(kAmsynthParameter_AmpEnvDecay)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpEnvSustain()
{
    if (ImGuiKnobs::Knob("Sustain", KNOB_PARAMS(kAmsynthParameter_AmpEnvSustain)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpEnvRelease()
{
    if (ImGuiKnobs::Knob("Release", KNOB_PARAMS(kAmsynthParameter_AmpEnvRelease)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpDistortion()
{
    if (ImGuiKnobs::Knob("Distortion", KNOB_PARAMS(kAmsynthParameter_AmpDistortion)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_MasterVolume()
{
    if (ImGuiKnobs::Knob("Master Volume", KNOB_PARAMS(kAmsynthParameter_MasterVolume)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_OscillatorMix()
{
    if (ImGuiKnobs::Knob("Balance", KNOB_PARAMS(kAmsynthParameter_OscillatorMix)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_OscillatorMixRingMod()
{
    if (ImGuiKnobs::Knob("Ring Mod", KNOB_PARAMS(kAmsynthParameter_OscillatorMixRingMod)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOWaveform()
{
    const unsigned char LFO_WAVEFORM_COUNT = 7;
    static const char *lfo_waveformOptions[LFO_WAVEFORM_COUNT] = {
        ICON_FAD_MODSINE,  ICON_FAD_MODSQUARE, ICON_FAD_MODTRI,    ICON_FAD_MODRANDOM,
        ICON_FAD_WAVEFORM, ICON_FAD_MODSAWUP,  ICON_FAD_MODSAWDOWN};
    static const char *lfo_waveformTooltips[LFO_WAVEFORM_COUNT] = {
        "Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample & Hold", "Sawtooth (up)", "Sawtooth (down)"};
    unsigned char lfo_waveformSelected = (char)paramList[kAmsynthParameter_LFOWaveform];

    if (ImGui::SelectorPanel("lfo_waveform_selector", lfo_waveformOptions, lfo_waveformSelected, LFO_WAVEFORM_COUNT,
                             lfo_waveformTooltips, ImVec2(70, 0), 2))
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

    if (ImGui::ComboButton("OSC Selector", lfo_oscSelected, lfo_oscSelectorOptions, LFO_OSC_SELECTION_COUNT,
                           ImVec2(70, 0), "OSC Selector"))
    {
        paramList[kAmsynthParameter_LFOOscillatorSelect] = (float)lfo_oscSelected;
        _onParamChange(paramList, effInstance);
    }
}

void ImguiEditor::_AmsynthControl_LFOFreq()
{
    if (ImGuiKnobs::Knob("Frequency", KNOB_PARAMS(kAmsynthParameter_LFOFreq)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOToOscillators()
{
    if (ImGuiKnobs::Knob("Mod Amount", KNOB_PARAMS(kAmsynthParameter_LFOToOscillators)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOToFilterCutoff()
{
    if (ImGuiKnobs::Knob("To Filter", KNOB_PARAMS(kAmsynthParameter_LFOToFilterCutoff)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_LFOToAmp()
{
    if (ImGuiKnobs::Knob("To Amp", KNOB_PARAMS(kAmsynthParameter_LFOToAmp)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbWet()
{
    if (ImGuiKnobs::Knob("Amount", KNOB_PARAMS(kAmsynthParameter_ReverbWet))) // Reverb amount
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbRoomsize()
{
    if (ImGuiKnobs::Knob("Size", KNOB_PARAMS(kAmsynthParameter_ReverbRoomsize))) // Room size
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbDamp()
{
    if (ImGuiKnobs::Knob("Damp", KNOB_PARAMS(kAmsynthParameter_ReverbDamp)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_ReverbWidth()
{
    if (ImGuiKnobs::Knob("Width", KNOB_PARAMS(kAmsynthParameter_ReverbWidth)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_PortamentoTime()
{
    if (ImGuiKnobs::Knob("Portamento Time", KNOB_PARAMS(kAmsynthParameter_PortamentoTime)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_PortamentoMode()
{
    ImGui::BeginGroup();
    {
        const int PORTAMENTO_MODE_COUNT = 2;
        unsigned char portamento_modeSelected = (int)paramList[kAmsynthParameter_PortamentoMode];
        const char *portamento_modeOptions[PORTAMENTO_MODE_COUNT] = {"Always", "Legato"};

#if 0
        if (ImGui::ComboButton("Portamento Mode", portamento_modeSelected, portamento_modeOptions,
                               PORTAMENTO_MODE_COUNT, ImVec2(100, 0), nullptr))
        {
            paramList[kAmsynthParameter_PortamentoMode] = (float)portamento_modeSelected;
            _onParamChange(paramList, effInstance);
        }
#else
        if (ImGui::SelectorPanel("Portamento Mode", portamento_modeOptions, portamento_modeSelected,
                                 PORTAMENTO_MODE_COUNT, NULL, ImVec2(80, 0)))
        {
            paramList[kAmsynthParameter_PortamentoMode] = (float)portamento_modeSelected;
            _onParamChange(paramList, effInstance);
        }
#endif
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

        if (ImGui::ComboButton("Keyboard Mode", keyboard_modeSelected, keyboard_modeOptions, KEYBOARD_MODE_COUNT,
                               ImVec2(100, 0), nullptr))
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
    static const char *filter_typeOptions[FILTER_TYPE_COUNT] = {ICON_FAD_FILTER_LOWPASS, ICON_FAD_FILTER_HIGHPASS,
                                                                ICON_FAD_FILTER_BANDPASS, ICON_FAD_FILTER_NOTCH,
                                                                ICON_FAD_FILTER_BYPASS};
    static const char *filter_typeTooltips[FILTER_TYPE_COUNT] = {"Low Pass", "High Pass", "Band Pass", "Notch",
                                                                 "Bypass"};
    unsigned char filter_typeSelected = (int)paramList[kAmsynthParameter_FilterType];

    if (ImGui::SelectorPanel("filter_type_selector", filter_typeOptions, filter_typeSelected, FILTER_TYPE_COUNT,
                             filter_typeTooltips, ImVec2(70, 0), 1))
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

    if (ImGui::ComboButton("Filter Slope", filter_slopeSelected, filter_slopeOptions, FILTER_SLOPE_COUNT, ImVec2(70, 0),
                           "Filter Slope"))
    {
        paramList[kAmsynthParameter_FilterSlope] = (float)filter_slopeSelected;
        _onParamChange(paramList, effInstance);
    }
}

// ------ Filter basic options ------

void ImguiEditor::_AmsynthControl_FilterResonance()
{
    if (ImGuiKnobs::Knob("Reson", KNOB_PARAMS(kAmsynthParameter_FilterResonance)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterCutoff()
{
    if (ImGuiKnobs::Knob("Cut Off", KNOB_PARAMS(kAmsynthParameter_FilterCutoff)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterKeyTrackAmount()
{
    if (ImGuiKnobs::Knob("Key Track", KNOB_PARAMS(kAmsynthParameter_FilterKeyTrackAmount)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvAmount()
{
    if (ImGuiKnobs::Knob("Env Amt", KNOB_PARAMS(kAmsynthParameter_FilterEnvAmount)))
        _onParamChange(paramList, effInstance);
}

// ------ Filter ADSR ------

void ImguiEditor::_AmsynthControl_FilterEnvAttack()
{
    if (ImGuiKnobs::Knob("Attack##FLT", KNOB_PARAMS(kAmsynthParameter_FilterEnvAttack)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvDecay()
{
    if (ImGuiKnobs::Knob("Decay##FLT", KNOB_PARAMS(kAmsynthParameter_FilterEnvDecay)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvSustain()
{
    if (ImGuiKnobs::Knob("Sustain##FLT", KNOB_PARAMS(kAmsynthParameter_FilterEnvSustain)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_FilterEnvRelease()
{
    if (ImGuiKnobs::Knob("Release##FLT", KNOB_PARAMS(kAmsynthParameter_FilterEnvRelease)))
        _onParamChange(paramList, effInstance);
}

// Velocity strategy - How to process velocity

void ImguiEditor::_AmsynthControl_FilterKeyVelocityAmount()
{
    // Velocity to Filter Amount
    if (ImGuiKnobs::Knob("VEL -> FLT", KNOB_PARAMS(kAmsynthParameter_FilterKeyVelocityAmount)))
        _onParamChange(paramList, effInstance);
}

void ImguiEditor::_AmsynthControl_AmpVelocityAmount()
{
    // Velocity to Amp Amount
    if (ImGuiKnobs::Knob("VEL -> AMP", KNOB_PARAMS(kAmsynthParameter_AmpVelocityAmount)))
        _onParamChange(paramList, effInstance);
}
