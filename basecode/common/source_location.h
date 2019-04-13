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

    struct location_t {
        uint32_t line = 0;
        uint32_t column = 0;
    };

    class source_location {
    public:
        const location_t& end() const;

        const location_t& start() const;

        void end(const location_t& value);

        void start(const location_t& value);

        void end(uint32_t line, uint32_t column);

        void start(uint32_t line, uint32_t column);

    private:
        location_t _end {};
        location_t _start {};
    };

}

