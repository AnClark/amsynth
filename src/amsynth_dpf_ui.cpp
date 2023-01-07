/*
 * ImGui plugin example
 * Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
 * SPDX-License-Identifier: ISC
 */

#include "amsynth_dpf_ui.h"
#include "ImGui/EditorUI.h"

#include "Preset.h"

START_NAMESPACE_DISTRHO

/**
   UI class constructor.
   The UI should be initialized to a default state that matches the plugin side.
 */
AmsynthPluginUI::AmsynthPluginUI()
    : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT)
    , fResizeHandle(this)
    , fEditorUI(nullptr)
{
    setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT, true);

    // hide handle if UI is resizable
    if (isResizable())
        fResizeHandle.hide();

    // Initialize parameters
    _initParameterProperties();

    // Create ImGui UI assistant instance
    fEditorUI = new EditorUI(this);
}

AmsynthPluginUI::~AmsynthPluginUI() { }

// ----------------------------------------------------------------------------------------------------------------
// DSP/Plugin Callbacks

/**
   A parameter has changed on the plugin side.@n
   This is called by the host to inform the UI about parameter changes.
 */
void AmsynthPluginUI::parameterChanged(uint32_t index, float value)
{
    fParamValues[index] = value;

    repaint();
}

/**
    A program has been loaded on the plugin side.@n
    This is called by the host to inform the UI about program changes.
*/
void AmsynthPluginUI::programLoaded(uint32_t index)
{
    repaint();
}

/**
   A state has changed on the plugin side.@n
   This is called by the host to inform the UI about state changes.
 */
void AmsynthPluginUI::stateChanged(const char* key, const char* value)
{
    d_stderr("Invoked stateChanged(). Key = %s, Value = %s", key, value);

    repaint();
}

// ----------------------------------------------------------------------------------------------------------------
// Widget Callbacks

/**
  ImGui specific onDisplay function.
*/
void AmsynthPluginUI::onImGuiDisplay()
{
#if 0
    const float width = getWidth();
    const float height = getHeight();
    const float margin = 20.0f * getScaleFactor();

    ImGui::SetNextWindowPos(ImVec2(margin, margin));
    ImGui::SetNextWindowSize(ImVec2(width - 2 * margin, height - 2 * margin));

    if (ImGui::Begin("amsynth ui", nullptr, ImGuiWindowFlags_NoResize)) {
        static char aboutText[256] = "This is a demo plugin made with ImGui.\n";
        ImGui::InputTextMultiline("About", aboutText, sizeof(aboutText));

        if (ImGui::SliderFloat("amp_attack", &fParamValues[kAmsynthParameter_AmpEnvAttack], fParamMinValues[kAmsynthParameter_AmpEnvAttack], fParamMaxValues[kAmsynthParameter_AmpEnvAttack])) {
            if (ImGui::IsItemActivated())
                editParameter(kAmsynthParameter_AmpEnvAttack, true);

            setParameterValue(kAmsynthParameter_AmpEnvAttack, fParamValues[kAmsynthParameter_AmpEnvAttack]);
        }

        if (ImGui::IsItemDeactivated()) {
            editParameter(kAmsynthParameter_AmpEnvAttack, false);
        }
    }

    ImGui::End();
#else
    fEditorUI->amsynthWindow_Main();
#endif
}

// ----------------------------------------------------------------------------------------------------------------
// Internal Procedures

/**
  Initialize parameter properties.
  This function should be called only once in the constructor.
*/
void AmsynthPluginUI::_initParameterProperties()
{
    ::Preset preset;

    for (int index = 0; index < kAmsynthParameterCount; index++) {
        ::Parameter& parameter = preset.getParameter(index);

        fParamMinValues[index] = parameter.getMin();
        fParamMaxValues[index] = parameter.getMax();
        fParamDefaultValues[index] = parameter.getValue();
        fParamStepSizes[index] = parameter.getStep();
    }
}

// ----------------------------------------------------------------------------------------------------------------
// Utility APIs

/**
  Randomize parameters from UI side.
*/
void AmsynthPluginUI::randomiseParameters()
{
    ::Preset preset;

    for (int index = 0; index < kAmsynthParameterCount; index++) {
        ::Parameter& parameter = preset.getParameter(index);
        parameter.setValue(fParamValues[index]);

        if (index != kAmsynthParameter_MasterVolume) {
            parameter.randomise();
        }

        // Synchronize randomised parameter
        setParameterValue(index, parameter.getValue()); // DSP side
        fParamValues[index] = parameter.getValue(); // UI side
    }
}

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new AmsynthPluginUI();
}

END_NAMESPACE_DISTRHO
