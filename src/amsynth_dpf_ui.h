#pragma once

#include "controls.h"

#include "DistrhoUI.hpp"
#include "ResizeHandle.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

class AmsynthPluginUI : public UI {

    float fParamValues[kAmsynthParameterCount];
    float fParamMinValues[kAmsynthParameterCount], fParamMaxValues[kAmsynthParameterCount];
    float fParamDefaultValues[kAmsynthParameterCount];
    float fParamStepSizes[kAmsynthParameterCount];

    ResizeHandle fResizeHandle;

    // ----------------------------------------------------------------------------------------------------------------

public:
    AmsynthPluginUI();
    ~AmsynthPluginUI();

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
