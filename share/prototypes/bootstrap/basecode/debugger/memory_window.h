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

#include "window.h"

namespace basecode::debugger {

    class memory_window : public window {
    public:
        memory_window(
            window* parent,
            int x,
            int y,
            int width,
            int height);

        uint64_t address() const;

        void address(uint64_t value);

    protected:
        void on_draw(environment& env) override;

    private:
        uint64_t _address;
    };

};

