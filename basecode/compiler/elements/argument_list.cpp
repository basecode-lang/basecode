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

    void argument_list::reverse() {
        std::reverse(std::begin(_elements), std::end(_elements));
    }

    size_t argument_list::size() const {
        return _elements.size();
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

        if (_elements.size() < field_list.size()) {
            _elements.resize(field_list.size());
        }

        size_t index = 0;
        for (auto fld : field_list) {
            _param_index.insert(std::make_pair(
                fld->identifier()->symbol()->name(),
                index++));
        }

        element_list_t temp {};
        temp.resize(_elements.size());

        for (index = 0; index < _elements.size(); index++) {
        _retry:
            auto param = _elements[index];
            if (param == nullptr)
                continue;

            if (param->element_type() == element_type_t::argument_pair) {
                auto arg_pair = dynamic_cast<compiler::argument_pair*>(param);
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
                            "P019",
                            fmt::format("invalid procedure parameter: {}", key),
                            param->location());
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

    bool argument_list::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        for (auto it = _elements.rbegin(); it != _elements.rend(); ++it) {
            element* arg = *it;
            switch (arg->element_type()) {
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
                    arg_var->read();
                    block->push(arg_var->value_reg());
                    break;
                }
                default:
                    break;
            }
        }

        return true;
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

    compiler::element* argument_list::param_by_name(const std::string& name) {
        auto it = _param_index.find(name);
        if (it == _param_index.end())
            return nullptr;
        return _elements[it->second];
    }

};