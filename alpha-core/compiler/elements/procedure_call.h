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

    class procedure_call : public element {
    public:
        procedure_call(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference* reference,
            compiler::argument_list* args);

        compiler::argument_list* arguments();

        compiler::identifier_reference* reference();

        void reference(compiler::identifier_reference* value);

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        compiler::argument_list* _arguments = nullptr;
        compiler::identifier_reference* _reference = nullptr;
    };

};

