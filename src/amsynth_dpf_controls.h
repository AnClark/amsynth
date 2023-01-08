#ifndef AMSYNTH_DPF_CONTROLS_H
#define AMSYNTH_DPF_CONTROLS_H

#include <string>

typedef enum {
    kAmsynthState_BankName,
    kAmsynthState_BankId,
    kAmsynthState_PresetName,
    kAmsynthState_PresetId,
    kAmsynthState_LastTouchedBankName,
    kAmsynthState_LastTouchedBankId,
    kAmsynthStateCount
} Amsynth_StateType;

struct AmsynthState {
    std::string bankName;
    int bankId;
    std::string presetName;
    int presetId;
    std::string lastTouchedBankName;
    int lastTouchedBankId;

    AmsynthState()
    {
        bankName = "Default";
        bankId = 0;
        presetName = "No Preset";
        presetId = 0;
        lastTouchedBankName = "Default";
        lastTouchedBankId = 0;
    }
};

#endif // AMSYNTH_DPF_CONTROLS_H