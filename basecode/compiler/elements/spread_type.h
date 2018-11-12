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
#include "type_reference.h"

namespace basecode::compiler {

    class spread_type : public compiler::type {
    public:
        spread_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* type);

        compiler::type_reference* type();

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::type_reference* _type_ref = nullptr;
    };

};

