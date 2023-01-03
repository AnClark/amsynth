#include "imgui_oscilloscope.h"

#include <stdlib.h>

/**
 * ImGui::Oscilloscope - Simple Oscilloscope based on ImGui::PlotLines()
 *
 * @param label             Widget's label.
 * @param samples           An array of sample data. Items are between (-1.0f, 1.0f).
 *                          Applications like VST plugins use this format.
 * @param sampleCount       Length of sample array.
 * @param drawSampleCount   Sample count to be rendered into oscilloscope.
 *                          You can consider that it controls "zooming":
 *                          The larger drawSampeCount, the smaller graphs, and the more waves in a screen.
 * @param graphSize         Widget's size.
 * 
 * @warning Do not specify too large value for drawSampleCount (e.g. Values > 256),
 *          or ImGui::PlotLines() won't be able to render.
 */
void ImGui::Oscilloscope(char const *label, float *samples, int sampleCount, int drawSampleCount, ImVec2 graphSize)
{

    if (sampleCount < drawSampleCount) // Make sure drawSampleCount is below sample array's size
        drawSampleCount = sampleCount;
    float *drawSamples = (float*)calloc(drawSampleCount, sizeof(float)); // Samples to be used to draw oscilloscope

    // Quantize sample value for ImGui::PlotLines() rendering
    // To make each sample value resides on the base of a middle value
    for (int i = 0; i < drawSampleCount; i++)
    {
        drawSamples[i] = 0.0f - (samples[i] * 2);
    }

    // Render oscilloscope with ImGui::PlotLines()
    // NOTICE: Remember to set scales properly: (scale_max + scale_min) / 2 = middle_value
    //         Use larger scale to prevent cutting off
    ImGui::PlotLines(label, drawSamples, drawSampleCount, 0, "", -2.0f, 2.0f, graphSize);
}