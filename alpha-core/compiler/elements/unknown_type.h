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

#include "type.h"

namespace basecode::compiler {

    class unknown_type : public compiler::type {
    public:
        unknown_type(
            block* parent_scope,
            const std::string& name);

        bool is_array() const;

        void is_array(bool value);

        size_t array_size() const;

        void array_size(size_t value);

    protected:
        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

    private:
        size_t _array_size = 0;
        bool _is_array = false;
    };

};

