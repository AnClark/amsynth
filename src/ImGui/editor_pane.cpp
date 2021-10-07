/*
 *  editor_pane.cpp
 *
 *  Copyright (c) 2021 AnClark Liu
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "editor_pane.h"
#include "font.h"

static int glfw_initialized_cnt;

/**
 * Calculate string hash.
 * Reference: https://www.cnblogs.com/moyujiang/p/11213535.html
 */
int mini_hash(char *str)
{
    // Mod factors. Both P and MOD should be prime number.
    const int P = 13;
    const int MOD = 101;

    const int len = strlen(str);
    int hash[len] = {0};

    for (int i = 0; i < len; i++)
        hash[i] = (hash[i - 1]) * P + (int)(str[i]) % MOD; // Hash formula

    return hash[len - 1];
}

#ifdef _WIN32

/**
 * Set / reset window's parent on Windows.
 * I handle these two functions with my modded GLFW (https://github.com/anclark/GLFW)
 */
void reparent_window([[maybe_unused]] GLFWwindow *window, void *host_window) {}

void reparent_window_to_root([[maybe_unused]] GLFWwindow *window) {}
#else
/**
 * Set / reset window's parent on Linux.
 * I handle these two functions with my modded GLFW (https://github.com/anclark/GLFW)
 */
void reparent_window([[maybe_unused]] GLFWwindow *window, void *host_window) {}

void reparent_window_to_root([[maybe_unused]] GLFWwindow *window) {}
#endif

void ImguiEditor::_getParamProperties(int parameter_index, double *minimum, double *maximum, double *default_value, double *step_size)
{
    Preset preset;
    Parameter &parameter = preset.getParameter(parameter_index);

    if (minimum)
        *minimum = parameter.getMin();

    if (maximum)
        *maximum = parameter.getMax();

    if (default_value)
        *default_value = parameter.getValue();

    if (step_size)
        *step_size = parameter.getStep();
}

void ImguiEditor::_getParamValues()
{
    for (int i = 0; i < kAmsynthParameterCount; i++)
    {
        paramList[i] = (float)synthInstance->getParameterValue((Param)i);

        paramNameList[i] = (char *)malloc(sizeof(char *) * 32);
        synthInstance->getParameterName((Param)i, paramNameList[i], 32);
    }
}

void ImguiEditor::panic()
{
    // Build MIDI data of All Sound Off
	unsigned char buffer[3] = { MIDI_STATUS_CONTROLLER, MIDI_CC_ALL_SOUND_OFF, 0};	// Buffer format: {status, data1, data2}

	// Directly apply MIDI data
	synthInstance->getMidiController()->HandleMidiData(buffer, 3);
}

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
#if _WIN32
    MessageBoxA(NULL, (LPCSTR)description, "Error", MB_OK);
#endif
}

ImguiEditor::ImguiEditor(void *parentId, int width, int height, Synthesizer *synthInstance)
{
    this->parentId = parentId;
    this->width = width;
    this->height = height;
    this->synthInstance = synthInstance;
}

ImguiEditor::~ImguiEditor()
{
    // Re-call closeEditor() in case user forget to call it
    closeEditor();
}

void ImguiEditor::setParamChangeCallback(ParamChangeCallback func, AEffect *effInstance)
{
    this->_onParamChange = func;
    this->effInstance = effInstance;
}

void ImguiEditor::setCurrentSample(int numSamples, float *samples)
{
    this->numCurrentSample = numSamples;
    this->currentSample = samples;
}

void ImguiEditor::_setupGLFW()
{
    /**
     * Setup window as well as initialized count.
     * Here I manage a reference count of UI instances and keep glfw initialized
     * when you have multiple instances open
     *
     * This is Justin Frankel's implementation. Noizebox also has this feature
     * included in his modded GLFW (from which I forked)
     */
    if (!glfw_initialized_cnt++)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return;
    }

    // Omit explicit version specification to let GLFW guess GL version,
    // or GLFW will fail to load on old environments with GL 2.x
#if 0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Do not allow resizing
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Disable decoration. Or you will see a weird titlebar :-)

    // Enable embedded window
    glfwWindowHint(GLFW_EMBEDDED_WINDOW, GLFW_TRUE);
    glfwWindowHintVoid(GLFW_PARENT_WINDOW_ID, this->parentId);

    window = glfwCreateWindow(this->width, this->height, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL); // This size is only the standalone window's size, NOT editor's size
    if (window == NULL)
        return;

    // Embed editor to host
    // On both Windows and Linux, there's an implementation within my modded GLFW.
    // So they are empty functions now.
    reparent_window(window, this->parentId);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
}

void ImguiEditor::_setupImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    myImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(myImGuiContext);

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Set actual editor UI size here
    io.DisplaySize.x = (float)this->width;
    io.DisplaySize.y = (float)this->height;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    /* The font is loaded from generated/font.h. The font file is in generated by the
     * binary_to_source utility included in Dear ImGui, this util is built and run by
     * CMake when generating the make files. Default font is Roboto
     * To change font, set the CMake varible INCLUDED_FONT */
    ImFontConfig config;
    io.Fonts->AddFontFromMemoryCompressedTTF(font_compressed_data, font_compressed_size, 16, &config);
}

void ImguiEditor::drawFrame()
{
    // Get current parameter names and values
    _getParamValues();

    // Called once per idle slice
    // Remember to check myImGuiContext before drawing frames, or ImGui_ImplOpenGL2_NewFrame() may execute
    // on an empty context after closeEditor()!
    if (myImGuiContext)
    {
        ImGui::SetCurrentContext(myImGuiContext);

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /** ==============================================================================
         *  Main Window Section
         *  ==============================================================================
         * 
         * Notes:
         * - ImGui widgets may use label name to identify each other. Keep them unique.
         *   To create widgets with same label, append "##%d" (%d means a unique integer).
         *   For example:
         *       ImGui::Button("Test Button##1");
         *       ImGui::Button("Test Button##2");
         * 
         * - Do not use static variables to store current values, especially for widgets
         *   using int / enum values (DropDown, SliderInt, etc.).
         *   Otherwise, you will not be able to get value from host when setting the value
         *   on host side, or loading programs.
         * 
         *   Call _onParamChange() instead. Parameter synchronization between DSP and
         *   editor will do the right things.
         */

        // Fullscreen window parameters
        static ImGuiWindowFlags flagsMainWindow = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        // Common buffers
        double lower = 0, upper = 0, step_increment = 0; // Buffer for initializing parameter range
                                                         // NOTICE: step_increment is not supported by ImGui
        char buttonLabel[20];                            // Buffer for creating unique button labels

        ImGui::Begin("Amsynth Main Window", (bool *)true, flagsMainWindow);

        // Section 01: OSC1
        {
            ImGui::BeginGroup();
            ImGui::Text("OSC 1");

            // Waveform Switch
            {
                const unsigned char OSC1_WAVEFORM_COUNT = 5;
                static const char *osc1_waveformOptions[OSC1_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample (Hold)"};
                unsigned char osc1_waveformSelected = (int)paramList[kAmsynthParameter_Oscillator1Waveform];
                if (ImGui::ComboButton("OSC1 Waveform", osc1_waveformSelected, osc1_waveformOptions, OSC1_WAVEFORM_COUNT, ImVec2(70, 0), "OSC1 Waveform"))
                {
                    paramList[kAmsynthParameter_Oscillator1Waveform] = (float)osc1_waveformSelected;
                    _onParamChange(paramList, effInstance);
                }
            }
            ImGui::SameLine();

            // Shape (Pulse Width)
            fetchParamRange(kAmsynthParameter_Oscillator1Pulsewidth);
            if (ImGui::Knob("Shape", &paramList[kAmsynthParameter_Oscillator1Pulsewidth], lower, upper, ImVec2(40, 40), "Shape"))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        // Section 01.5: Oscilloscope
        ImGui::SameLine();
        {
            ImGui::Oscilloscope("##Oscilloscope", currentSample, numCurrentSample, 180, ImVec2(0, 80.0f));
        }

        // Section 02: OSC2
        {
            ImGui::BeginGroup();
            ImGui::Text("OSC 2");

            // Waveform Switch
            {
                const unsigned char OSC2_WAVEFORM_COUNT = 5;
                static const char *osc2_waveformOptions[OSC2_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample (Hold)"};
                unsigned char osc2_waveformSelected = (int)paramList[kAmsynthParameter_Oscillator2Waveform];

                if (ImGui::ComboButton("OSC2 Waveform", osc2_waveformSelected, osc2_waveformOptions, OSC2_WAVEFORM_COUNT, ImVec2(70, 0), "OSC2 Waveform"))
                {
                    paramList[kAmsynthParameter_Oscillator2Waveform] = (float)osc2_waveformSelected;
                    _onParamChange(paramList, effInstance);
                }
            }

            ImGui::SameLine();

            // Shape (Pulse Width)
            fetchParamRange(kAmsynthParameter_Oscillator2Pulsewidth);
            if (ImGui::Knob("Shape 2", &paramList[kAmsynthParameter_Oscillator2Pulsewidth], lower, upper, ImVec2(40, 40), "Shape"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            // "Sync to OSC1" switch
            bool osc2_syncWithOSC1Toggle = paramList[kAmsynthParameter_Oscillator2Sync] >= 1.0f ? true : false;
            if (ImGui::Checkbox("Sync to OSC1", &osc2_syncWithOSC1Toggle))
            {
                paramList[kAmsynthParameter_Oscillator2Sync] = osc2_syncWithOSC1Toggle ? 1.0f : 0.0f;
                _onParamChange(paramList, effInstance);
            }

            ImGui::EndGroup();
        }

        // Section 03: Tune for OSC 2
        {
            ImGui::BeginGroup();
            ImGui::Text("Tune for OSC 2");

            // Octave
            int osc2_octaveValue = (int)paramList[kAmsynthParameter_Oscillator2Octave]; // NOTICE: Also avoid using static var
            if (ImGui::SliderInt("OSC2 Octave", &osc2_octaveValue, -3, 4))
            {
                paramList[kAmsynthParameter_Oscillator2Octave] = (float)osc2_octaveValue;
                _onParamChange(paramList, effInstance);
            }

            ImGui::SameLine();

            // Semitone
            // TODO: Create ImGui::KnobInt
            fetchParamRange(kAmsynthParameter_Oscillator2Pitch);
            if (ImGui::Knob("Pitch", &paramList[kAmsynthParameter_Oscillator2Pitch], lower, upper, ImVec2(90, 40), "Semitone"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            // Detune
            fetchParamRange(kAmsynthParameter_Oscillator2Detune);
            if (ImGui::Knob("Detune", &paramList[kAmsynthParameter_Oscillator2Detune], lower, upper, ImVec2(90, 40), "Detune"))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        // Section 04: AMP Envelope
        {
            ImGui::BeginGroup();
            ImGui::Text("AMP Envelope");

            const int rows = 4;
            const float spacing = 4;
            const ImVec2 small_slider_size(18, (float)(int)((320.0f - (rows - 1) * spacing) / rows));

            fetchParamRange(kAmsynthParameter_AmpEnvAttack);
            if (ImGui::VSliderFloat("Attack", small_slider_size, &paramList[kAmsynthParameter_AmpEnvAttack], lower, upper, ""))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_AmpEnvDecay);
            if (ImGui::VSliderFloat("Decay", small_slider_size, &paramList[kAmsynthParameter_AmpEnvDecay], lower, upper, ""))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_AmpEnvSustain);
            if (ImGui::VSliderFloat("Sustain", small_slider_size, &paramList[kAmsynthParameter_AmpEnvSustain], lower, upper, ""))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_AmpEnvRelease);
            if (ImGui::VSliderFloat("Release", small_slider_size, &paramList[kAmsynthParameter_AmpEnvRelease], lower, upper, ""))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        ImGui::SameLine();

        // Section 05: OSC Mix
        {
            ImGui::BeginGroup();
            ImGui::Text("OSC Mix");

            fetchParamRange(kAmsynthParameter_OscillatorMix);
            if (ImGui::Knob("OSC Balance", &paramList[kAmsynthParameter_OscillatorMix], lower, upper, ImVec2(70, 40), "OSC Mix"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_OscillatorMixRingMod);
            if (ImGui::Knob("Ring Mod", &paramList[kAmsynthParameter_OscillatorMixRingMod], lower, upper, ImVec2(80, 40), "Ring Mod"))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        ImGui::SameLine();

        // Section 06: AMP Volume / Drive
        {
            ImGui::BeginGroup();
            ImGui::Text("AMP");

            fetchParamRange(kAmsynthParameter_MasterVolume);
            if (ImGui::Knob("Master Volume", &paramList[kAmsynthParameter_MasterVolume], lower, upper, ImVec2(90, 40), "Master Volume"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_AmpDistortion);
            if (ImGui::Knob("Distortion", &paramList[kAmsynthParameter_AmpDistortion], lower, upper, ImVec2(70, 40), "Distortion"))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        // Section: LFO
        {
            ImGui::BeginGroup();
            ImGui::Text("LFO");
            ImGui::SameLine(0, 90);

            // LFO Waveform Switch
            {
                const unsigned char LFO_WAVEFORM_COUNT = 7;
                static const char *lfo_waveformOptions[LFO_WAVEFORM_COUNT] = {"Sine", "Square", "Triangle", "Whitenoise", "Noise / Sample & Hold", "Sawtooth (up)", "Sawtooth (down)"};
                static const char *lfo_waveformIcon[LFO_WAVEFORM_COUNT]; // TODO: Add icon resource
                unsigned char lfo_waveformSelected = (int)paramList[kAmsynthParameter_LFOWaveform];

                if (ImGui::ComboButton("LFO Waveform", lfo_waveformSelected, lfo_waveformOptions, LFO_WAVEFORM_COUNT, ImVec2(70, 0), "LFO Waveform"))
                {
                    paramList[kAmsynthParameter_LFOWaveform] = (float)lfo_waveformSelected;
                    _onParamChange(paramList, effInstance);
                }
            }
            ImGui::SameLine(0, 40);

            // LFO OSC selector
            {
                const int LFO_OSC_SELECTION_COUNT = 3;
                unsigned char lfo_oscSelected = (int)paramList[kAmsynthParameter_LFOOscillatorSelect];
                const char *lfo_oscSelectorOptions[LFO_OSC_SELECTION_COUNT] = {"OSC 1+2", "OSC 1", "OSC2"};

                if (ImGui::ComboButton("OSC Selector", lfo_oscSelected, lfo_oscSelectorOptions, LFO_OSC_SELECTION_COUNT, ImVec2(70, 0), "OSC Selector"))
                {
                    paramList[kAmsynthParameter_LFOOscillatorSelect] = (float)lfo_oscSelected;
                    _onParamChange(paramList, effInstance);
                }
            }
            // Frequency
            fetchParamRange(kAmsynthParameter_LFOFreq);
            if (ImGui::Knob("Speed", &paramList[kAmsynthParameter_LFOFreq], lower, upper, ImVec2(80, 40), "LFO Frequency"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            // Freq Mod Amount
            fetchParamRange(kAmsynthParameter_LFOToOscillators);
            if (ImGui::Knob("Mod Amount", &paramList[kAmsynthParameter_LFOToOscillators], lower, upper, ImVec2(90, 40), "LFO Mod Amount"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_LFOToFilterCutoff);
            if (ImGui::Knob("To Filter", &paramList[kAmsynthParameter_LFOToFilterCutoff], lower, upper, ImVec2(80, 40), "LFO to Filter"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_LFOToAmp);
            if (ImGui::Knob("To Amp", &paramList[kAmsynthParameter_LFOToAmp], lower, upper, ImVec2(80, 40), "LFO to Amp"))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        // Section: Reverb
        {
            ImGui::BeginGroup();
            ImGui::Text("Reverb");

            fetchParamRange(kAmsynthParameter_ReverbWet);
            if (ImGui::Knob("Amount", &paramList[kAmsynthParameter_ReverbWet], lower, upper, ImVec2(80, 40), "Reverb Amount"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_ReverbRoomsize);
            if (ImGui::Knob("Size", &paramList[kAmsynthParameter_ReverbRoomsize], lower, upper, ImVec2(80, 40), "Room Size"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_ReverbDamp);
            if (ImGui::Knob("Damp", &paramList[kAmsynthParameter_ReverbDamp], lower, upper, ImVec2(80, 40), "Damp"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_ReverbWidth);
            if (ImGui::Knob("Width", &paramList[kAmsynthParameter_ReverbWidth], lower, upper, ImVec2(80, 40), "Reverb Width"))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        ImGui::SameLine();

        // Section: Filter
        {
            ImGui::BeginGroup();
            ImGui::Text("Filter");
            ImGui::SameLine(0, 90);

            // ------ Filter option selectors ------
            {
                const int FILTER_TYPE_COUNT = 5;
                static const char *filter_typeOptions[FILTER_TYPE_COUNT] = {"Low Pass", "High Pass", "Band Pass", "Notch", "Bypass"};
                unsigned char filter_typeSelected = (int)paramList[kAmsynthParameter_FilterType];

                if (ImGui::ComboButton("Filter Type", filter_typeSelected, filter_typeOptions, FILTER_TYPE_COUNT, ImVec2(70, 0), "Filter Type"))
                {
                    paramList[kAmsynthParameter_FilterType] = (float)filter_typeSelected;
                    _onParamChange(paramList, effInstance);
                }
            }

            ImGui::SameLine(0, 40);

            {
                const int FILTER_SLOPE_COUNT = 2;
                unsigned char filter_slopeSelected = (int)paramList[kAmsynthParameter_FilterSlope];
                const char *filter_slopeOptions[FILTER_SLOPE_COUNT] = {"12 dB", "24 dB"};

                if (ImGui::ComboButton("Filter Slope", filter_slopeSelected, filter_slopeOptions, FILTER_SLOPE_COUNT, ImVec2(70, 0), "Filter Slope"))
                {
                    paramList[kAmsynthParameter_FilterSlope] = (float)filter_slopeSelected;
                    _onParamChange(paramList, effInstance);
                }
            }

            // ------ Filter basic options ------
            fetchParamRange(kAmsynthParameter_FilterResonance);
            if (ImGui::Knob("Reson", &paramList[kAmsynthParameter_FilterResonance], lower, upper, ImVec2(90, 40), "Resonance"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_FilterCutoff);
            if (ImGui::Knob("Cut Off", &paramList[kAmsynthParameter_FilterCutoff], lower, upper, ImVec2(90, 40), "Cut Off"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_FilterKeyTrackAmount);
            if (ImGui::Knob("Key Track", &paramList[kAmsynthParameter_FilterKeyTrackAmount], lower, upper, ImVec2(90, 40), "Key Track"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_FilterEnvAmount);
            if (ImGui::Knob("Env Amt", &paramList[kAmsynthParameter_FilterEnvAmount], lower, upper, ImVec2(90, 40), "Env Amount"))
                _onParamChange(paramList, effInstance);

            // ------ Filter ADSR ------
            fetchParamRange(kAmsynthParameter_FilterEnvAttack);
            if (ImGui::Knob("FLT Attack", &paramList[kAmsynthParameter_FilterEnvAttack], lower, upper, ImVec2(90, 40), ""))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_FilterEnvDecay);
            if (ImGui::Knob("FLT Decay", &paramList[kAmsynthParameter_FilterEnvDecay], lower, upper, ImVec2(90, 40), ""))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_FilterEnvSustain);
            if (ImGui::Knob("FLT Sustain", &paramList[kAmsynthParameter_FilterEnvSustain], lower, upper, ImVec2(90, 40), ""))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_FilterEnvRelease);
            if (ImGui::Knob("FLT Release", &paramList[kAmsynthParameter_FilterEnvRelease], lower, upper, ImVec2(90, 40), ""))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        // Section: Keyboard Options
        {
            ImGui::BeginGroup();
            ImGui::Text("Keyboard Options");

            fetchParamRange(kAmsynthParameter_PortamentoTime);
            if (ImGui::Knob("Portamento Time", &paramList[kAmsynthParameter_PortamentoTime], lower, upper, ImVec2(100, 40), "Portamento Time"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            {
                ImGui::BeginGroup();
                {
                    const int PORTAMENTO_MODE_COUNT = 2;
                    unsigned char portamento_modeSelected = (int)paramList[kAmsynthParameter_PortamentoMode];
                    const char *portamento_modeOptions[PORTAMENTO_MODE_COUNT] = {"Always", "Legato"};

                    if (ImGui::ComboButton("Portamento Mode", portamento_modeSelected, portamento_modeOptions, PORTAMENTO_MODE_COUNT, ImVec2(100, 0), nullptr))
                    {
                        paramList[kAmsynthParameter_PortamentoMode] = (float)portamento_modeSelected;
                        _onParamChange(paramList, effInstance);
                    }
                }
                ImGui::Text("Portamento Mode");
                ImGui::EndGroup();
            }

            ImGui::SameLine();

            {
                ImGui::BeginGroup();
                {
                    const int KEYBOARD_MODE_COUNT = 3;
                    unsigned char keyboard_modeSelected = (int)paramList[kAmsynthParameter_KeyboardMode];
                    const char *keyboard_modeOptions[KEYBOARD_MODE_COUNT] = {"Poly", "Mono", "Legato"};

                    if (ImGui::ComboButton("Keyboard Mode", keyboard_modeSelected, keyboard_modeOptions, KEYBOARD_MODE_COUNT, ImVec2(100, 0), nullptr))
                    {
                        paramList[kAmsynthParameter_KeyboardMode] = (float)keyboard_modeSelected;
                        _onParamChange(paramList, effInstance);
                    }
                }
                ImGui::Text("Keyboard Mode");
                ImGui::EndGroup();
            }

            ImGui::EndGroup();
        }

        ImGui::SameLine();

        // Section: Velocity strategy - How to process velocity
        {
            ImGui::BeginGroup();
            ImGui::Text("Velocity Strategy");

            fetchParamRange(kAmsynthParameter_FilterKeyVelocityAmount);
            if (ImGui::Knob("VEL -> FLT", &paramList[kAmsynthParameter_FilterKeyVelocityAmount], lower, upper, ImVec2(100, 40), "Velocity to Filter Amount"))
                _onParamChange(paramList, effInstance);

            ImGui::SameLine();

            fetchParamRange(kAmsynthParameter_AmpVelocityAmount);
            if (ImGui::Knob("VEL -> AMP", &paramList[kAmsynthParameter_AmpVelocityAmount], lower, upper, ImVec2(100, 40), "Velocity to Amp Amount"))
                _onParamChange(paramList, effInstance);

            ImGui::EndGroup();
        }

        // Section: Option buttons
        // TODO: Must move to elsewhere properly
        {
            if (ImGui::Button("Presets", ImVec2(90, 0)))
                ImguiEditor::showPresetWindow = !ImguiEditor::showPresetWindow;

            ImGui::SameLine();

            if (ImGui::Button("Randomise", ImVec2(90, 0)))
                synthInstance->getPresetController()->randomiseCurrentPreset();

            ImGui::SameLine();

            if (ImGui::Button("Panic", ImVec2(90, 0)))
                panic();
        }

        /**
         * ================== FREE TEST ==================
         */
        {
            //ImGui::ImGuiWindow *window = ImGui::GetCurrentWindow();
            ImGuiEx::EnvelopeEditor(200.0f, 100.0f, &paramList[kAmsynthParameter_AmpEnvAttack], &paramList[kAmsynthParameter_AmpEnvDecay], &paramList[kAmsynthParameter_AmpEnvSustain], &paramList[kAmsynthParameter_AmpEnvRelease], ImGuiEnvelopeEditorType_ADSR);
        }

        ImGui::End();

        /** ==============================================================================
         *  Preset Window Section
         *  ==============================================================================
         * 
         * TODO: Sync with host's program viewer (Memorymoon behavior)
         */
        if (showPresetWindow)
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

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        //ImGuiIO &io = ImGui::GetIO();
        //glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        //GLint last_program;
        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //glUseProgram(last_program);

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
}

void ImguiEditor::openEditor()
{
    _setupGLFW();
    _setupImGui();
}

void ImguiEditor::closeEditor()
{
    if (myImGuiContext)
    {
        // Set current context to make sure that the following two shutdown functions
        // can be in right context
        ImGui::SetCurrentContext(myImGuiContext);

        // Cleanup
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(myImGuiContext);
        myImGuiContext = nullptr;

        glfwDestroyWindow(window);
        if (!--glfw_initialized_cnt)
            glfwTerminate();
    }
}
