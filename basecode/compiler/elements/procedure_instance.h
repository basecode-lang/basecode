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

    class procedure_instance : public element {
    public:
        procedure_instance(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::procedure_type* procedure_type,
            compiler::block* scope);

        compiler::block* scope();

        compiler::procedure_type* procedure_type();

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::block* _scope = nullptr;
        compiler::procedure_type* _procedure_type = nullptr;
    };

};

