/*
 *  EmbedPresetController.cpp
 *
 *  Copyright (C) 2022-2023 AnClark Liu
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

#include "EmbedPresetController.h"
#include "stb_decompress.h"

#include <cstring>

EmbedPresetController::EmbedPresetController()
    : PresetController()
{
}

EmbedPresetController::~EmbedPresetController()
{
}

void decode_compressed_file_array(const void* compressed_file_data, const int compressed_file_size, std::string& decompressed_file_data)
{
    const unsigned int buf_decompressed_size = amsynth_stb::stb_decompress_length((const unsigned char*)compressed_file_data);
    unsigned char* buf_decompressed_data = (unsigned char*)malloc(buf_decompressed_size);
    amsynth_stb::stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_file_data, (unsigned int)compressed_file_size);

    std::stringstream output_builder;
    output_builder << buf_decompressed_data;
    decompressed_file_data = output_builder.str();
}

static const char amsynth_file_header[] = { 'a', 'm', 'S', 'y', 'n', 't', 'h', '\n' };

static float float_from_string(const char* s)
{
    if (strchr(s, 'e'))
        return Parameter::valueFromString(std::string(s));
    float rez = 0, fact = 1;
    if (*s == '-') {
        s++;
        fact = -1;
    };
    for (int point_seen = 0; *s; s++) {
        if (*s == '.') {
            point_seen = 1;
            continue;
        };
        int d = *s - '0';
        if (d >= 0 && d <= 9) {
            if (point_seen)
                fact /= 10.0f;
            rez = rez * 10.0f + (float)d;
        };
    };
    return rez * fact;
}

static bool readBankFile(const std::string& file_content, Preset* presets)
{
    const char* buffer = file_content.c_str();

    off_t buffer_length = file_content.length();
    if (!buffer)
        return false;

    if (memcmp(buffer, amsynth_file_header, sizeof(amsynth_file_header)) != 0) {
        return false;
    }

    char* buffer_end = ((char*)buffer) + buffer_length;

    int preset_index = -1;
    char* line_ptr = (char*)buffer + sizeof(amsynth_file_header);
    for (char* end_ptr = line_ptr; end_ptr < buffer_end && *end_ptr; end_ptr++) {
        if (*end_ptr == '\n') {
            *end_ptr = '\0';
            end_ptr++;

            static char preset_prefix[] = "<preset> <name> ";
            if (strncmp(line_ptr, preset_prefix, sizeof(preset_prefix) - 1) == 0) {
                presets[++preset_index] = Preset(std::string(line_ptr + sizeof(preset_prefix) - 1));
            }

            static char parameter_prefix[] = "<parameter> ";
            if (strncmp(line_ptr, parameter_prefix, sizeof(parameter_prefix) - 1) == 0) {
                char* ptr = line_ptr + sizeof(parameter_prefix) - 1;
                char* sep = strchr(ptr, ' ');
                if (sep) {
                    Preset& preset = presets[preset_index];
                    Parameter& param = preset.getParameter(std::string(ptr, sep - ptr));
                    float fval = float_from_string(sep + 1);
                    param.setValue(fval);
                }
            }

            line_ptr = end_ptr;
        }
    }
    for (preset_index++; preset_index < PresetController::kNumPresets; preset_index++)
        presets[preset_index] = Preset();

    return true;
}

void load_embed_preset_banks()
{
    std::string decoded_bank_data;

    for (auto iter = embed_factory_banks.begin(); iter != embed_factory_banks.end(); iter++) {
        BankInfo bank_info;
        bank_info.name = iter->name;
        bank_info.file_path = iter->file_path;
        bank_info.read_only = iter->read_only;

        decode_compressed_file_array(iter->file_array, iter->file_array_size, decoded_bank_data);
        readBankFile(decoded_bank_data, bank_info.presets);

        s_factory_banks.push_back(bank_info);
    }
}

const std::vector<BankInfo>& EmbedPresetController::getPresetBanks()
{
    if (embed_factory_banks.empty()) {
        InitializeEmbedFactoryBanks(embed_factory_banks);
    }

    if (s_factory_banks.empty())
        load_embed_preset_banks();

    return s_factory_banks;
}
