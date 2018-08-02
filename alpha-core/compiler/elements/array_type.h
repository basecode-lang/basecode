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

#include "composite_type.h"

namespace basecode::compiler {

    class array_type : public compiler::composite_type {
    public:
        static std::string name_for_array(
            compiler::type* entry_type,
            size_t size);

        array_type(
            block* parent_scope,
            compiler::block* scope,
            compiler::type* entry_type,
            size_t size);

        uint64_t size() const;

        void size(uint64_t value);

        compiler::type* entry_type();

    protected:
        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

        type_access_model_t on_access_model() const override;

    private:
        uint64_t _size = 0;
        compiler::type* _entry_type = nullptr;
    };

};

