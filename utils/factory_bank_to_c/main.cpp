/*
 *  factory_bank_to_c main.cpp: Factory bank converter
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

#include <algorithm>
#include <string>
#include <vector>

#include "binary_to_compressed_c.hpp"

#ifndef _WIN32
#include <dirent.h>
#include <unistd.h>
#else
#include <io.h>
#endif

struct BankInfo_Converter {
    ~BankInfo_Converter() { }

    std::string name;
    std::string file_path;
    bool read_only;

    std::string file_array_name;
    int file_array_size;

    bool success;
};

const char* BANK_CODE_NAME = "amsynth_factory_banks";

static std::vector<BankInfo_Converter> s_banks;

static const char amsynth_file_header[] = { 'a', 'm', 'S', 'y', 'n', 't', 'h', '\n' };

static bool is_amsynth_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file)
        return false;

    char buffer[sizeof(amsynth_file_header)] = { 0 };
    size_t count = fread(buffer, sizeof(buffer), 1, file);
    fclose(file);

    if (count != 1)
        return false;

    if (memcmp(buffer, amsynth_file_header, sizeof(amsynth_file_header)) != 0)
        return false;

    return true;
}

static void scan_preset_bank(const std::string dir_path, const std::string file_name, bool read_only)
{
    std::string file_path = dir_path + std::string("/") + std::string(file_name);

    std::string bank_name = std::string(file_name);
    if (bank_name == "default") {
        bank_name = "User bank";
    } else {
        std::string::size_type pos = bank_name.find_first_of(".");
        if (pos != std::string::npos)
            bank_name.erase(pos, std::string::npos);
    }

    std::replace(bank_name.begin(), bank_name.end(), '_', ' ');

    if (!is_amsynth_file(file_path.c_str()))
        return;

    BankInfo_Converter bank_info;
    bank_info.name = bank_name;
    bank_info.file_path = file_path;
    bank_info.read_only = read_only;

    s_banks.push_back(bank_info);
}

static void scan_preset_banks(const std::string dir_path, bool read_only)
{
    std::vector<std::string> filenames;

#if _WIN32
    /** Implement preset scanner on Windows
     * Reference: https://www.cnblogs.com/collectionne/p/6815924.html
     */
    intptr_t handle;
    _finddata_t findData;
    const std::string dir_path_for_search(dir_path + "\\*.*");

    handle = _findfirst(dir_path_for_search.c_str(), &findData); // Find the first file
    if (handle == -1)
        return;

    do {
        if (findData.attrib & _A_SUBDIR
            && strcmp(findData.name, ".") == 0
            && strcmp(findData.name, "..") == 0) // If sub directory or "."/".." detected, ignore
            continue;
        else
            filenames.push_back(std::string(findData.name));
    } while (_findnext(handle, &findData) == 0); // Find next file

    _findclose(handle); // Close search handle
#else
    DIR* dir = opendir(dir_path.c_str());
    if (!dir)
        return;

    struct dirent* entry;
    while ((entry = readdir(dir)))
        filenames.push_back(std::string(entry->d_name));

    closedir(dir);
#endif

    std::sort(filenames.begin(), filenames.end());

    for (auto& filename : filenames)
        scan_preset_bank(dir_path, filename, read_only);
}

static void scan_preset_banks(const std::string scan_path)
{
    s_banks.clear();

    if (!scan_path.empty()) {
        scan_preset_banks(scan_path, true);
    }
}

bool start_convert(const std::string bank_directory, const std::string output_dir)
{
    scan_preset_banks(bank_directory);

    std::stringstream bank_array_code; // C++ code of bank file encoded in an array
    std::string bank_array_symbol; // Generated C++ symbol of bank file array
    int bank_array_size = 0; // Array size of bank file array

    // Variables for generating bank file array's symbol
    char symbol[32];
    int symbol_index = 0;

    std::stringstream output_header_path, output_cpp_path;
    output_header_path << output_dir << "/" << BANK_CODE_NAME << ".h";
    output_cpp_path << output_dir << "/" << BANK_CODE_NAME << ".cpp";

    FILE* out_h = fopen(output_header_path.str().c_str(), "w");
    FILE* out_cpp = fopen(output_cpp_path.str().c_str(), "w");

    const char* reminder = "// Auto generated by factory_bank_to_c utility. DO NOT EDIT.\n\n";
    fprintf(out_h, "%s", reminder);
    fprintf(out_cpp, "%s", reminder);

    // fprintf(stderr, "* Factory bank path: %s\n", bank_directory.c_str());
    fprintf(stderr, "* Converting bank file:\n");

    for (auto iter = s_banks.begin(); iter != s_banks.end(); iter++) {
        sprintf_s(symbol, "bank_%d", symbol_index++);

        bool res = binary_to_compressed_c_local(iter->file_path.c_str(), symbol, bank_array_code, bank_array_size, bank_array_symbol);
        if (res) {
            fprintf(out_h, "%s\n", bank_array_code.str().c_str());
            fprintf(stderr, "    - [ OK ] %s\n", iter->file_path.c_str());

            iter->success = true;
        } else {
            fprintf(stderr, "    - [FAIL] %s\n", iter->file_path.c_str());

            fprintf(out_h, "// WARNING: Bank file \"%s\" is failed to read\n", iter->name.c_str());
            fprintf(out_h, "//          (%s)\n\n", iter->file_path.c_str());

            iter->success = false;
        }

        iter->file_array_name = bank_array_symbol;
        iter->file_array_size = bank_array_size;
    }

    fprintf(stderr, "* Writing C++ initializer function InitializeEmbedFactoryBanks() ...");

    fprintf(out_cpp, "#include <EmbedPresetController.h>\n");
    fprintf(out_cpp, "#include \"%s.h\"\n\n", BANK_CODE_NAME);

    fprintf(out_cpp, "void InitializeEmbedFactoryBanks(std::vector<EmbedBankInfo> &factory_banks_list) {\n");
    fprintf(out_cpp, "    factory_banks_list.clear();\n\n");

    symbol_index = 0;
    for (auto iter = s_banks.begin(); iter != s_banks.end(); iter++) {
        if (iter->success) {
            sprintf_s(symbol, "bank_%d", symbol_index++);

            fprintf(out_cpp, "    EmbedBankInfo %s = {\n", symbol);
            fprintf(out_cpp, "        .name = \"%s\",\n", iter->name.c_str());
            fprintf(out_cpp, "        .file_path = \"%s\",\n", iter->file_path.c_str());
            fprintf(out_cpp, "        .read_only = %s,\n", iter->read_only ? "true" : "false");
            fprintf(out_cpp, "        .file_array = %s,\n", iter->file_array_name.c_str());
            fprintf(out_cpp, "        .file_array_size = %d,\n", iter->file_array_size);
            fprintf(out_cpp, "    };\n");

            fprintf(out_cpp, "    factory_banks_list.push_back(%s);\n\n", symbol);
        } else {
            fprintf(out_cpp, "// WARNING: Bank file \"%s\" is failed to read\n", iter->name.c_str());
            fprintf(out_cpp, "//          (%s)\n\n", iter->file_path.c_str());
        }
    }

    fprintf(out_cpp, "}\n");

    fprintf(stderr, " Success!\n");

    fclose(out_cpp);
    fclose(out_h);

    fprintf(stderr, "* Output C++ source files are:\n    - %s\n    - %s\n", output_header_path.str().c_str(), output_cpp_path.str().c_str());

    return true;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Syntax: %s <bank_directory> <output_dir>\n", argv[0]);
        return 1;
    }

    int argn = 1;
    bool ret = start_convert(argv[argn], argv[argn + 1]);
    if (!ret) {
        fprintf(stderr, "Error opening or reading file: '%s'\n", argv[argn]);
    }

    return ret ? 0 : 1;
}
