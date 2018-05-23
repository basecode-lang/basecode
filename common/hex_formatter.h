// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

namespace basecode::common {

    class hex_formatter {
    public:
        static std::string dump_to_string(
            const void* data,
            size_t size);
    };

};

