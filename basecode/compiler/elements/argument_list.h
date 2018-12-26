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

        compiler::element* replace(
            size_t index,
            compiler::element* item);

        bool is_foreign_call() const;

        void remove(common::id_t id);

        bool index_to_procedure_type(
            compiler::session& session,
            compiler::procedure_type* proc_type);

        uint64_t allocated_size() const;

        void add(compiler::element* item);

        int32_t find_index(common::id_t id);

        const element_list_t& elements() const;

        compiler::element* find(common::id_t id);

        compiler::element* param_at_index(size_t index);

        compiler::argument_list* variadic_arguments() const;

        compiler::element* param_by_name(const std::string& name);

    protected:
        bool on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) override;

        bool emit_elements(
            compiler::session& session,
            vm::instruction_block* block,
            const compiler::element_list_t& elements);

        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        uint64_t _allocated_size = 0;
        bool _is_foreign_call = false;
        compiler::element_list_t _elements {};
        std::map<std::string, size_t> _param_index {};
    };

};

