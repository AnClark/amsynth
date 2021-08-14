/*
 *  amsynth_vst.cpp
 *
 *  Copyright (c) 2008-2020 Nick Dowell
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "midi.h"
#include "Preset.h"
#include "PresetController.h"
#include "Synthesizer.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <vestige/aeffectx.h>

// from http://www.asseca.org/vst-24-specs/index.html
#define effGetParamLabel        6
#define effGetParamDisplay      7
#define effGetChunk             23
#define effSetChunk             24
#define effCanBeAutomated       26
#define effGetOutputProperties  34
#define effGetTailSize          52
#define effGetMidiKeyName       66
#define effBeginLoadBank        75
#define effFlagsProgramChunks   (1 << 5)

#ifdef WITH_GUI
// TODO: ImGui-related headers
#include "ImGui/editor_pane.h"
#endif

struct ERect
{
	short top;
	short left;
	short bottom;
	short right;
};

static char hostProductString[64] = "";

constexpr size_t kPresetsPerBank = sizeof(BankInfo::presets) / sizeof(BankInfo::presets[0]);

struct Plugin
{
	Plugin(audioMasterCallback master)
	{
		audioMaster = master;
		synthesizer = new Synthesizer;
		midiBuffer = (unsigned char *)malloc(4096);
#ifdef WITH_GUI
		editorInstance = nullptr;
#endif
	}

	~Plugin()
	{ 
		delete synthesizer;
		free(midiBuffer);
	}

	audioMasterCallback audioMaster;
	Synthesizer *synthesizer;
	unsigned char *midiBuffer;
	std::vector<amsynth_midi_event_t> midiEvents;
	int programNumber = 0;
	std::string presetName;
#ifdef WITH_GUI
	ImguiEditor *editorInstance;
#endif
};

#ifdef WITH_GUI
#if 0
static void on_adjustment_value_changed(GtkAdjustment *adjustment, AEffect *effect)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

	static Preset dummyPreset;

	for (int i = 0; i < kAmsynthParameterCount; i++) {
		if (adjustment == plugin->adjustments[i]) {
			float value = gtk_adjustment_get_value(adjustment);
			Parameter &param = dummyPreset.getParameter(i);
			param.setValue(value);
			plugin->synthesizer->setParameterValue((Param)i, value);
			if (plugin->audioMaster && !strstr(hostProductString, "Qtractor")) {
				plugin->audioMaster(effect, audioMasterAutomate, i, 0, nullptr, param.getNormalisedValue());
			}
		}
	}
}
#endif
void modal_midi_learn(Param param_index) {}

#endif // WITH_GUI

static intptr_t dispatcher(AEffect *effect, int opcode, int index, intptr_t val, void *ptr, float f)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

	switch (opcode) {
		case effOpen:
			return 0;

		case effClose:
			delete plugin;
			memset(effect, 0, sizeof(AEffect));
			free(effect);
			return 0;

		case effSetProgram: {
			auto &bank = PresetController::getPresetBanks().at(val / kPresetsPerBank);
			auto &preset = bank.presets[val % kPresetsPerBank];
			plugin->presetName = preset.getName();
			plugin->programNumber = val;
			plugin->synthesizer->_presetController->setCurrentPreset(preset);
			return 1;
		}

		case effGetProgram:
			return plugin->programNumber;

		case effGetProgramName:
			strncpy((char *)ptr, plugin->presetName.c_str(), 24);
			return 1;

		case effGetParamLabel:
			plugin->synthesizer->getParameterLabel((Param)index, (char *)ptr, 32);
			return 0;

		case effGetParamDisplay:
			plugin->synthesizer->getParameterDisplay((Param)index, (char *)ptr, 32);
			return 0;

		case effGetParamName:
			plugin->synthesizer->getParameterName((Param)index, (char *)ptr, 32);
			return 0;

		case effSetSampleRate:
			plugin->synthesizer->setSampleRate(f);
			return 0;

		case effSetBlockSize:
		case effMainsChanged:
			return 0;

#ifdef WITH_GUI
		case effEditGetRect: {
			static ERect rect = {0, 0, 400, 600};
			ERect **er = (ERect **)ptr;
			*er = &rect;
			return 1;
		}
		case effEditOpen: {
			static bool initialized = false;
			if (!initialized) {

				plugin->editorInstance = new ImguiEditor;

				initialized = true;
			}

			return 1;
		}
		case effEditClose: {
			delete plugin->editorInstance;
			plugin->editorInstance = nullptr;

			return 0;
		}
		case effEditIdle: {

			return 0;
		}
#endif

		case effGetChunk:
			return plugin->synthesizer->saveState((char **)ptr);

		case effSetChunk:
			plugin->synthesizer->loadState((char *)ptr);
			return 0;

		case effProcessEvents: {
			VstEvents *events = (VstEvents *)ptr;

#if _WIN32		// Do not assert on Windows
			plugin->midiEvents.empty();
#else
			assert(plugin->midiEvents.empty());
#endif

			memset(plugin->midiBuffer, 0, 4096);
			unsigned char *buffer = plugin->midiBuffer;

			for (int32_t i=0; i<events->numEvents; i++) {
				VstMidiEvent *event = (VstMidiEvent *)events->events[i];
				if (event->type != kVstMidiType) {
					continue;
				}
				
				int msgLength = 0;
				unsigned char statusByte = event->midiData[0];
				if (statusByte < MIDI_STATUS_NOTE_OFF) {
					continue; // Not a status byte
				}
				if (statusByte >= 0xF0) {
					continue; // Ignore system messages
				}
				switch (statusByte & 0xF0) {
				case MIDI_STATUS_PROGRAM_CHANGE:
				case MIDI_STATUS_CHANNEL_PRESSURE:
					msgLength = 2;
					break;
				default:
					msgLength = 3;
				}
				
				memcpy(buffer, event->midiData, msgLength);

				amsynth_midi_event_t midi_event;
				memset(&midi_event, 0, sizeof(midi_event));
				midi_event.offset_frames = event->deltaFrames;
				midi_event.buffer = buffer;
				midi_event.length = msgLength;
				plugin->midiEvents.push_back(midi_event);

				buffer += msgLength;

				assert(buffer < plugin->midiBuffer + 4096);
			}
			
			return 1;
		}

		case effCanBeAutomated:
		case effGetOutputProperties:
			return 0;

		case effGetPlugCategory:
			return kPlugCategSynth;
		case effGetEffectName:
			strcpy((char *)ptr, "amsynth");
			return 1;
		case effGetVendorString:
			strcpy((char *)ptr, "Nick Dowell");
			return 1;
		case effGetProductString:
			strcpy((char *)ptr, "amsynth");
			return 1;
		case effGetVendorVersion:
			return 0;

		case effCanDo:
			if (strcmp("receiveVstMidiEvent", (char *)ptr) == 0 ||
				false) return 1;
			if (strcmp("midiKeyBasedInstrumentControl", (char *)ptr) == 0 ||
				strcmp("midiSingleNoteTuningChange", (char *)ptr) == 0 ||
				strcmp("receiveVstSysexEvent", (char *)ptr) == 0 ||
				strcmp("sendVstMidiEvent", (char *)ptr) == 0 ||
				false) return 0;
#if DEBUG
			fprintf(logFile, "[amsynth_vst] unhandled canDo: %s\n", (char *)ptr);
			fflush(logFile);
#endif
			return 0;

		case effGetTailSize:
		case effIdle:
		case effGetParameterProperties:
			return 0;

		case effGetVstVersion:
			return 2400;

		case effGetMidiKeyName:
		case effStartProcess:
		case effStopProcess:
		case effBeginSetProgram:
		case effEndSetProgram:
		case effBeginLoadBank:
			return 0;

		default:
			return 0;
	}
}

static void process(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	std::vector<amsynth_midi_cc_t> midi_out;
	plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, midi_out, outputs[0], outputs[1]);
	plugin->midiEvents.clear();
}

static void processReplacing(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	std::vector<amsynth_midi_cc_t> midi_out;
	plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, midi_out, outputs[0], outputs[1]);
	plugin->midiEvents.clear();
}

static void setParameter(AEffect *effect, int i, float f)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	plugin->synthesizer->setNormalizedParameterValue((Param) i, f);
}

static float getParameter(AEffect *effect, int i)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	return plugin->synthesizer->getNormalizedParameterValue((Param) i);
}

static int getNumPrograms()
{
	return PresetController::getPresetBanks().size() * kPresetsPerBank;
}

extern "C"
#if _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
AEffect * VSTPluginMain(audioMasterCallback audioMaster)
{
	if (audioMaster) {
		audioMaster(nullptr, audioMasterGetProductString, 0, 0, hostProductString, 0.0f);
	}
	AEffect *effect = (AEffect *)calloc(1, sizeof(AEffect));
	effect->magic = kEffectMagic;
	effect->dispatcher = dispatcher;
	effect->process = process;
	effect->setParameter = setParameter;
	effect->getParameter = getParameter;
	effect->numPrograms = getNumPrograms();
	effect->numParams = kAmsynthParameterCount;
	effect->numInputs = 0;
	effect->numOutputs = 2;
	effect->flags = effFlagsCanReplacing | effFlagsIsSynth | effFlagsProgramChunks;
#ifdef WITH_GUI
#if _WIN32
	effect->flags |= effFlagsHasEditor;		// On Windows, amsynth's GTK GUI works in REAPER! (^.^)
#else
	if (strcmp("REAPER", hostProductString) == 0) {
		// amsynth's GTK GUI doesn't work in REAPER :-[
	} else {
		effect->flags |= effFlagsHasEditor;
	}
#endif // _WIN32
#endif // WITH_GUI
	// Do no use the ->user pointer because ardour clobbers it
	effect->ptr3 = new Plugin(audioMaster);
	effect->uniqueID = CCONST('a', 'm', 's', 'y');
	effect->processReplacing = processReplacing;
	return effect;
}

#if _WIN32

/** Entrance for MSVC */
extern "C"
AEffect * MAIN(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}

/** Entrance for CMake */
extern "C" {
__attribute__ ((visibility ("default")))
AEffect* wrap (audioMasterCallback audioMaster) asm ("main");

AEffect* wrap (audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}

}

#else

// this is required because GCC throws an error if we declare a non-standard function named 'main'
extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster) asm ("main");

extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}

#endif
