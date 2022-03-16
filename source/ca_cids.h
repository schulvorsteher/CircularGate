//------------------------------------------------------------------------
// Copyright(c) 2022 schulvorsteher.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace csse {
//------------------------------------------------------------------------
static const Steinberg::FUID kCircularGateProcessorUID (0x2E5F7381, 0x1CDD56E5, 0xA1D88B19, 0x6E7072F6);
static const Steinberg::FUID kCircularGateControllerUID (0xC2B4909B, 0x8FF056BB, 0xA2FEA9F7, 0xF5B380BC);

#define CircularGateVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace csse
