#include "EditorUI.h"

#include "imgui_addons/anclark/imgui_extra_button.h"

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

#include "EditorUI.h"

#include "imgui_addons/anclark/imgui_extra_button.h"
#include "imgui_addons/imgui-knobs/imgui-knobs.h"

/** FontAudio icon font */
#include "IconsFontaudio.h" // FontAudio UTF-8 codepoints

#define KNOB_PARAMS(index) &fUI->fParamValues[index], fUI->fParamMinValues[index], fUI->fParamMaxValues[index]
#define SET_PARAM_VALUE(index) fUI->setParameterValue(index, fUI->fParamValues[index])

#define EDIT_PARAM_ON(index) fUI->editParameter(index, true)
#define EDIT_PARAM_OFF(index) fUI->editParameter(index, false)

void EditorUI::_AmsynthControl_Oscilloscope()
{
#if 0 // TODO: Any better way to fetch sample data?
    ImGui::Oscilloscope("##Oscilloscope", currentSample, numCurrentSample, 180, ImVec2(0, 80.0f));
#endif
}

void EditorUI::_AmsynthControl_Oscillator1Waveform()
{
    const unsigned char OSC1_WAVEFORM_COUNT = 5;
    static const char* osc1_waveformOptions[OSC1_WAVEFORM_COUNT] = {
        ICON_FAD_MODSINE, ICON_FAD_MODSQUARE, ICON_FAD_MODTRI, ICON_FAD_MODRANDOM, ICON_FAD_WAVEFORM
    };
    static const char* osc1_waveformTooltips[OSC1_WAVEFORM_COUNT] = { "Sine", "Square", "Triangle", "Whitenoise",
        "Noise / Sample (Hold)" };

    if (ImGui::SelectorPanel("osc1_waveform_selector", osc1_waveformOptions, fUI->fParamValues[kAmsynthParameter_Oscillator1Waveform],
            OSC1_WAVEFORM_COUNT, osc1_waveformTooltips, ImVec2(50, 0), 1)) {
        EDIT_PARAM_ON(kAmsynthParameter_Oscillator1Waveform);
        SET_PARAM_VALUE(kAmsynthParameter_Oscillator1Waveform);
        EDIT_PARAM_OFF(kAmsynthParameter_Oscillator1Waveform);
    }
}

void EditorUI::_AmsynthControl_Oscillator1Pulsewidth()
{
    _insertKnob("Shape##OSC1", kAmsynthParameter_Oscillator1Pulsewidth);
}

void EditorUI::_AmsynthControl_Oscillator2Waveform()
{
    const unsigned char OSC2_WAVEFORM_COUNT = 5;
    static const char* osc2_waveformOptions[OSC2_WAVEFORM_COUNT] = {
        ICON_FAD_MODSINE, ICON_FAD_MODSQUARE, ICON_FAD_MODTRI, ICON_FAD_MODRANDOM, ICON_FAD_WAVEFORM
    };
    static const char* osc2_waveformTooltips[OSC2_WAVEFORM_COUNT] = { "Sine", "Square", "Triangle", "Whitenoise",
        "Noise / Sample (Hold)" };

    if (ImGui::SelectorPanel("osc2_waveform_selector", osc2_waveformOptions, fUI->fParamValues[kAmsynthParameter_Oscillator2Waveform], OSC2_WAVEFORM_COUNT,
            osc2_waveformTooltips, ImVec2(50, 0), 1)) {
        EDIT_PARAM_ON(kAmsynthParameter_Oscillator2Waveform);
        SET_PARAM_VALUE(kAmsynthParameter_Oscillator2Waveform);
        EDIT_PARAM_OFF(kAmsynthParameter_Oscillator2Waveform);
    }
}

void EditorUI::_AmsynthControl_Oscillator2Pulsewidth()
{
    _insertKnob("Shape##OSC2", kAmsynthParameter_Oscillator2Pulsewidth);
}

void EditorUI::_AmsynthControl_Oscillator2Sync()
{
    bool osc2_syncWithOSC1Toggle = fUI->fParamValues[kAmsynthParameter_Oscillator2Sync] >= 1.0f ? true : false;
    if (ImGui::Checkbox("Sync to OSC1", &osc2_syncWithOSC1Toggle)) {
        if (ImGui::IsItemActivated())
            EDIT_PARAM_ON(kAmsynthParameter_Oscillator2Sync);

        fUI->fParamValues[kAmsynthParameter_Oscillator2Sync] = osc2_syncWithOSC1Toggle ? 1.0f : 0.0f;
        fUI->setParameterValue(kAmsynthParameter_Oscillator2Sync, osc2_syncWithOSC1Toggle ? 1.0f : 0.0f);

        if (ImGui::IsItemDeactivated())
            EDIT_PARAM_OFF(kAmsynthParameter_Oscillator2Sync);
    }
}

void EditorUI::_AmsynthControl_Oscillator2Octave()
{
    int val_Oscillator2Octave = (int)fUI->fParamValues[kAmsynthParameter_Oscillator2Octave];
    if (ImGuiKnobs::KnobInt("Octave", &val_Oscillator2Octave, (int)fUI->fParamMinValues[kAmsynthParameter_Oscillator2Octave],
            (int)fUI->fParamMaxValues[kAmsynthParameter_Oscillator2Octave], 0.0F, (const char*)nullptr,
            ImGuiKnobVariant_Stepped)) {
        if (ImGui::IsItemActivated())
            fUI->editParameter(kAmsynthParameter_Oscillator2Octave, true);

        fUI->fParamValues[kAmsynthParameter_Oscillator2Octave] = (float)val_Oscillator2Octave;
        fUI->setParameterValue(kAmsynthParameter_Oscillator2Octave, (float)val_Oscillator2Octave);

        if (ImGui::IsItemDeactivated())
            fUI->editParameter(kAmsynthParameter_Oscillator2Octave, false);
    }
}

void EditorUI::_AmsynthControl_Oscillator2Detune()
{
    _insertKnob("Detune", kAmsynthParameter_Oscillator2Detune);
}

void EditorUI::_AmsynthControl_Oscillator2Pitch()
{
    // Use KnobInt from imgui-knobs
    // KnobInt only supports int, so I need to convert between float and int
    int val_Oscillator2Pitch = (int)fUI->fParamValues[kAmsynthParameter_Oscillator2Pitch];
    if (ImGuiKnobs::KnobInt("Pitch", &val_Oscillator2Pitch, (int)fUI->fParamMinValues[kAmsynthParameter_Oscillator2Pitch],
            (int)fUI->fParamMaxValues[kAmsynthParameter_Oscillator2Pitch], 0.0F, (const char*)nullptr,
            ImGuiKnobVariant_Stepped)) {
        if (ImGui::IsItemActivated())
            fUI->editParameter(kAmsynthParameter_Oscillator2Pitch, true);

        fUI->fParamValues[kAmsynthParameter_Oscillator2Pitch] = (float)val_Oscillator2Pitch;
        fUI->setParameterValue(kAmsynthParameter_Oscillator2Pitch, (float)val_Oscillator2Pitch);

        if (ImGui::IsItemDeactivated())
            fUI->editParameter(kAmsynthParameter_Oscillator2Pitch, false);
    }
}

void EditorUI::_AmsynthControl_AmpEnvAttack()
{
    _insertKnob("Attack", kAmsynthParameter_AmpEnvAttack);
}

void EditorUI::_AmsynthControl_AmpEnvDecay()
{
    _insertKnob("Decay", kAmsynthParameter_AmpEnvDecay);
}

void EditorUI::_AmsynthControl_AmpEnvSustain()
{
    _insertKnob("Sustain", kAmsynthParameter_AmpEnvSustain);
}

void EditorUI::_AmsynthControl_AmpEnvRelease()
{
    _insertKnob("Release", kAmsynthParameter_AmpEnvRelease);
}

void EditorUI::_AmsynthControl_AmpDistortion()
{
    _insertKnob("Distortion", kAmsynthParameter_AmpDistortion);
}

void EditorUI::_AmsynthControl_MasterVolume()
{
    _insertKnob("Master Volume", kAmsynthParameter_MasterVolume);
}

void EditorUI::_AmsynthControl_OscillatorMix()
{
    _insertKnob("Balance", kAmsynthParameter_OscillatorMix);
}

void EditorUI::_AmsynthControl_OscillatorMixRingMod()
{
    _insertKnob("Ring Mod", kAmsynthParameter_OscillatorMixRingMod);
}

void EditorUI::_AmsynthControl_LFOWaveform()
{
    const unsigned char LFO_WAVEFORM_COUNT = 7;
    static const char* lfo_waveformOptions[LFO_WAVEFORM_COUNT] = {
        ICON_FAD_MODSINE, ICON_FAD_MODSQUARE, ICON_FAD_MODTRI, ICON_FAD_MODRANDOM,
        ICON_FAD_WAVEFORM, ICON_FAD_MODSAWUP, ICON_FAD_MODSAWDOWN
    };
    static const char* lfo_waveformTooltips[LFO_WAVEFORM_COUNT] = {
        "Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample & Hold", "Sawtooth (up)", "Sawtooth (down)"
    };

    if (ImGui::SelectorPanel("lfo_waveform_selector", lfo_waveformOptions, fUI->fParamValues[kAmsynthParameter_LFOWaveform], LFO_WAVEFORM_COUNT,
            lfo_waveformTooltips, ImVec2(70, 0), 2)) {
        EDIT_PARAM_ON(kAmsynthParameter_LFOWaveform);
        SET_PARAM_VALUE(kAmsynthParameter_LFOWaveform);
        EDIT_PARAM_OFF(kAmsynthParameter_LFOWaveform);
    }
}

void EditorUI::_AmsynthControl_LFOOscillatorSelect()
{
    const int LFO_OSC_SELECTION_COUNT = 3;
    const char* lfo_oscSelectorOptions[LFO_OSC_SELECTION_COUNT] = { "OSC 1+2", "OSC 1", "OSC2" };

    if (ImGui::ComboButton("OSC Selector", fUI->fParamValues[kAmsynthParameter_LFOOscillatorSelect], lfo_oscSelectorOptions, LFO_OSC_SELECTION_COUNT,
            ImVec2(70, 0), "OSC Selector")) {
        EDIT_PARAM_ON(kAmsynthParameter_LFOOscillatorSelect);
        SET_PARAM_VALUE(kAmsynthParameter_LFOOscillatorSelect);
        EDIT_PARAM_OFF(kAmsynthParameter_LFOOscillatorSelect);
    }
}

void EditorUI::_AmsynthControl_LFOFreq()
{
    _insertKnob("Frequency", kAmsynthParameter_LFOFreq);
}

void EditorUI::_AmsynthControl_LFOToOscillators()
{
    _insertKnob("Mod Amount", kAmsynthParameter_LFOToOscillators);
}

void EditorUI::_AmsynthControl_LFOToFilterCutoff()
{
    _insertKnob("To Filter", kAmsynthParameter_LFOToFilterCutoff);
}

void EditorUI::_AmsynthControl_LFOToAmp()
{
    _insertKnob("To Amp", kAmsynthParameter_LFOToAmp);
}

void EditorUI::_AmsynthControl_ReverbWet()
{
    _insertKnob("Amount", kAmsynthParameter_ReverbWet); // Reverb amount
}

void EditorUI::_AmsynthControl_ReverbRoomsize()
{
    _insertKnob("Size", kAmsynthParameter_ReverbRoomsize); // Room size
}

void EditorUI::_AmsynthControl_ReverbDamp()
{
    _insertKnob("Damp", kAmsynthParameter_ReverbDamp);
}

void EditorUI::_AmsynthControl_ReverbWidth()
{
    _insertKnob("Width", kAmsynthParameter_ReverbWidth);
}

void EditorUI::_AmsynthControl_PortamentoTime()
{
    _insertKnob("Portamento Time", kAmsynthParameter_PortamentoTime);
}

void EditorUI::_AmsynthControl_PortamentoMode()
{
    ImGui::BeginGroup();
    {
        const int PORTAMENTO_MODE_COUNT = 2;
        const char* portamento_modeOptions[PORTAMENTO_MODE_COUNT] = { "Always", "Legato" };

#if 0
        if (ImGui::ComboButton("Portamento Mode", fUI->fParamValues[kAmsynthParameter_PortamentoMode], portamento_modeOptions,
                               PORTAMENTO_MODE_COUNT, ImVec2(100, 0), nullptr))
        {
            EDIT_PARAM_ON(kAmsynthParameter_PortamentoMode);
            SET_PARAM_VALUE(kAmsynthParameter_PortamentoMode);
            EDIT_PARAM_OFF(kAmsynthParameter_PortamentoMode);
        }
#else
        if (ImGui::SelectorPanel("Portamento Mode", portamento_modeOptions, fUI->fParamValues[kAmsynthParameter_PortamentoMode],
                PORTAMENTO_MODE_COUNT, NULL, ImVec2(80, 0))) {
            EDIT_PARAM_ON(kAmsynthParameter_PortamentoMode);
            SET_PARAM_VALUE(kAmsynthParameter_PortamentoMode);
            EDIT_PARAM_OFF(kAmsynthParameter_PortamentoMode);
        }
#endif
    }
    ImGui::Text("Portamento Mode");
    ImGui::EndGroup();
}

void EditorUI::_AmsynthControl_KeyboardMode()
{
    ImGui::BeginGroup();
    {
        const int KEYBOARD_MODE_COUNT = 3;
        const char* keyboard_modeOptions[KEYBOARD_MODE_COUNT] = { "Poly", "Mono", "Legato" };

        if (ImGui::ComboButton("Keyboard Mode", fUI->fParamValues[kAmsynthParameter_KeyboardMode], keyboard_modeOptions, KEYBOARD_MODE_COUNT,
                ImVec2(100, 0), nullptr)) {
            EDIT_PARAM_ON(kAmsynthParameter_KeyboardMode);
            SET_PARAM_VALUE(kAmsynthParameter_KeyboardMode);
            EDIT_PARAM_OFF(kAmsynthParameter_KeyboardMode);
        }
    }
    ImGui::Text("Keyboard Mode");
    ImGui::EndGroup();
}

// ------ Filter option selectors ------

void EditorUI::_AmsynthControl_FilterType()
{
    const int FILTER_TYPE_COUNT = 5;
    static const char* filter_typeOptions[FILTER_TYPE_COUNT] = { ICON_FAD_FILTER_LOWPASS, ICON_FAD_FILTER_HIGHPASS,
        ICON_FAD_FILTER_BANDPASS, ICON_FAD_FILTER_NOTCH,
        ICON_FAD_FILTER_BYPASS };
    static const char* filter_typeTooltips[FILTER_TYPE_COUNT] = { "Low Pass", "High Pass", "Band Pass", "Notch",
        "Bypass" };

    if (ImGui::SelectorPanel("filter_type_selector", filter_typeOptions, fUI->fParamValues[kAmsynthParameter_FilterType], FILTER_TYPE_COUNT,
            filter_typeTooltips, ImVec2(70, 0), 1)) {
        EDIT_PARAM_ON(kAmsynthParameter_FilterType);
        SET_PARAM_VALUE(kAmsynthParameter_FilterType);
        EDIT_PARAM_OFF(kAmsynthParameter_FilterType);
    }
}

void EditorUI::_AmsynthControl_FilterSlope()
{
    const int FILTER_SLOPE_COUNT = 2;
    const char* filter_slopeOptions[FILTER_SLOPE_COUNT] = { "12 dB", "24 dB" };

    if (ImGui::ComboButton("Filter Slope", fUI->fParamValues[kAmsynthParameter_FilterSlope], filter_slopeOptions, FILTER_SLOPE_COUNT, ImVec2(70, 0),
            "Filter Slope")) {
        EDIT_PARAM_ON(kAmsynthParameter_FilterSlope);
        SET_PARAM_VALUE(kAmsynthParameter_FilterSlope);
        EDIT_PARAM_OFF(kAmsynthParameter_FilterSlope);
    }
}

// ------ Filter basic options ------

void EditorUI::_AmsynthControl_FilterResonance()
{
    _insertKnob("Reson", kAmsynthParameter_FilterResonance);
}

void EditorUI::_AmsynthControl_FilterCutoff()
{
    _insertKnob("Cut Off", kAmsynthParameter_FilterCutoff);
}

void EditorUI::_AmsynthControl_FilterKeyTrackAmount()
{
    _insertKnob("Key Track", kAmsynthParameter_FilterKeyTrackAmount);
}

void EditorUI::_AmsynthControl_FilterEnvAmount()
{
    _insertKnob("Env Amt", kAmsynthParameter_FilterEnvAmount);
}

// ------ Filter ADSR ------

void EditorUI::_AmsynthControl_FilterEnvAttack()
{
    _insertKnob("Attack##FLT", kAmsynthParameter_FilterEnvAttack);
}

void EditorUI::_AmsynthControl_FilterEnvDecay()
{
    _insertKnob("Decay##FLT", kAmsynthParameter_FilterEnvDecay);
}

void EditorUI::_AmsynthControl_FilterEnvSustain()
{
    _insertKnob("Sustain##FLT", kAmsynthParameter_FilterEnvSustain);
}

void EditorUI::_AmsynthControl_FilterEnvRelease()
{
    _insertKnob("Release##FLT", kAmsynthParameter_FilterEnvRelease);
}

// Velocity strategy - How to process velocity

void EditorUI::_AmsynthControl_FilterKeyVelocityAmount()
{
    // Velocity to Filter Amount
    _insertKnob("VEL -> FLT", kAmsynthParameter_FilterKeyVelocityAmount);
}

void EditorUI::_AmsynthControl_AmpVelocityAmount()
{
    // Velocity to Amp Amount
    _insertKnob("VEL -> AMP", kAmsynthParameter_AmpVelocityAmount);
}
