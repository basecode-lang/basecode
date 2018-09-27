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

#include "element.h"

namespace basecode::compiler {

    class with : public element {
    public:
        with(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference* ref,
            compiler::block* body);

        compiler::block* body();

        compiler::identifier_reference* ref();

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::block* _body = nullptr;
        compiler::identifier_reference* _ref = nullptr;
    };

};

