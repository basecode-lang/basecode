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

#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "program.h"
#include "identifier.h"
#include "argument_list.h"

namespace basecode::compiler {

    argument_list::argument_list(
        compiler::module* module,
        block* parent_scope) : element(module, parent_scope, element_type_t::argument_list) {
    }

    bool argument_list::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();

        auto instruction_block = assembler.current_block();
        for (auto it = _elements.rbegin(); it != _elements.rend(); ++it) {
            element* arg = *it;
            switch (arg->element_type()) {
                case element_type_t::proc_call:
                case element_type_t::expression:
                case element_type_t::float_literal:
                case element_type_t::string_literal:
                case element_type_t::unary_operator:
                case element_type_t::binary_operator:
                case element_type_t::boolean_literal:
                case element_type_t::integer_literal:
                case element_type_t::identifier_reference: {
                    auto arg_reg = register_for(session, arg);
                    if (arg_reg.var != nullptr) {
                        arg_reg.clean_up = true;
                    }
                    assembler.push_target_register(arg_reg.reg);
                    arg->emit(session);
                    assembler.pop_target_register();
                    instruction_block->push(arg_reg.reg);
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

    int32_t argument_list::find_index(common::id_t id) {
        for (size_t i = 0; i < _elements.size(); i++) {
            if (_elements[i]->id() == id)
                return static_cast<int32_t>(i);
        }
        return -1;
    }

    const element_list_t& argument_list::elements() const {
        return _elements;
    }

    element* argument_list::replace(size_t index, element* item) {
        auto old = _elements[index];
        _elements[index] = item;
        return old;
    }

    void argument_list::on_owned_elements(element_list_t& list) {
        for (auto element : _elements)
            list.emplace_back(element);
    }

};