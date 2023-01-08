#include "EditorUI.h"

#include "EmbedPresetController.h"
#include "Preset.h"
#include "PresetController.h"
#include "controls.h"

static const int MAX_PRESET_NAME_LENGTH = 128;

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
            int selected = fUI->fState.bankId;
            int index = 0;

            for (auto& bank : EmbedPresetController::getPresetBanks()) {
                snprintf(text, sizeof(text), "%s##%d", bank.name.c_str(), index);

                if (ImGui::Selectable(text, selected == index)) {
                    // Set current bank ID state
                    selected = index;

                    // Set current bank on state (WIP)
                    _updateBankState(bank.name.c_str(), index);

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

        int selected = fUI->fState.presetId;
        auto& current_bank = EmbedPresetController::getPresetBanks().at(fUI->fState.bankId);

        for (int index = 0; index < PresetController::kNumPresets; index++) {
            auto& current_preset = current_bank.presets[index];

            snprintf(text, sizeof(text), "%s##%d_%d", current_preset.getName().c_str(), fUI->fState.bankId, index);
            if (ImGui::Selectable(text, (selected == index) && (fUI->fState.lastTouchedBankId == fUI->fState.bankId))) {
                // Set current program ID state
                selected = index;
                _updatePresetState(current_preset.getName().c_str(), index);

                // Mark down this bank as the last touched bank
                _updateLastTouchedBank(fUI->fState.bankName.c_str(), fUI->fState.bankId);

                fUI->loadPreset(current_preset);
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();

    ImGui::SameLine();
}