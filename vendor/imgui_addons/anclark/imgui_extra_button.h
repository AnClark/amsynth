#ifndef IMGUI_EXTRA_BUTTON_H
#define IMGUI_EXTRA_BUTTON_H

#include <imgui.h>

#include "../zynlab/imgui_knob.h"

namespace ImGui
{

    IMGUI_API bool ComboButton(char const *label, unsigned char &value, char const *const names[], unsigned int nameCount, ImVec2 const &size, char const *tooltip);
    IMGUI_API bool SelectorPanel(char const *label, char const *names[], unsigned char &value, const unsigned int nameCount, char const *tooltips[] = NULL, ImVec2 const &size = ImVec2(0, 0), const unsigned int tableColumn = 1);

} // namespace ImGui

#endif // IMGUI_EXTRA_BUTTON_H
