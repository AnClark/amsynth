#pragma once

#include "controls.h"

#include "DistrhoUI.hpp"
#include "ResizeHandle.hpp"

#include "amsynth_dpf_controls.h"

// Forward declarations
class EditorUI;
class Preset;

// --------------------------------------------------------------------------------------------------------------------

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

class AmsynthPluginUI : public UI {

    float fParamValues[kAmsynthParameterCount];
    float fParamMinValues[kAmsynthParameterCount], fParamMaxValues[kAmsynthParameterCount];
    float fParamDefaultValues[kAmsynthParameterCount];
    float fParamStepSizes[kAmsynthParameterCount];

    ResizeHandle fResizeHandle;

    EditorUI* fEditorUI; // ImGui editor UI assistant instance
    friend EditorUI; // Allow EditorUI to access private members (param properties)

    AmsynthState fState;

    // ----------------------------------------------------------------------------------------------------------------

public:
    AmsynthPluginUI();
    ~AmsynthPluginUI();

    // ----------------------------------------------------------------------------------------------------------------
    // Utility APIs

    void loadPreset(const Preset& preset);

    void randomiseParameters();
    void panic();

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // DSP/Plugin Callbacks

    void parameterChanged(uint32_t index, float value) override;
    void programLoaded(uint32_t index) override;
    void stateChanged(const char* key, const char* value) override;

    // ----------------------------------------------------------------------------------------------------------------
    // Widget Callbacks

    void onImGuiDisplay() override;

private:
    // ----------------------------------------------------------------------------------------------------------------
    // Internal Procedures

    void _initParameterProperties();

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmsynthPluginUI)
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
