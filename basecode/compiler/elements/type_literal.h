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

    class type_literal : public element {
    public:
        type_literal(
            compiler::module* module,
            compiler::block* parent_scope,
            type_literal_type_t type,
            compiler::argument_list* args,
            const compiler::type_reference_list_t& type_params = {},
            const compiler::element_list_t& subscripts = {});

        compiler::argument_list* args();

        type_literal_type_t type() const;

        const compiler::element_list_t& subscripts() const;

        const compiler::type_reference_list_t& type_params() const;

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_emit(compiler::session& session) override;

    private:
        type_literal_type_t _type;
        compiler::element_list_t _subscripts {};
        compiler::argument_list* _args = nullptr;
        compiler::type_reference_list_t _type_params {};
    };

};

