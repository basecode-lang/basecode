// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <basecode/types.h>
#include <basecode/result.h>

namespace basecode::io::text {

    bool read(
        result_t& r,
        const path_t& path,
        std::iostream& stream);

    bool write(
        result_t& r,
        const path_t& path,
        const std::iostream& stream);

    bool write(
        result_t& r,
        const path_t& path,
        const string_t& text);

}