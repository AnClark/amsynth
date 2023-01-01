#include "amsynth_dpf.h"

// Headers for reimplementing Synthesizer class
#include "MidiController.h"
#include "VoiceAllocationUnit.h"
#include "VoiceBoard/VoiceBoard.h"
#include <cassert>

#include "EmbedPresetController.h"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

/**
    Populate how many factory banks and programs should the plugin load.
*/
constexpr size_t kPresetsPerBank = sizeof(BankInfo::presets) / sizeof(BankInfo::presets[0]); // Preset count per bank
static const int numPrograms = EmbedPresetController::getPresetBanks().size() * kPresetsPerBank; // Total program count

/**
    Plugin class constructor.@n
    You must set all parameter values to their defaults, matching ParameterRanges::def.
 */
AmsynthPlugin::AmsynthPlugin()
    : Plugin(kAmsynthParameterCount, numPrograms, 0) // parameters, programs, states
{
    // Must explicitly set a default sample rate first,
    // because derived Synthesizer class cannot obtain default value.
    fSynthesizer->setSampleRate(44100);
}

// -----------------------------------------------------------------------
// Init

/**
    Initialize the parameter @a index.@n
    This function will be called once, shortly after the plugin is created.
*/
void AmsynthPlugin::initParameter(uint32_t index, DISTRHO::Parameter& parameter)
{
    ::Preset amsynth_preset;
    ::Parameter& amsynth_parameter = amsynth_preset.getParameter(index);

    parameter.name = amsynth_parameter.getName().c_str();
    parameter.shortName = amsynth_parameter.getName().c_str();
    parameter.symbol = amsynth_parameter.getName().c_str();
    parameter.unit = amsynth_parameter.getLabel().c_str();
    parameter.ranges.min = amsynth_parameter.getMin();
    parameter.ranges.max = amsynth_parameter.getMax();
    parameter.ranges.def = amsynth_parameter.getDefault();
    parameter.hints = kParameterIsAutomatable;

    // Set special parameter type (e.g. Boolean, Integer)
    // NOTICE: DPF does not support custom parameter value display [via fSynthesizer->getParameterDisplay()],
    //         so in generic UI the values will not be capable with their units.
    switch (index) {
    case kAmsynthParameter_Oscillator1Waveform:
    case kAmsynthParameter_Oscillator2Waveform:
        parameter.hints |= kParameterIsInteger;
        break;

    case kAmsynthParameter_Oscillator2Sync:
        parameter.hints |= kParameterIsBoolean;
        break;
    }
}

/**
    Set the name of the program @a index.@n
    This function will be called once, shortly after the plugin is created.@n
    Must be implemented by your plugin class only if DISTRHO_PLUGIN_WANT_PROGRAMS is enabled.
*/
/**
    TODO: Issues about plugin programs
    - DPF has not implement program in VST2
    - DPF's VST3/CLAP program implementation is partial
    - Each factory bank has up to 128 presets, however there are 28 banks. So DPF should support dynamically
      initialize programs using std::vector.
    - REAPER can only show no more than about 507 programs in its dropdown menu.
*/
void AmsynthPlugin::initProgramName(uint32_t index, String& programName)
{
    // Prepare all the built-in program names.
    // Host will query every built-in program by invoking effGetProgramNameIndexed.
    // @param index: The program index where the host queries

    // Reserve program #0 as default/customized program as "Initial",
    // so that users can load user-defined presets from host instead of always being redirected to factory program #0.
    //
    // NOTE: Hosts like REAPER will set index to 0 when loading user-defined presets.
    //       But some hosts don't. See "case effSetProgram" for another polyfill.
    if (index <= 0) {
        programName = "-- Init / User --";
        return;
    }

    // Check if there were preset files or not
    // If you don't check, host will crash if no preset was installed
    if (EmbedPresetController::getPresetBanks().size()) {
        // As #0 is "Initial", other factory programs should be put into the next hole.
        // "index" is host's display index, and "actual_index" is the corresponding bank's index.
        int actual_index = index - 1;

        // Obtain bank and preset
        auto& bank = EmbedPresetController::getPresetBanks().at(actual_index / kPresetsPerBank);
        auto& preset = bank.presets[actual_index % kPresetsPerBank];

        // Set current obtained preset name, then show the name on host
        programName = preset.getName().c_str();

        //d_stderr("Preset index: %d, bank: %s, name: %s", index, bank.name.c_str(), preset.getName().c_str());

        return;
    } else {
        // Do not continue accessing if no preset was found
        return;
    }
}

// ----------------------------------------------------------------------------------------------------------------
// Internal data

/**
    Get the current value of a parameter.@n
    The host may call this function from any context, including realtime processing.
*/
float AmsynthPlugin::getParameterValue(uint32_t index) const
{
    // NOTICE: Do not use getNormalizedParameterValue(), since it does not behave well on DPF.
    return fSynthesizer->getParameterValue((Param)index);
}

/**
    Change a parameter value.@n
    The host may call this function from any context, including realtime processing.@n
    When a parameter is marked as automatable, you must ensure no non-realtime operations are performed.
    @note This function will only be called for parameter inputs.
*/
void AmsynthPlugin::setParameterValue(uint32_t index, float value)
{
    // NOTICE: Do not use setNormalizedParameterValue(), since it does not behave well on DPF.
    fSynthesizer->setParameterValue((Param)index, value);
}

/**
    Load a program.@n
    The host may call this function from any context, including realtime processing.@n
    Must be implemented by your plugin class only if DISTRHO_PLUGIN_WANT_PROGRAMS is enabled.
*/
void AmsynthPlugin::loadProgram(uint32_t index)
{
    // Obtain bank and preset
    // The corresponding factory program index is just by one item before host's index.
    int actual_index = index >= 1 ? index - 1 : 0; // In case of possible error
    auto& bank = EmbedPresetController::getPresetBanks().at(actual_index / kPresetsPerBank);
    auto& preset = bank.presets[actual_index % kPresetsPerBank];

    // Apply preset
    fSynthesizer->_presetController->setCurrentPreset(preset);
}

// ----------------------------------------------------------------------------------------------------------------
// Audio/MIDI Processing

/**
    Activate this plugin.
*/
void AmsynthPlugin::activate()
{
    // Currently unimplemented
}

/**
    Run/process function for plugins without MIDI input.
    @note Some parameters might be null if there are no audio inputs or outputs.
*/
void AmsynthPlugin::run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount)
{
    std::vector<amsynth_midi_cc_t> midi_out;
    fSynthesizer->process(frames, midiEvents, midiEventCount, midi_out, outputs[0], outputs[1]);
}

// ----------------------------------------------------------------------------------------------------------------
// Callbacks (optional)

/**
   Optional callback to inform the plugin about a sample rate change.@n
   This function will only be called when the plugin is deactivated.
   @see getSampleRate()
 */
void AmsynthPlugin::sampleRateChanged(double newSampleRate)
{
    fSynthesizer->setSampleRate((int)newSampleRate);
}

// --------------------------------------------------------------------------------------------------------------------

Synthesizer_DPF::Synthesizer_DPF()
    : Synthesizer()
{
}

void Synthesizer_DPF::process(unsigned int nframes,
    const MidiEvent*& midi_in, uint32_t midi_in_event_count,
    std::vector<amsynth_midi_cc_t>& midi_out,
    float* audio_l, float* audio_r, unsigned audio_stride)
{
    if (_sampleRate < 0) {
        assert(nullptr == "sample rate has not been set");
        return;
    }
    if (getIfNeedsResetAllVoices()) {
        setIfNeedsResetAllVoices(false);
        _voiceAllocationUnit->resetAllVoices();
    }
    uint32_t event_index = 0;
    unsigned frames_left_in_buffer = nframes, frame_index = 0;
    while (frames_left_in_buffer) {
        while (event_index < midi_in_event_count && midi_in[event_index].frame <= frame_index) {
            _midiController->HandleMidiData(midi_in[event_index].data, midi_in[event_index].size);
            ++event_index;
        }

        unsigned block_size_frames = std::min(frames_left_in_buffer, (unsigned)VoiceBoard::kMaxProcessBufferSize);
        if (event_index < midi_in_event_count && midi_in[event_index].frame > frame_index) {
            unsigned frames_until_next_event = midi_in[event_index].frame - frame_index;
            block_size_frames = std::min(block_size_frames, frames_until_next_event);
        }

        _voiceAllocationUnit->Process(audio_l + (frame_index * audio_stride),
            audio_r + (frame_index * audio_stride),
            block_size_frames, audio_stride);

        frame_index += block_size_frames;
        frames_left_in_buffer -= block_size_frames;
    }
    while (event_index < midi_in_event_count) {
        _midiController->HandleMidiData(midi_in[event_index].data, midi_in[event_index].size);
        ++event_index;
    }
    _midiController->generateMidiOutput(midi_out);
}

// --------------------------------------------------------------------------------------------------------------------

DISTRHO::Plugin* createPlugin()
{
    return new AmsynthPlugin();
}

END_NAMESPACE_DISTRHO
