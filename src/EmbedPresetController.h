/*
 *  EmbedPresetController.h
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

#ifndef _EMBEDPRESETCONTROLLER_H
#define _EMBEDPRESETCONTROLLER_H

#include <string>
#include <vector>
#include "PresetController.h"

struct EmbedBankInfo {
    ~EmbedBankInfo() { }

    std::string name;
    std::string file_path;
    bool read_only;

    const unsigned int *file_array;
    int file_array_size;
};

static std::vector<EmbedBankInfo> embed_factory_banks;
void InitializeEmbedFactoryBanks(std::vector<EmbedBankInfo> &factory_banks_list);

static std::vector<BankInfo> s_factory_banks;

class EmbedPresetController : public PresetController
{
public:
    EmbedPresetController();
	~EmbedPresetController();

    static const std::vector<BankInfo> & getPresetBanks();
	static void rescanPresetBanks();
};

#endif  // defined _EMBEDPRESETCONTROLLER_H
