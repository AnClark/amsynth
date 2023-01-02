/*
 * ImGui plugin example
 * Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
 * SPDX-License-Identifier: ISC
 */

/**
  NOTICE: About Including Header Files
    Both DPF and amsynth have the same class name "Parameter", which will cause conflict.
    Even though they are in different namespaces, confilct still occurs.
    So we must include amsynth's own header files first, then include DPF's.
*/

// Include amsynth header files first
#include "Parameter.h"
#include "types.h"

#include "Preset.h"
#include "controls.h"
#include "Synthesizer.h"

// Then, include DPF header file
#include "DistrhoPlugin.hpp"

// STL headers
#include <memory>
#include <vector>

// --------------------------------------------------------------------------------------------------------------------

START_NAMESPACE_DISTRHO

class Synthesizer_DPF : public Synthesizer
{
public:
    Synthesizer_DPF();

    String getParameterDisplay(Param parameter) const;

    void process(unsigned int nframes,
                            const MidiEvent* &midi_in, uint32_t midi_in_event_count,
                            std::vector<amsynth_midi_cc_t> &midi_out,
                            float *audio_l, float *audio_r, unsigned audio_stride = 1);
};

class AmsynthPlugin : public Plugin
{
    double fSampleRate = getSampleRate();
    std::unique_ptr<Synthesizer_DPF> fSynthesizer = std::make_unique<Synthesizer_DPF>();

public:
    AmsynthPlugin();

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // Information

   /**
      Get the plugin label.@n
      This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    */
    const char* getLabel() const noexcept override
    {
        return "amsynth";
    }

   /**
      Get an extensive comment/description about the plugin.@n
      Optional, returns nothing by default.
    */
    const char* getDescription() const override
    {
        return "Analogue Modeling SYNTHesizer";
    }

   /**
      Get the plugin author/maker.
    */
    const char* getMaker() const noexcept override
    {
        return "Nick Dowell, AnClark Liu";
    }

   /**
      Get the plugin license (a single line of text or a URL).@n
      For commercial plugins this should return some short copyright information.
    */
    const char* getLicense() const noexcept override
    {
        return "GPLv2";
    }

   /**
      Get the plugin version, in hexadecimal.
      @see d_version()
    */
    uint32_t getVersion() const noexcept override
    {
        return d_version(0, 1, 0);
    }

   /**
      Get the plugin unique Id.@n
      This value is used by LADSPA, DSSI and VST plugin formats.
      @see d_cconst()
    */
    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('a', 'm', 's', 't');
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, DISTRHO::Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;

    // ----------------------------------------------------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;
    String getParameterDisplay(uint32_t index) const override;
    void loadProgram(uint32_t index) override;

    // ----------------------------------------------------------------------------------------------------------------
    // Audio/MIDI Processing

    void activate() override;
    void run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount) override;

    // ----------------------------------------------------------------------------------------------------------------
    // Callbacks (optional)

    void sampleRateChanged(double newSampleRate) override;

    // ----------------------------------------------------------------------------------------------------------------

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmsynthPlugin)
};

END_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------
