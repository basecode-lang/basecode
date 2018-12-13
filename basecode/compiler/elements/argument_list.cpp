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
#include "initializer.h"
#include "declaration.h"
#include "argument_pair.h"
#include "argument_list.h"
#include "composite_type.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "type_reference.h"
#include "binary_operator.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    argument_list::argument_list(
            compiler::module* module,
            compiler::block* parent_scope) : element(module,
                                                     parent_scope,
                                                     element_type_t::argument_list) {
    }

    bool argument_list::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();
        return emit_elements(session, block, _elements);
    }

    void argument_list::clear() {
        _elements.clear();
        _param_index.clear();
    }

    size_t argument_list::size() const {
        auto size = _elements.size();
        auto variadic_args = variadic_arguments();
        if (variadic_args != nullptr)
            size += (variadic_args->size() - 1);
        return size;
    }

    bool argument_list::emit_elements(
            compiler::session& session,
            vm::instruction_block* block,
            const compiler::element_list_t& elements) {
        for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
            element* arg = *it;
            switch (arg->element_type()) {
                case element_type_t::argument_list: {
                    auto result = emit_elements(
                        session,
                        block,
                        dynamic_cast<compiler::argument_list*>(arg)->_elements);
                    if (!result)
                        return false;
                    break;
                }
                case element_type_t::proc_call:
                case element_type_t::expression:
                case element_type_t::float_literal:
                case element_type_t::string_literal:
                case element_type_t::unary_operator:
                case element_type_t::assembly_label:
                case element_type_t::binary_operator:
                case element_type_t::boolean_literal:
                case element_type_t::integer_literal:
                case element_type_t::character_literal:
                case element_type_t::identifier_reference: {
                    variable_handle_t arg_var;
                    if (!session.variable(arg, arg_var))
                        return false;

                    auto type = arg_var->type_result().inferred_type;
                    if (!type->is_composite_type()
                    ||   arg->element_type() == element_type_t::string_literal) {
                        arg_var->read();
                        block->push(arg_var->emit_result().operands.back());
                    } else {
                        arg_var->address();
                        block->push(vm::instruction_operand_t(arg_var->address_reg()));
//                        vm::register_t temp {};
//                        temp.type = vm::register_type_t::integer;
//                        session.assembler().allocate_reg(temp);
//                        defer(session.assembler().free_reg(temp));
//
//                        auto size = static_cast<uint64_t>(type->size_in_bytes());
//                        block->move(
//                            vm::instruction_operand_t(temp),
//                            vm::instruction_operand_t::sp());
//                        block->sub(
//                            vm::instruction_operand_t::sp(),
//                            vm::instruction_operand_t::sp(),
//                            vm::instruction_operand_t(size, vm::op_sizes::word));
//                        block->copy(
//                            vm::op_sizes::byte,
//                            vm::instruction_operand_t(temp),
//                            vm::instruction_operand_t(arg_var->address_reg()),
//                            vm::instruction_operand_t(size, vm::op_sizes::word));
                    }
                    break;
                }
                default:
                    break;
            }
        }

        return true;
    }

    compiler::element* argument_list::replace(
            size_t index,
            compiler::element* item) {
        auto old = _elements[index];
        _elements[index] = item;
        return old;
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

    // XXX: type checking will need to be deferred if one of the
    //      types is unknown
    bool argument_list::index_to_procedure_type(
            compiler::session& session,
            compiler::procedure_type* proc_type) {
        _param_index.clear();

        auto& param_map = proc_type->parameters();
        auto field_list = param_map.as_list();
        if (field_list.empty()) {
            if (!_elements.empty()) {
                session.error(
                    this,
                    "P019",
                    "procedure declares no parameters.",
                    parent_element()->location());
                return false;
            }
            return true;
        }

        auto& builder = session.builder();

        if (_elements.size() < field_list.size()) {
            _elements.resize(field_list.size());
        }

        element_list_t temp {};
        temp.resize(field_list.size());
        compiler::argument_list* variadic_args = nullptr;

        size_t index = 0;
        for (index = 0; index < field_list.size(); index++) {
            auto fld = field_list[index];
            auto fld_name = fld->identifier()->symbol()->name();
            if (fld->is_variadic()) {
                if (index < field_list.size() - 1) {
                    session.error(
                        this,
                        "P019",
                        fmt::format(
                            "variadic parameter only valid in final position: {}",
                            fld_name),
                        parent_element()->location());
                    return false;
                } else {
                    variadic_args = builder.make_argument_list(parent_scope());
                    temp[index] = variadic_args;
                }
            }
            _param_index.insert(std::make_pair(fld_name, index));
        }

        auto last_field = field_list.back();
        for (index = 0; index < _elements.size(); index++) {
        _retry:
            auto arg = _elements[index];
            if (arg == nullptr)
                continue;

            if (last_field->is_variadic()
            &&  index >= field_list.size() - 1) {
                if (variadic_args == nullptr) {
                    session.error(
                        this,
                        "P019",
                        "no variadic parameter defined.",
                        parent_element()->location());
                    return false;
                }

                variadic_args->add(arg);
                if (_elements.size() == field_list.size()) {
                    _elements[index] = nullptr;
                } else {
                    if (index < _elements.size()) {
                        _elements.erase(_elements.begin() + index);
                    } else {
                        continue;
                    }
                }
                goto _retry;
            } else {
                if (arg->element_type() != element_type_t::argument_pair)
                    continue;

                auto arg_pair = dynamic_cast<compiler::argument_pair*>(arg);
                std::string key;
                if (arg_pair->lhs()->as_string(key)) {
                    auto it = _param_index.find(key);
                    if (it != _param_index.end()) {
                        temp[it->second] = arg_pair->rhs();
                        _elements.erase(_elements.begin() + index);
                        _elements.emplace_back(nullptr);
                        goto _retry;
                    } else {
                        session.error(
                            this,
                            "P019",
                            fmt::format("invalid procedure parameter: {}", key),
                            arg->location());
                        return false;
                    }
                }
            }
        }

        index = 0;
        for (auto fld : field_list) {
            auto param = _elements[index];
            if (param != nullptr) {
                infer_type_result_t type_result{};
                if (!param->infer_type(session, type_result)) {
                    session.error(
                        this,
                        "P019",
                        fmt::format(
                            "unable to infer type for parameter: {}",
                            fld->identifier()->symbol()->name()),
                        param->location());
                    return false;
                }

                auto type_ref = fld->identifier()->type_ref();
                if (!type_ref->type()->type_check(type_result.inferred_type)) {
                    session.error(
                        this,
                        "C051",
                        fmt::format(
                            "type mismatch: cannot assign {} to parameter {}.",
                            type_result.type_name(),
                            fld->identifier()->symbol()->name()),
                        param->location());
                    return false;
                }
            } else {
                if (temp[index] != nullptr) {
                    _elements[index] = temp[index];
                    goto _next;
                }

                auto init = fld->identifier()->initializer();
                if (init == nullptr) {
                    auto decl = fld->declaration();
                    if (decl != nullptr) {
                        auto assignment = decl->assignment();
                        if (assignment != nullptr) {
                            _elements[index] = assignment->rhs();
                            goto _next;
                        }
                    }

                    session.error(
                        this,
                        "P019",
                        fmt::format(
                            "missing required parameter: {}",
                            fld->identifier()->symbol()->name()),
                        parent_element()->location());
                    return false;
                }

                _elements[index] = init->expression();
            }

        _next:
            index++;
        }

        return true;
    }

    void argument_list::add(compiler::element* item) {
        _elements.emplace_back(item);
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

    compiler::element* argument_list::find(common::id_t id) {
        auto it = std::find_if(
            _elements.begin(),
            _elements.end(),
            [&id](auto item) { return item->id() == id; });
        if (it == _elements.end())
            return nullptr;
        return *it;
    }

    void argument_list::on_owned_elements(element_list_t& list) {
        for (auto element : _elements)
            list.emplace_back(element);
    }

    compiler::element* argument_list::param_at_index(size_t index) {
        if (index < _elements.size())
            return _elements[index];
        return nullptr;
    }

    compiler::argument_list* argument_list::variadic_arguments() const {
        if (_elements.empty())
            return nullptr;
        auto last_arg = _elements.back();
        if (last_arg != nullptr
        &&  last_arg->element_type() == element_type_t::argument_list) {
            return dynamic_cast<compiler::argument_list*>(last_arg);
        }
        return nullptr;
    }

    compiler::element* argument_list::param_by_name(const std::string& name) {
        auto it = _param_index.find(name);
        if (it == _param_index.end())
            return nullptr;
        return _elements[it->second];
    }

};