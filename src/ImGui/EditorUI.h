#ifndef _EDITOR_UI_H
#define _EDITOR_UI_H

#include "../amsynth_dpf_ui.h"

class EditorUI {
    AmsynthPluginUI* fUI; // UI instance

    bool showPresetWindow;

public:
    EditorUI(AmsynthPluginUI* uiInstance);
    ~EditorUI();

    void amsynthWindow_Main();

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // Widgets / Controllers

    void _AmsynthControl_AmpEnvAttack();
    void _AmsynthControl_AmpEnvDecay();
    void _AmsynthControl_AmpEnvSustain();
    void _AmsynthControl_AmpEnvRelease();

    void _AmsynthControl_Oscillator1Waveform();

    void _AmsynthControl_FilterEnvAttack();
    void _AmsynthControl_FilterEnvDecay();
    void _AmsynthControl_FilterEnvSustain();
    void _AmsynthControl_FilterEnvRelease();
    void _AmsynthControl_FilterResonance();
    void _AmsynthControl_FilterEnvAmount();
    void _AmsynthControl_FilterCutoff();

    void _AmsynthControl_Oscillator2Detune(); // OSC2 detune
    void _AmsynthControl_Oscillator2Waveform(); // OSC2 waveform

    void _AmsynthControl_MasterVolume(); // Main volume

    void _AmsynthControl_LFOFreq(); // LFO frequency (speed)
    void _AmsynthControl_LFOWaveform(); // LFO waveform

    void _AmsynthControl_Oscillator2Octave();
    void _AmsynthControl_OscillatorMix();

    void _AmsynthControl_LFOToOscillators(); // Freq Mod Amount
    void _AmsynthControl_LFOToFilterCutoff();
    void _AmsynthControl_LFOToAmp();

    void _AmsynthControl_OscillatorMixRingMod();

    void _AmsynthControl_Oscillator1Pulsewidth();
    void _AmsynthControl_Oscillator2Pulsewidth();

    void _AmsynthControl_ReverbRoomsize();
    void _AmsynthControl_ReverbDamp();
    void _AmsynthControl_ReverbWet();
    void _AmsynthControl_ReverbWidth();

    void _AmsynthControl_AmpDistortion();

    void _AmsynthControl_Oscillator2Sync();

    void _AmsynthControl_PortamentoTime();

    void _AmsynthControl_KeyboardMode();

    void _AmsynthControl_Oscillator2Pitch();
    void _AmsynthControl_FilterType();
    void _AmsynthControl_FilterSlope();

    void _AmsynthControl_LFOOscillatorSelect();

    void _AmsynthControl_FilterKeyTrackAmount();
    void _AmsynthControl_FilterKeyVelocityAmount();

    void _AmsynthControl_AmpVelocityAmount();

    void _AmsynthControl_PortamentoMode();

    void _AmsynthControl_Oscilloscope();

    // ----------------------------------------------------------------------------------------------------------------
    // Windows / Panels

    void _AmsynthWindow_Preset();

private:
    // ----------------------------------------------------------------------------------------------------------------
    // Helper functions for building up widgets

    void _insertKnob(const char* label, uint32_t paramIndex);
    void _insertKnobInt(const char *label, uint32_t paramIndex, int &paramIntStorage);    // FIXME: This does not work!

    // ----------------------------------------------------------------------------------------------------------------
    // Helper functions for resources
    void _loadBuiltInFonts();

    // ----------------------------------------------------------------------------------------------------------------
    // Helper functions for states

    void _updateBankState(const char* bankName, const int bankId);
    void _updatePresetState(const char* presetName, const int presetId);
    void _updateLastTouchedBank(const char* bankName, const int bankId);
};

#endif // _EDITOR_UI_H