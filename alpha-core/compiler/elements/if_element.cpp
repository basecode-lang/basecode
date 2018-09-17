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

#include <fmt/format.h>
#include <compiler/session.h>
#include "if_element.h"

namespace basecode::compiler {

    if_element::if_element(
            compiler::module* module,
            block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch,
            bool is_else_if) : element(module, parent_scope, element_type_t::if_e),
                               _is_else_if(is_else_if),
                               _predicate(predicate),
                               _true_branch(true_branch),
                               _false_branch(false_branch) {
    }

    element* if_element::predicate() {
        return _predicate;
    }

    element* if_element::true_branch() {
        return _true_branch;
    }

    element* if_element::false_branch() {
        return _false_branch;
    }

    bool if_element::is_else_if() const {
        return _is_else_if;
    }

    bool if_element::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto true_label_name = fmt::format("{}_true", label_name());
        auto false_label_name = fmt::format("{}_false", label_name());
        auto end_label_name = fmt::format("{}_end", label_name());

        vm::register_t target_reg {
            .size = vm::op_sizes::byte,
            .type = vm::register_type_t::integer
        };
        assembler.allocate_reg(target_reg);
        defer({
            assembler.free_reg(target_reg);
        });

        assembler.push_target_register(target_reg);
        _predicate->emit(session);
        assembler.pop_target_register();

        block->bz(target_reg, assembler.make_label_ref(false_label_name));
        block->label(assembler.make_label(true_label_name));
        _true_branch->emit(session);
        block->jump_direct(assembler.make_label_ref(end_label_name));

        block->label(assembler.make_label(false_label_name));
        if (_false_branch != nullptr) {
            _false_branch->emit(session);
        }

        block->label(assembler.make_label(end_label_name));

        return true;
    }

    void if_element::on_owned_elements(element_list_t& list) {
        if (_predicate != nullptr)
            list.emplace_back(_predicate);
        if (_true_branch != nullptr)
            list.emplace_back(_true_branch);
        if (_false_branch != nullptr)
            list.emplace_back(_false_branch);
    }

};