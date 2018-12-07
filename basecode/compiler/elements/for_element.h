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

    class for_element : public element {
    public:
        for_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::declaration* induction_decl,
            compiler::element* expression,
            compiler::block* body);

        compiler::block* body();

        compiler::element* expression();

        compiler::declaration* induction_decl();

    protected:
        bool on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::block* _body = nullptr;
        compiler::element* _expression = nullptr;
        compiler::declaration* _induction_decl = nullptr;
    };

};

