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
#include "type.h"
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

    void argument_list::clear() {
        _elements.clear();
        _argument_index.clear();
    }

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

            const auto& inferred = type_result.types.back();
            value.type = inferred.type->to_ffi_type();
            if (value.type == vm::ffi_types_t::struct_type) {
                auto composite_type = dynamic_cast<compiler::composite_type*>(inferred.type);
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

    void argument_list::allocated_size(size_t size) {
        _allocated_size = size;
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

}