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

    class module_reference : public element {
    public:
        module_reference(
            compiler::block* parent_scope,
            compiler::element* expr);

        compiler::module* module();

        compiler::element* expression();

        void module(compiler::module* value);

    protected:
        bool on_is_constant() const override;

        void on_owned_elements(element_list_t& list) override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        compiler::module* _module = nullptr;
        compiler::element* _expression = nullptr;
    };

};

