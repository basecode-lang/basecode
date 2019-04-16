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
#include <common/id_pool.h>
#include <compiler/session.h>
#include <compiler/element_builder.h>
#include <compiler/type_name_builder.h>
#include "block.h"
#include "field.h"
#include "element.h"
#include "identifier.h"
#include "family_type.h"
#include "declaration.h"
#include "initializer.h"
#include "generic_type.h"
#include "argument_pair.h"
#include "argument_list.h"
#include "procedure_call.h"
#include "procedure_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "integer_literal.h"
#include "binary_operator.h"

namespace basecode::compiler {

    std::string procedure_type::name_for_procedure_type() {
        type_name_builder builder {};
        builder
            .add_part("proc")
            .add_part(common::id_pool::instance()->allocate());
        return builder.format();
    }

    ///////////////////////////////////////////////////////////////////////////

    procedure_type::procedure_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* header_scope,
            compiler::symbol_element* symbol) : compiler::type(
                                                      module,
                                                      parent_scope,
                                                      element_type_t::proc_type,
                                                      symbol),
                                                _header_scope(header_scope) {
    }

    bool procedure_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        if (other == nullptr)
            return false;

        // XXX: very temporary hack...
        return other->element_type() == element_type_t::proc_type;
    }

    bool procedure_type::prepare_call_site(
            compiler::session& session,
            bool uniform_function_call,
            compiler::argument_list* args,
            compiler::prepare_call_site_result_t& result) const {
        result.arguments = args->elements();

        auto field_list = _parameters.as_list();
        if (field_list.empty()) {
            if (!result.arguments.empty()) {
                result.messages.error(
                    "X000",
                    "procedure declares no parameters.",
                    args->parent_element()->location());
                return false;
            }
            return true;
        }

        if (uniform_function_call) {
            auto first_param = field_list.front();
            auto type = first_param->identifier()->type_ref()->type();
            if (!type->is_pointer_type()) {
                result.messages.error(
                    "X000",
                    "procedures used in uniform function calls must declare the first parameter as a pointer.",
                    args->parent_element()->location());
                return false;
            }
        }

        auto& builder = session.builder();

        if (result.arguments.size() < field_list.size())
            result.arguments.resize(field_list.size());

        element_list_t temp {};
        temp.resize(field_list.size());
        compiler::argument_list* variadic_args = nullptr;

        size_t index = 0;
        for (index = 0; index < field_list.size(); index++) {
            auto fld = field_list[index];
            auto fld_name = fld->identifier()->symbol()->name();
            if (fld->is_variadic()) {
                if (index < field_list.size() - 1) {
                    result.messages.error(
                        "X000",
                        fmt::format(
                            "variadic parameter only valid in final position: {}",
                            fld_name),
                            args->parent_element()->location());
                    return false;
                } else {
                    variadic_args = builder.make_argument_list(args->parent_scope());
                    variadic_args->parent_element(args);
                    temp[index] = variadic_args;
                }
            }
            result.index.insert(std::make_pair(fld_name, index));
        }

        auto last_field = field_list.back();
        for (index = 0; index < result.arguments.size(); index++) {
        _retry:
            auto arg = result.arguments[index];
            if (arg == nullptr)
                continue;

            if (last_field->is_variadic()
            &&  index >= field_list.size() - 1) {
                if (variadic_args == nullptr) {
                    result.messages.error(
                        "X000",
                        "no variadic parameter defined.",
                        args->parent_element()->location());
                    return false;
                }

                variadic_args->add(arg);
                if (result.arguments.size() == field_list.size()) {
                    result.arguments[index] = nullptr;
                } else {
                    if (index < result.arguments.size()) {
                        result.arguments.erase(result.arguments.begin() + index);
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
                    auto it = result.index.find(key);
                    if (it != result.index.end()) {
                        temp[it->second] = arg_pair->rhs();
                        result.arguments.erase(result.arguments.begin() + index);
                        result.arguments.emplace_back(nullptr);
                        goto _retry;
                    } else {
                        result.messages.error(
                            "X000",
                            fmt::format("invalid procedure parameter: {}", key),
                            arg->location());
                        return false;
                    }
                }
            }
        }

        index = 0;
        type_check_options_t type_check_options {};
        for (auto fld : field_list) {
            auto param = result.arguments[index];
            if (param != nullptr) {
                infer_type_result_t type_result{};
                if (!param->infer_type(session, type_result)) {
                    result.messages.error(
                        "X000",
                        fmt::format(
                            "unable to infer type for parameter: {}",
                            fld->identifier()->symbol()->name()),
                        param->location());
                    return false;
                }

                const auto& inferred = type_result.types.back();

                auto type_ref = fld->identifier()->type_ref();
                type_check_options.strict =
                    !(param->element_type() == element_type_t::integer_literal
                        || param->element_type() == element_type_t::float_literal);
                if (!type_ref->type()->type_check(inferred.type, type_check_options)) {
                    std::string parameter_type {};
                    auto fld_type = fld->identifier()->type_ref()->type();
                    if (fld_type->element_type() == element_type_t::generic_type) {
                        auto generic_type = dynamic_cast<compiler::generic_type*>(fld_type);
                        auto constraint_type = generic_type->constraints().front()->type();
                        if (constraint_type->is_family_type()) {
                            parameter_type = "family(";
                            auto family_type = dynamic_cast<compiler::family_type*>(constraint_type);
                            const auto& types = family_type->types();
                            for (size_t i = 0; i < types.size(); i++) {
                                if (i > 0) parameter_type += ",";
                                parameter_type += types[i]->symbol()->name();
                            }
                            parameter_type += ")";
                        } else {
                            parameter_type = constraint_type->symbol()->name();
                        }
                    } else {
                        parameter_type = fld_type->symbol()->name();
                    }
                    result.messages.error(
                        "X000",
                        fmt::format(
                            "type mismatch: cannot assign {} to parameter {}:{}.",
                            inferred.type_name(),
                            fld->identifier()->symbol()->name(),
                            parameter_type),
                        param->location());
                    return false;
                }
            } else {
                if (temp[index] != nullptr) {
                    result.arguments[index] = temp[index];
                    goto _next;
                }

                auto init = fld->identifier()->initializer();
                if (init == nullptr) {
                    auto decl = fld->declaration();
                    if (decl != nullptr) {
                        auto assignment = decl->assignment();
                        if (assignment != nullptr) {
                            result.arguments[index] = assignment->rhs();
                            goto _next;
                        }
                    }

                    result.messages.error(
                        "X000",
                        fmt::format(
                            "missing required parameter: {}",
                            fld->identifier()->symbol()->name()),
                        args->parent_element()->location());

                    return false;
                }

                result.arguments[index] = init->expression();
            }

        _next:
            index++;
        }

        return true;
    }

    bool procedure_type::has_return() const {
        return _has_return;
    }

    bool procedure_type::is_foreign() const {
        return _is_foreign;
    }

    bool procedure_type::is_template() const {
        return !_type_parameters.empty();
    }

    field_map_t& procedure_type::parameters() {
        return _parameters;
    }

    bool procedure_type::is_proc_type() const {
        return true;
    }

    void procedure_type::is_foreign(bool value) {
        _is_foreign = value;
    }

    void procedure_type::has_return(bool value) {
        _has_return = value;
    }

    bool procedure_type::on_is_constant() const {
        return true;
    }

    type_map_t& procedure_type::type_parameters() {
        return _type_parameters;
    }

    compiler::block* procedure_type::body_scope() {
        return _body_scope;
    }

    compiler::block* procedure_type::header_scope() {
        return _header_scope;
    }

    std::string procedure_type::label_name() const {
        auto parent_init = const_cast<procedure_type*>(this)->parent_element_as<compiler::initializer>();
        if (parent_init != nullptr) {
            auto parent_var = parent_init->parent_element_as<compiler::identifier>();
            if (parent_var != nullptr)
                return parent_var->label_name();
        }
        return element::label_name();
    }

    field_map_t& procedure_type::return_parameters() {
        return _return_parameters;
    }

    uint64_t procedure_type::foreign_address() const {
        return _foreign_address;
    }

    void procedure_type::foreign_address(uint64_t value) {
        _foreign_address = value;
    }

    compiler::procedure_type* procedure_type::instance_for(
            compiler::session& session,
            compiler::procedure_call* call) {
        if (!is_template())
            return this;

        std::unordered_map<std::string_view, compiler::type*> types {};
        const auto& name_list = _type_parameters.name_list();
        const auto& call_type_params = call->type_parameters();
        if (call_type_params.size() == name_list.size()) {
            size_t index = 0;
            for (auto type_param : name_list) {
                auto type_ref = call_type_params[index++];
                types.insert(std::make_pair(type_param, type_ref->type()));
            }
        } else {
            const auto& args = call->arguments()->elements();
            const auto& fields = _parameters.as_list();
            for (const auto& name : name_list) {
                auto vars = _header_scope->identifiers().find(name);
                if (vars.empty()) {
                    // XXX: error
                    return nullptr;
                }

                size_t field_index = 0;
                auto type_identifier = vars.front();
                auto type_id = type_identifier->type_ref()->type()->id();
                compiler::field* matching_field = nullptr;
                for (auto fld : fields) {
                    if (fld->identifier()->type_ref()->type()->id() == type_id) {
                        matching_field = fld;
                        break;
                    }
                    ++field_index;
                }

                if (matching_field == nullptr) {
                    // XXX: error
                    return nullptr;
                }

                auto arg = args[field_index];
                infer_type_result_t type_result{};
                if (!arg->infer_type(session, type_result))
                    return nullptr;

                types.insert(std::make_pair(name, type_result.types.front().type));
            }
        }

        // need to recursively clone the procedure_type

        return nullptr;
    }

    void procedure_type::body_scope(compiler::block* value) {
        _body_scope = value;
    }

    void procedure_type::on_owned_elements(element_list_t& list) {
        if (_body_scope != nullptr)
            list.emplace_back(_body_scope);

        if (_header_scope != nullptr)
            list.emplace_back(_header_scope);

        for (auto element : _parameters.as_list())
            list.emplace_back(element);

        for (auto element : _return_parameters.as_list())
            list.emplace_back(element);
    }

    bool procedure_type::on_initialize(compiler::session& session) {
        return true;
    }

}