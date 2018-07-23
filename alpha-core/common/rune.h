// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace basecode::common {

    using rune_t = int32_t;

    static constexpr rune_t rune_invalid = 0xfffd;
    static constexpr rune_t rune_max     = 0x0010ffff;
    static constexpr rune_t rune_bom     = 0xfeff;
    static constexpr rune_t rune_eof     = -1;

};