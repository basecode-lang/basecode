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

#include "source_location.h"

namespace basecode::common {

    const location_t& source_location::end() const {
        return _end;
    }

    const location_t& source_location::start() const {
        return _start;
    }

    void source_location::end(const location_t& value) {
        _end = value;
    }

    void source_location::start(const location_t& value) {
        _start = value;
    }

    void source_location::end(uint32_t line, uint32_t column) {
        _end.line = line;
        _end.column = column;
    }

    void source_location::start(uint32_t line, uint32_t column) {
        _start.line = line;
        _start.column = column;
    }

}