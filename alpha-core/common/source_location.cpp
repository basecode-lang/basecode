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

    source_location::source_location() {
    }

    source_location::source_location(
            uint32_t line,
            uint32_t start_column,
            uint32_t end_column) : _line(line),
                                   _end_column(end_column),
                                   _start_column(start_column) {
    }

    uint32_t source_location::line() const {
        return _line;
    }

    void source_location::line(uint32_t value) {
        _line = value;
    }

    uint32_t source_location::end_column() const {
        return _end_column;
    }

    uint32_t source_location::start_column() const {
        return _start_column;
    }

    void source_location::end_column(uint32_t value) {
        _end_column = value;
    }

    void source_location::start_column(uint32_t value) {
        _start_column = value;
    }

}