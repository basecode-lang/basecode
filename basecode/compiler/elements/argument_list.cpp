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

#include <common/bytes.h>
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

//    bool argument_list::on_emit(
//            compiler::session& session,
//            compiler::emit_context_t& context,
//            compiler::emit_result_t& result) {
//        auto& assembler = session.assembler();
//        auto block = assembler.current_block();
//        return emit_elements(session, block, _elements);
//    }

    void argument_list::clear() {
        _elements.clear();
        _argument_index.clear();
    }

//    bool argument_list::emit_elements(
//            compiler::session& session,
//            vm::instruction_block* block,
//            const compiler::element_list_t& elements) {
//        for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
//            compiler::type* type = nullptr;
//
//            element* arg = *it;
//            switch (arg->element_type()) {
//                case element_type_t::argument_list: {
//                    auto result = emit_elements(
//                        session,
//                        block,
//                        dynamic_cast<compiler::argument_list*>(arg)->_elements);
//                    if (!result)
//                        return false;
//                    break;
//                }
//                case element_type_t::cast:
//                case element_type_t::transmute:
//                case element_type_t::proc_call:
//                case element_type_t::intrinsic:
//                case element_type_t::expression:
//                case element_type_t::nil_literal:
//                case element_type_t::float_literal:
//                case element_type_t::string_literal:
//                case element_type_t::unary_operator:
//                case element_type_t::assembly_label:
//                case element_type_t::binary_operator:
//                case element_type_t::boolean_literal:
//                case element_type_t::integer_literal:
//                case element_type_t::character_literal: {
//                    variable_handle_t arg_var;
//                    if (!session.variable(arg, arg_var))
//                        return false;
//
//                    if (!arg_var->read())
//                        return false;
//
//                    block->push(arg_var->emit_result().operands.back());
//
//                    if (!_is_foreign_call)
//                        type = arg_var->type_result().inferred_type;
//                    break;
//                }
//                case element_type_t::identifier_reference: {
//                    variable_handle_t arg_var;
//                    if (!session.variable(arg, arg_var))
//                        return false;
//
//                    type = arg_var->type_result().inferred_type;
//
//                    switch (type->element_type()) {
//                        case element_type_t::array_type:
//                        case element_type_t::tuple_type:
//                        case element_type_t::composite_type: {
//                            arg_var->address();
//                            if (!_is_foreign_call) {
//                                vm::register_t temp{};
//                                temp.type = vm::register_type_t::integer;
//                                session.assembler().allocate_reg(temp);
//                                defer(session.assembler().free_reg(temp));
//
//                                auto size = static_cast<uint64_t>(common::align(
//                                    type->size_in_bytes(),
//                                    8));
//                                block->sub(
//                                    vm::instruction_operand_t::sp(),
//                                    vm::instruction_operand_t::sp(),
//                                    vm::instruction_operand_t(size, vm::op_sizes::word));
//                                block->copy(
//                                    vm::op_sizes::byte,
//                                    vm::instruction_operand_t::sp(),
//                                    vm::instruction_operand_t(arg_var->address_reg()),
//                                    vm::instruction_operand_t(size, vm::op_sizes::word));
//                            } else {
//                                block->push(vm::instruction_operand_t(arg_var->address_reg()));
//                            }
//                            break;
//                        }
//                        default: {
//                            if (!arg_var->read())
//                                return false;
//
//                            block->push(arg_var->emit_result().operands.back());
//                            break;
//                        }
//                    }
//
//                    if (_is_foreign_call)
//                        type = nullptr;
//                    break;
//                }
//                default:
//                    break;
//            }
//
//            if (type != nullptr) {
//                auto size = static_cast<uint64_t>(common::align(
//                    type->size_in_bytes(),
//                    8));
//                _allocated_size += size;
//            }
//        }
//
//        return true;
//    }

    size_t argument_list::size() const {
        auto size = _elements.size();
        auto variadic_args = variadic_arguments();
        if (variadic_args != nullptr)
            size += (variadic_args->size() - 1);
        return size;
    }

    bool argument_list::as_ffi_arguments(
            compiler::session& session,
            vm::function_value_list_t& args) const {
        return recurse_ffi_arguments(session, _elements, args);
    }

    bool argument_list::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        auto index = find_index(e->id());
        if (index == -1) {
            return false;
        }
        replace(static_cast<size_t>(index), fold_result.element);
        return true;
    }

    compiler::element* argument_list::replace(
            size_t index,
            compiler::element* item) {
        auto old = _elements[index];
        _elements[index] = item;
        return old;
    }

    bool argument_list::recurse_ffi_arguments(
            compiler::session& session,
            const element_list_t& elements,
            vm::function_value_list_t& args) const {
        for (compiler::element* arg : elements) {
            if (arg->element_type() == element_type_t::argument_list) {
                auto arg_list = dynamic_cast<compiler::argument_list*>(arg);
                auto success = recurse_ffi_arguments(session, arg_list->elements(), args);
                if (!success)
                    return false;
                continue;
            }

            infer_type_result_t type_result {};
            if (!arg->infer_type(session, type_result))
                return false;

            vm::function_value_t value {};
            value.type = type_result.inferred_type->to_ffi_type();
            if (value.type == vm::ffi_types_t::struct_type) {
                auto composite_type = dynamic_cast<compiler::composite_type*>(type_result.inferred_type);
                if (composite_type == nullptr)
                    return false;
                for (auto fld : composite_type->fields().as_list()) {
                    vm::function_value_t fld_value;
                    fld_value.name = fld->identifier()->symbol()->name();
                    fld_value.type = fld->identifier()->type_ref()->type()->to_ffi_type();
                    value.fields.push_back(fld_value);
                }
            }
            args.push_back(value);
        }
        return true;
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

    bool argument_list::is_foreign_call() const {
        return _is_foreign_call;
    }

    uint64_t argument_list::allocated_size() const {
        return _allocated_size;
    }

    void argument_list::is_foreign_call(bool value) {
        _is_foreign_call = value;
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

    void argument_list::elements(const element_list_t& value) {
        _elements = value;
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

    void argument_list::argument_index(const argument_index_map_t& value) {
        _argument_index = value;
    }

    compiler::element* argument_list::param_by_name(const std::string& name) {
        auto it = _argument_index.find(name);
        if (it == _argument_index.end())
            return nullptr;
        return _elements[it->second];
    }

};