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

#include <common/defer.h>
#include <compiler/session.h>
#include <compiler/element_builder.h>
#include "intrinsic.h"
#include "identifier.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    using intrinsic_builder_callable = std::function<compiler::intrinsic* (
        compiler::element_builder&,
        compiler::block*,
        compiler::argument_list*,
        compiler::procedure_type*,
        const compiler::type_reference_list_t& type_params)>;

    std::unordered_map<intrinsic_type_t, compiler::procedure_type*> s_proc_types {};

    std::unordered_map<intrinsic_type_t, intrinsic_builder_callable> s_intrinsics = {
        {
            intrinsic_type_t::size_of,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_size_of_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::free,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_free_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::alloc,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_alloc_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::align_of,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_align_of_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::address_of,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_address_of_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::type_of,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_type_of_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::copy,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_copy_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::fill,
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args,
                    auto proc_type,
                    auto type_params) -> compiler::intrinsic* {
                return builder.make_fill_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
        {
            intrinsic_type_t::range,
            [](compiler::element_builder& builder,
                   auto parent_scope,
                   auto args,
                   auto proc_type,
                   auto type_params) -> compiler::intrinsic* {
                return builder.make_range_intrinsic(
                    parent_scope,
                    args,
                    proc_type,
                    type_params);
            }
        },
        {
            intrinsic_type_t::length_of,
            [](compiler::element_builder& builder,
                   auto parent_scope,
                   auto args,
                   auto proc_type,
                   auto type_params) -> compiler::intrinsic* {
                return builder.make_length_of_intrinsic(
                    parent_scope,
                    args,
                    proc_type);
            }
        },
    };

    bool intrinsic::register_intrinsic_procedure_type(
            const std::string_view& name,
            compiler::procedure_type* procedure_type) {
        auto type = intrinsic_type_from_name(name);
        if (type == intrinsic_type_t::unknown)
            return false;
        s_proc_types[type] = procedure_type;
        return true;
    }

    intrinsic* intrinsic::intrinsic_for_call(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            intrinsic_type_t type,
            const common::source_location& location,
            const compiler::type_reference_list_t& type_params) {
        auto it = s_intrinsics.find(type);
        if (it == std::end(s_intrinsics))
            return nullptr;

        auto proc_type_it = s_proc_types.find(type);
        if (proc_type_it == std::end(s_proc_types)) {
            // XXX: error
            return nullptr;
        }

        auto intrinsic_element = it->second(
            session.builder(),
            parent_scope,
            args,
            proc_type_it->second,
            type_params);
        intrinsic_element->location(location);
        return intrinsic_element;
    }

    intrinsic::intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params) : element(module, parent_scope, element_type_t::intrinsic),
                                                                  _arguments(args),
                                                                  _procedure_type(proc_type),
                                                                  _type_parameters(type_params) {
    }

    bool intrinsic::can_fold() const {
        return false;
    }

    compiler::element* intrinsic::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto intrinsic = intrinsic::intrinsic_for_call(
            session,
            new_scope,
            _arguments->clone<compiler::argument_list>(session, new_scope),
            type(),
            location(),
            _type_parameters);
        intrinsic->_uniform_function_call = _uniform_function_call;
        return intrinsic;
    }

    intrinsic_type_t intrinsic::type() const {
        return intrinsic_type_t::unknown;
    }

    bool intrinsic::uniform_function_call() const {
        return _uniform_function_call;
    }

    void intrinsic::uniform_function_call(bool value) {
        _uniform_function_call = value;
    }

    compiler::argument_list* intrinsic::arguments() const {
        return _arguments;
    }

    compiler::procedure_type* intrinsic::procedure_type() {
        return _procedure_type;
    }

    void intrinsic::on_owned_elements(element_list_t& list) {
        if (_arguments != nullptr)
            list.emplace_back(_arguments);

        for (auto type_param : _type_parameters)
            list.emplace_back(type_param);
    }

    const compiler::type_reference_list_t& intrinsic::type_parameters() const {
        return _type_parameters;
    }

}