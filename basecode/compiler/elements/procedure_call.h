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
            compiler::argument_list* args,
            compiler::type_reference_list_t type_params,
            compiler::identifier_reference_list_t identifier_references);

        bool is_inline() const;

        bool is_foreign() const;

        bool uniform_function_call() const;

        compiler::argument_list* arguments();

        void uniform_function_call(bool value);

        compiler::procedure_type* procedure_type();

        compiler::identifier_reference* identifier();

        bool resolve_overloads(compiler::session& session);

        const compiler::type_reference_list_t& type_parameters() const;

        const compiler::identifier_reference_list_t& references() const;

        void references(const compiler::identifier_reference_list_t& refs);

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        bool _uniform_function_call = false;
        compiler::argument_list* _arguments = nullptr;
        compiler::type_reference_list_t _type_parameters {};
        compiler::identifier_reference_list_t _references {};
        compiler::procedure_type* _resolved_proc_type = nullptr;
        compiler::identifier_reference* _resolved_identifier_ref = nullptr;
    };

}

