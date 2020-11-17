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

#include "directive.h"

namespace basecode::compiler {

    class type_directive : public directive {
    public:
        type_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression);

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _expression = nullptr;
    };

};

