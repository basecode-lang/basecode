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

#include "element.h"

namespace basecode::compiler {

    class procedure_call : public element {
    public:
        procedure_call(
            compiler::element* parent,
            compiler::identifier* identifier,
            compiler::argument_list* args);

        compiler::identifier* identifier();

        compiler::argument_list* arguments();

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        compiler::argument_list* _arguments = nullptr;
        compiler::identifier* _identifier = nullptr;
    };

};

