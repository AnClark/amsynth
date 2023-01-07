#include "EditorUI.h"

#include "imgui_addons/anclark/imgui_extra_button.h"
#include "imgui_addons/imgui-knobs/imgui-knobs.h"

/** FontAudio icon font */
#include "IconsFontaudio.h" // FontAudio UTF-8 codepoints
#include "fontaudio.h" // FontAudio font data

#define KNOB_PARAMS(index) &fUI->fParamValues[index], fUI->fParamMinValues[index], fUI->fParamMaxValues[index]

EditorUI::EditorUI(AmsynthPluginUI* uiInstance)
    : fUI(uiInstance)
{
    _loadBuiltInFonts();
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

/**
    Load built-in fonts.
*/
void EditorUI::_loadBuiltInFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Merge icon font. See ImGui docs/FONTS.md
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = { ICON_MIN_FAD, ICON_MAX_FAD, 0 };
    io.Fonts->AddFontFromMemoryCompressedTTF(fontaudio_compressed_data, fontaudio_compressed_size, 16, &config,
        icon_ranges);
}
