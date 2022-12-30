#include "imgui_extra_button.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <stdio.h>
#include <string.h>

/**
 * Calculate string hash.
 * Reference: https://www.cnblogs.com/moyujiang/p/11213535.html
 */
int mini_hash(const char *str)
{
    // Mod factors. Both P and MOD should be prime number.
    const int P = 13;
    const int MOD = 101;

    const int len = strlen(str);
    int hash[len] = {0};

    for (int i = 1; i < len; i++)
        hash[i] = (hash[i - 1]) * P + (int)(str[i]) % MOD; // Hash formula

    return hash[len - 1];
}

bool ImGui::ComboButton(char const *label, unsigned char &value, char const *const names[], unsigned int nameCount, ImVec2 const &size, char const *tooltip)
{
    bool value_changed = false;
    auto current_effect_item = names[value];
    char button_label[128];

    if (ImGui::BeginPopupContextItem(label))
    {
        for (int i = 0; i < nameCount; i++)
        {
            bool is_selected = (current_effect_item == names[i]);
            if (ImGui::Selectable(names[i]))
            {
                value = i;
                value_changed = true;
            }
        }

        ImGui::EndPopup();
    }

    sprintf(button_label, "%s##%d", names[value], mini_hash(label));
    if (ImGui::Button(button_label, size))
        ImGui::OpenPopup(label);

    if (tooltip)
        ImGui::ShowTooltipOnHover(tooltip);

    return value_changed;
}

bool ImGui::SelectorPanel(char const *label, char const *names[], unsigned char &value, const unsigned int nameCount,
                          char const *tooltips[], ImVec2 const &size, const unsigned int tableColumn)
{
    bool value_changed = false;

    if (ImGui::BeginTable(label, tableColumn, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders, size))
    {
        for (int i = 0; i < nameCount; i++)
        {
            ImGui::TableNextColumn();
            if (ImGui::Selectable(names[i], value == i))
            {
                value = i;
                value_changed = true;
            }

            if (tooltips)
                ImGui::ShowTooltipOnHover(tooltips[i]);
        }
        ImGui::EndTable();
    }

    return value_changed;
}
