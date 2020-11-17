// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//      L A N G U A G E   P R O J E C T
//
// Copyright (C) 2018-2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/memory.h>

namespace basecode {
    struct compiler_t final {
        alloc_t*                alloc;
    };

    namespace compiler {
        enum class status_t : u32 {
            ok,
            error
        };

        u0 free(compiler_t& compiler);

        status_t init(compiler_t& compiler, alloc_t* alloc = context::top()->alloc);
    }
}
