#ifndef IMGUI_OSCILLOSCOPE_H
#define IMGUI_OSCILLOSCOPE_H

#include <imgui.h>

namespace ImGui
{

    IMGUI_API void Oscilloscope(char const *label, float *samples, int sampleCount, int drawSampleCount = 180, ImVec2 graphSize = ImVec2(0, 0));

} // namespace ImGui

#endif // IMGUI_OSCILLOSCOPE_H
