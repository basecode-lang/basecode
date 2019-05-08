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
#include "field.h"

namespace basecode::compiler {

    class procedure_type : public compiler::type {
    public:
        static std::string name_for_procedure_type();

        procedure_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* header_scope,
            compiler::symbol_element* symbol);

        bool prepare_call_site(
            compiler::session& session,
            bool uniform_function_call,
            compiler::argument_list* args,
            compiler::prepare_call_site_result_t& result) const;

        bool is_co() const;

        bool is_inline() const;

        void is_co(bool value);

        bool has_return() const;

        bool is_foreign() const;

        bool is_template() const;

        field_map_t& parameters();

        void is_inline(bool value);

        void is_foreign(bool value);

        void has_return(bool value);

        type_map_t& type_parameters();

        compiler::block* body_scope();

        compiler::block* header_scope();

        field_map_t& return_parameters();

        uint64_t foreign_address() const;

        bool is_proc_type() const override;

        void foreign_address(uint64_t value);

        compiler::procedure_type* instance_for(
            compiler::session& session,
            compiler::procedure_call* call);

        void body_scope(compiler::block* value);

        std::string label_name() const override;

    protected:
        bool on_type_check(
            compiler::type* other,
            const type_check_options_t& options) override;

        bool on_is_constant() const override;

        void on_owned_elements(element_list_t& list) override;

        bool on_initialize(compiler::session& session) override;

    private:
        bool _is_co = false;
        bool _is_inline = false;
        bool _has_return = false;
        bool _is_foreign = false;
        field_map_t _parameters {};
        uint64_t _foreign_address = 0;
        type_map_t _type_parameters {};
        field_map_t _return_parameters {};
        procedure_type_map_t* _instances {};
        compiler::block* _body_scope = nullptr;
        compiler::block* _header_scope = nullptr;
    };

}

