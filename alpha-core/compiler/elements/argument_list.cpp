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

#include <vm/instruction_block.h>
#include "identifier.h"
#include "argument_list.h"

namespace basecode::compiler {

    argument_list::argument_list(
        element* parent) : element(parent, element_type_t::argument_list) {
    }

    bool argument_list::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        for (auto arg : _elements) {
            switch (arg->element_type()) {
                case element_type_t::proc_call:
                case element_type_t::expression:
                case element_type_t::identifier:
                case element_type_t::float_literal:
                case element_type_t::string_literal:
                case element_type_t::unary_operator:
                case element_type_t::binary_operator:
                case element_type_t::boolean_literal:
                case element_type_t::integer_literal: {
                    vm::i_registers_t target_reg;
                    if (!instruction_block->allocate_reg(target_reg)) {
                        // XXX: error
                    }

                    instruction_block->push_target_register(target_reg);
                    arg->emit(r, context);
                    instruction_block->pop_target_register();
                    instruction_block->push_u64(target_reg);
                    instruction_block->free_reg(target_reg);
                    break;
                }
                default:
                    break;
            }
        }
        return true;
    }

    void argument_list::add(element* item) {
        _elements.emplace_back(item);
    }

    void argument_list::remove(common::id_t id) {
        auto item = find(id);
        if (item == nullptr)
            return;
        std::remove(
            _elements.begin(),
            _elements.end(),
            item);
    }

    element* argument_list::find(common::id_t id) {
        auto it = std::find_if(
            _elements.begin(),
            _elements.end(),
            [&id](auto item) { return item->id() == id; });
        if (it == _elements.end())
            return nullptr;
        return *it;
    }

    const element_list_t& argument_list::elements() const {
        return _elements;
    }

};