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

#include <compiler/session.h>
#include "composite_type.h"

namespace basecode::compiler {

    class array_type : public compiler::composite_type {
    public:
        static std::string name_for_array(
            compiler::type* entry_type,
            const element_list_t& subscripts);

        array_type(
            compiler::module* module,
            block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* base_type_ref,
            const element_list_t& subscripts);

        size_t data_size() const;

        compiler::element* replace(
            size_t index,
            compiler::element* item);

        size_t number_of_elements() const;

        int32_t find_index(common::id_t id);

        bool is_array_type() const override;

        bool are_subscripts_constant() const;

        const element_list_t& subscripts() const;

        compiler::type_reference* base_type_ref();

        compiler::element* find_subscript(common::id_t id);

    protected:
        bool on_type_check(
            compiler::type* other,
            const type_check_options_t& options) override;

        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        void on_owned_elements(element_list_t& list) override;

        bool on_initialize(compiler::session& session) override;

    private:
        element_list_t _subscripts {};
        compiler::type_reference* _base_type_ref = nullptr;
    };

}

