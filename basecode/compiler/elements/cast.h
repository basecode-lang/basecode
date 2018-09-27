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

    class cast : public element {
    public:
        cast(
            compiler::module* module,
            block* parent_scope,
            compiler::type_reference* type,
            element* expr);

        element* expression();

        compiler::type_reference* type();

        void type_location(const common::source_location& loc);

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        element* _expression = nullptr;
        common::source_location _type_location {};
        compiler::type_reference* _type_ref = nullptr;
    };

};
