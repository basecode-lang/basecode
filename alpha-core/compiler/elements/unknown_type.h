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
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* symbol);

        bool is_array() const;

        bool is_pointer() const;

        void is_array(bool value);

        void is_pointer(bool value);

        const element_list_t& subscripts() const;

        void subscripts(const element_list_t& subscripts);

    protected:
        bool on_initialize(compiler::session& session) override;

    private:
        bool _is_array = false;
        bool _is_pointer = false;
        element_list_t _subscripts {};
    };

};

