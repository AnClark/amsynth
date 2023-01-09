#include "EditorUI.h"

#include "EmbedPresetController.h"
#include "Preset.h"
#include "PresetController.h"
#include "controls.h"

#include "amsynth_dpf_banks.h"

static const int MAX_PRESET_NAME_LENGTH = 128;

// Mark the last bank which has been selected and whose preset has been loaded.
// Only the very picked preset will be highlighted, no matter how you switch banks.
static int last_touched_bank = 0;

void EditorUI::_AmsynthWindow_Preset()
{
    char text[MAX_PRESET_NAME_LENGTH];

    // Flags for vertical scrollable table
    static ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

    ImGui::SetNextWindowSize(ImVec2(600, 400));

    ImGui::Begin("Choose Preset");

    if (ImGui::BeginTable("_BANK_LIST", 1, tableFlags, ImVec2(200.0f, 0.0f))) {
        ImGui::TableNextColumn();

        if (ImGui::TreeNodeEx("Factory Banks", ImGuiTreeNodeFlags_DefaultOpen)) {
            int selected = s_bank_state.getCurrentBank();
            int index = 0;

            for (auto& bank : EmbedPresetController::getPresetBanks()) {
                snprintf(text, sizeof(text), "%s##%d", bank.name.c_str(), index);

                if (ImGui::Selectable(text, selected == index)) {
                    // Set current bank ID state
                    selected = index;
                    s_bank_state.setCurrentBank(index);

                    // TODO: Modify host side's bank index
                }
                index++;
            }

            ImGui::TreePop();
        }
        ImGui::EndTable();
    }

    ImGui::SameLine(0, 30);

    if (ImGui::BeginTable("_PRESET_LIST", 1, tableFlags, ImVec2(300.0f, 0.0f))) {
        ImGui::TableNextColumn();

        int selected = s_bank_state.getCurrentProgram();
        auto& current_bank = EmbedPresetController::getPresetBanks().at(s_bank_state.getCurrentBank());

        for (int index = 0; index < PresetController::kNumPresets; index++) {
            auto& current_preset = current_bank.presets[index];

            snprintf(text, sizeof(text), "%s##%d_%d", current_preset.getName().c_str(), s_bank_state.getCurrentBank(), index);
            if (ImGui::Selectable(text, (selected == index) && (last_touched_bank == s_bank_state.getCurrentBank()))) {
                // Set current program ID state
                selected = index;
                s_bank_state.setCurrentProgram(index);

                // Mark down this bank as the last touched bank
                last_touched_bank = s_bank_state.getCurrentBank();

                fUI->loadPreset(current_preset);
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();

    ImGui::SameLine();
}