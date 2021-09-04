#ifndef IMGUI_EXTRA_BUTTON_H
#define IMGUI_EXTRA_BUTTON_H

#include <imgui.h>

#include "../zynlab/imgui_knob.h"

namespace ImGui
{

    IMGUI_API bool ComboButton(char const *label, unsigned char &value, char const *const names[], unsigned int nameCount, ImVec2 const &size, char const *tooltip);

} // namespace ImGui

#endif // IMGUI_EXTRA_BUTTON_H
