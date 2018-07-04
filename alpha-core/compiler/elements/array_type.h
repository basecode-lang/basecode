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

#include "type.h"

namespace basecode::compiler {

    class array_type : public compiler::type {
    public:
        array_type(
            element* parent,
            const std::string& name,
            compiler::type* entry_type);

        uint64_t size() const;

        void size(uint64_t value);

        compiler::type* entry_type();

    protected:
        bool on_initialize(common::result& r) override;

    private:
        uint64_t _size = 0;
        compiler::type* _entry_type = nullptr;
    };

};

