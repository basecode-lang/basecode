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

        void reverse();

        size_t size() const;

        compiler::element* replace(
            size_t index,
            compiler::element* item);

        void remove(common::id_t id);

        bool index_to_procedure_type(
            compiler::session& session,
            compiler::procedure_type* proc_type);

        void add(compiler::element* item);

        int32_t find_index(common::id_t id);

        const element_list_t& elements() const;

        compiler::element* find(common::id_t id);

        compiler::element* param_at_index(size_t index);

        compiler::element* param_by_name(const std::string& name);

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element_list_t _elements {};
        std::map<std::string, size_t> _param_index {};
    };

};

