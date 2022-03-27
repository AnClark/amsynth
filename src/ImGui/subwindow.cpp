#include "editor_pane.h"

void ImguiEditor::_AmsynthWindow_Preset()
{
    ImGui::Begin("Hello Amsynth! - Preset list");

    const int MAX_PRESET_NAME_LENGTH = 128;
    int bank_index = 0; // Count a bank index to help keeping item unique,
                        // especically in case which have same bank names and preset names
    for (auto &bank : PresetController::getPresetBanks())
    {
        char text[MAX_PRESET_NAME_LENGTH]; // Buffer

        /**
         * a. Create root nodes for each bank
         */
        snprintf(text, sizeof(text), "[%s] %s##%d", bank.read_only ? _("F") : _("U"), bank.name.c_str(), bank_index);
        if (ImGui::TreeNode(text))
        {
            /**
             * b. Create child nodes for each preset item
             *    Each bank has up to 127 presets, accessed by index.
             */
            static int selected = -1; // Current selection's index

            PresetController presetController;
            presetController.loadPresets(bank.file_path.c_str());
            for (int i = 0; i < PresetController::kNumPresets; i++)
            {
                snprintf(text, sizeof(text), "%d: %s##%d", i, presetController.getPreset(i).getName().c_str(), bank_index);

                char *bank_file_path = strdup(bank.file_path.c_str());
                size_t preset_index = (size_t)i;

                /**
                 * c. Apply preset when you click a preset item
                 */
                int node_index = i + mini_hash(text); // Calculate unique index (almost unique among normal usages)
                if (ImGui::Selectable(text, selected == node_index))
                {
                    panic(); // Stop all sound before continue

                    selected = node_index; // Mark selected item

                    PresetController presetController;
                    presetController.loadPresets(bank_file_path); // Load preset bank

                    Preset &preset = presetController.getPreset((int)preset_index); // Load preset item
                    for (unsigned int i = 0; i < kAmsynthParameterCount; i++)       // Apply preset parameters
                    {
                        float value = preset.getParameter(i).getValue();
                        paramList[i] = value;
                        _onParamChange(paramList, effInstance);
                    }
                }
            }

            ImGui::TreePop(); // Must add this, or ImGui will crash!

            bank_index++;
        }
    }
    ImGui::End();
}
