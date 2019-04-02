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

    class intrinsic  : public element {
    public:
        static bool register_intrinsic_procedure_type(
            const std::string& name,
            compiler::procedure_type* procedure_type);

        static intrinsic* intrinsic_for_call(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const qualified_symbol_t& symbol,
            const compiler::type_reference_list_t& type_params);

        intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params);

        virtual bool can_fold() const;

        bool uniform_function_call() const;

        virtual intrinsic_type_t type() const;

        void uniform_function_call(bool value);

        compiler::argument_list* arguments() const;

        compiler::procedure_type* procedure_type();

        const compiler::type_reference_list_t& type_parameters() const;

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        bool _uniform_function_call = false;
        compiler::argument_list* _arguments = nullptr;
        compiler::procedure_type* _procedure_type = nullptr;
        compiler::type_reference_list_t _type_parameters {};
    };

}

