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

    class argument_list : public element {
    public:
        argument_list(
            compiler::module* module,
            compiler::block* parent_scope);

        void clear();

        size_t size() const;

        bool as_ffi_arguments(
            compiler::session& session,
            vm::function_value_list_t& args) const;

        compiler::element* replace(
            size_t index,
            compiler::element* item);

        bool is_foreign_call() const;

        void remove(common::id_t id);

        uint64_t allocated_size() const;

        void allocated_size(size_t size);

        void is_foreign_call(bool value);

        void add(compiler::element* item);

        int32_t find_index(common::id_t id);

        const element_list_t& elements() const;

        compiler::element* find(common::id_t id);

        void elements(const element_list_t& value);

        compiler::element* param_at_index(size_t index);

        compiler::argument_list* variadic_arguments() const;

        void argument_index(const argument_index_map_t& value);

        compiler::element* param_by_name(const std::string& name);

    protected:
        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        bool recurse_ffi_arguments(
            compiler::session& session,
            const element_list_t& elements,
            vm::function_value_list_t& args) const;

    private:
        uint64_t _allocated_size = 0;
        bool _is_foreign_call = false;
        compiler::element_list_t _elements {};
        argument_index_map_t _argument_index {};
    };

}

