/*
 *  amsynth_vst_imgui.cpp
 *
 *  Copyright (c) 2008-2020 Nick Dowell
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

#if _WIN32
#include <windows.h>
#endif

/** Default UI size */
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 540;

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

// From LSP Plugins (https://github.com/sadko4u/lsp-plugins)
#define effSetProgramName          4
#define effGetProgramNameIndexed   29

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
static void on_adjustment_value_changed(float values[], AEffect *effect)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

	static Preset dummyPreset;

	for (int i = 0; i < kAmsynthParameterCount; i++) {
		Parameter &param = dummyPreset.getParameter(i);
		param.setValue(values[i]);
		plugin->synthesizer->setParameterValue((Param)i, values[i]);
		if (plugin->audioMaster && !strstr(hostProductString, "Qtractor")) {
			plugin->audioMaster(effect, audioMasterAutomate, i, 0, nullptr, param.getNormalisedValue());
		}
	}
}

void modal_midi_learn(Param param_index) {}

#endif // WITH_GUI

void panic_on_dsp_side(AEffect *effect)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

    // Build MIDI data of All Sound Off
    unsigned char buffer[3] = {MIDI_STATUS_CONTROLLER, MIDI_CC_ALL_SOUND_OFF, 0}; // Buffer format: {status, data1, data2}

    // Directly apply MIDI data
    plugin->synthesizer->getMidiController()->HandleMidiData(buffer, 3);
}

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
			// Apply built-in program.
			// @param val: Target program index set by host.

			// Stop all sound before applying preset
			panic_on_dsp_side(effect);

			// Program #0 is reserved for default / user-defined presets.
			// Do not apply factory program at this time.
			if (val <= 0) {
				plugin->presetName = "Initial";
				plugin->programNumber = 0;
				return 1;
			}

			// Obtain bank and preset
			// The corresponding factory program index is just by one item before host's index.
			int actual_index = val >= 1 ? val - 1 : 0;	// In case of possible error
			auto &bank = PresetController::getPresetBanks().at(actual_index / kPresetsPerBank);
			auto &preset = bank.presets[actual_index % kPresetsPerBank];

			// Keep current preset name / index via our own variables
			plugin->presetName = preset.getName();
			plugin->programNumber = val;

			// Apply preset
			plugin->synthesizer->_presetController->setCurrentPreset(preset);
			return 1;
		}

		case effSetProgramName: {
			// Set current program name on plugin side
			// @param ptr: Pointer of host's current preset name

			plugin->presetName = std::string((char *)ptr);
			return 1;
		}

		case effGetProgramName: {
			// Set current program name on host when invoking effSetProgram
			// @param ptr: Pointer where host receives program name

			strncpy((char *)ptr, plugin->presetName.c_str(), 24);
			return 1;
		}

		case effGetProgram:
			// Get current program index.
			// Implement this so that host can mark which program we've set

			return plugin->programNumber;

		case effGetProgramNameIndexed: {
			// Prepare all the built-in program names.
			// Host will query every built-in program by invoking effGetProgramNameIndexed.
			// @param index: The program index where the host queries

			// Reserve program #0 as default/customized program as "Initial",
			// so that users can load user-defined presets from host instead of always being redirected to factory program #0.
			// NOTE: Hosts like REAPER will set index to 0 when loading user-defined presets.
			if (index <= 0) {
				strncpy((char *)ptr, "-- Init / User --", 24);
				return 1;
			}

			// Check if there were preset files or not
			// If you don't check, host will crash if no preset was installed
			if (PresetController::getPresetBanks().size()) {
				// As #0 is "Initial", other factory programs should be put into the next hole.
				// "index" is host's display index, and "actual_index" is the corresponding bank's index.
				int actual_index = index - 1;

				// Obtain bank and preset
				auto &bank = PresetController::getPresetBanks().at(actual_index / kPresetsPerBank);
				auto &preset = bank.presets[actual_index % kPresetsPerBank];

				// Set current obtained preset name, then show the name on host
				plugin->presetName = preset.getName();
				strncpy((char *)ptr, plugin->presetName.c_str(), 24);
				return 1;
			} else {
				// Do not continue accessing if no preset was found
				return 1;
			}
		}

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
			static ERect rect = {0, 0, WINDOW_HEIGHT, WINDOW_WIDTH};
			ERect **er = (ERect **)ptr;
			*er = &rect;
			return 1;
		}
		case effEditOpen: {
			if (!plugin->editorInstance) {
				plugin->editorInstance = new ImguiEditor(ptr, WINDOW_WIDTH, WINDOW_HEIGHT, plugin->synthesizer);
				plugin->editorInstance->setParamChangeCallback(on_adjustment_value_changed, effect);
				plugin->editorInstance->openEditor();
			}
			return 1;
		}
		case effEditClose: {
			if (plugin->editorInstance) {
				// FIXME: Maybe only invoke closeEditor() once can prevent assertion?
				//plugin->editorInstance->closeEditor();
				delete plugin->editorInstance;
				plugin->editorInstance = nullptr;
			}
			return 0;
		}
		case effEditIdle: {
			// Draw UI on every idle piece
			if (plugin->editorInstance) {
				plugin->editorInstance->drawFrame();
			}
			return 1;
		}
#endif

		case effGetChunk:
			return plugin->synthesizer->saveState((char **)ptr);

		case effSetChunk:
			plugin->synthesizer->loadState((char *)ptr);
			return 0;

		case effProcessEvents: {
			VstEvents *events = (VstEvents *)ptr;

#if !_WIN32		// Do not assert on Windows
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

				// FIXME: I don't know which bug will appear after bypassing this assertion.
				// In some hosts (e.g. Falcosoft Midi Player), this assertion can easily be triggered.
				//assert(buffer < plugin->midiBuffer + 4096);
			}
			
			return 1;
		}

		case effCanBeAutomated:
		case effGetOutputProperties:
			return 0;

		case effGetPlugCategory:
			return kPlugCategSynth;
		case effGetEffectName:
			strcpy((char *)ptr, "amsynth with ImGui");
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

#ifdef WITH_GUI
	// Prepare samples for editor's oscilloscope
	// NOTE: processReplacing() is the real function where I can get sample data,
	//       not process().
	if (plugin->editorInstance)
	{
		plugin->editorInstance->setCurrentSample(numSampleFrames, outputs[0]);
	}
#endif
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
	effect->flags |= effFlagsHasEditor;		// On Windows, amsynth's GTK GUI works in REAPER! (^.^)
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
