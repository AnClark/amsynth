#include "EditorUI.h"

#include "imgui_addons/anclark/imgui_extra_button.h"
#include "imgui_addons/imgui-knobs/imgui-knobs.h"

#define KNOB_PARAMS(index) &fUI->fParamValues[index], fUI->fParamMinValues[index], fUI->fParamMaxValues[index]

EditorUI::EditorUI(AmsynthPluginUI* uiInstance)
    : fUI(uiInstance)
{
}

EditorUI::~EditorUI()
{
}

void EditorUI::_insertKnob(const char* label, uint32_t paramIndex)
{
    if (ImGuiKnobs::Knob(label, KNOB_PARAMS(paramIndex))) {
        if (ImGui::IsItemActivated())
            fUI->editParameter(paramIndex, true);

        fUI->setParameterValue(paramIndex, fUI->fParamValues[paramIndex]);

        if (ImGui::IsItemDeactivated())
            fUI->editParameter(paramIndex, false);
    }
}

// FIXME: This does not work!
void EditorUI::_insertKnobInt(const char *label, uint32_t paramIndex, int &paramIntStorage)
{
    paramIntStorage = (int)fUI->fParamValues[paramIndex];

    if (ImGuiKnobs::KnobInt(label, &paramIntStorage, (int)fUI->fParamMinValues[paramIndex],
                            (int)fUI->fParamMaxValues[paramIndex], 0.0F, (const char *)nullptr,
                            ImGuiKnobVariant_Stepped))
    {
        if (ImGui::IsItemActivated())
            fUI->editParameter(paramIndex, true);

        fUI->setParameterValue(paramIndex, (float)paramIntStorage);

        if (ImGui::IsItemDeactivated())
            fUI->editParameter(paramIndex, false);
    }
}
