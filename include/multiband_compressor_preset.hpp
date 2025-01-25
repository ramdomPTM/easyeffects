/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <sys/types.h>
#include <nlohmann/json_fwd.hpp>
#include "plugin_preset_base.hpp"
#include "preset_type.hpp"
#include "tags_multiband_compressor.hpp"

class MultibandCompressorPreset : public PluginPresetBase {
 public:
  explicit MultibandCompressorPreset(PresetType preset_type, const int& index = 0);

 private:
  static constexpr uint n_bands = tags::multiband_compressor::n_bands;

  void save(nlohmann::json& json) override;

  void load(const nlohmann::json& json) override;
};
