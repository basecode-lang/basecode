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
#include <vm/instruction_block.h>
#include "program.h"
#include "intrinsic.h"
#include "identifier.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    using intrinsic_builder_callable = std::function<compiler::intrinsic* (
        compiler::element_builder&,
        compiler::block*,
        compiler::argument_list*)>;

    std::unordered_map<std::string, intrinsic_builder_callable> s_intrinsics = {
        {
            "size_of",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_size_of_intrinsic(
                    parent_scope,
                    args);
            }
        },
        {
            "free",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_free_intrinsic(
                    parent_scope,
                    args);
            }
        },
        {
            "alloc",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_alloc_intrinsic(
                    parent_scope,
                    args);
            }
        },
        {
            "align_of",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_align_of_intrinsic(
                    parent_scope,
                    args);
            }
        },
        {
            "address_of",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_address_of_intrinsic(
                    parent_scope,
                    args);
            }
        },
        {
            "type_of",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_type_of_intrinsic(
                    parent_scope,
                    args);
            }
        },
        {
            "copy",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_copy_intrinsic(
                    parent_scope,
                    args);
            }
        },
        {
            "fill",
            [](compiler::element_builder& builder,
                    auto parent_scope,
                    auto args) -> compiler::intrinsic* {
                return builder.make_fill_intrinsic(
                    parent_scope,
                    args);
            }
        },
    };

    intrinsic* intrinsic::intrinsic_for_call(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const qualified_symbol_t& symbol) {
        auto it = s_intrinsics.find(symbol.name);
        if (it == s_intrinsics.end())
            return nullptr;

        auto intrinsic_element = it->second(
            session.builder(),
            parent_scope,
            args);
        intrinsic_element->location(symbol.location);

        return intrinsic_element;
    }

    intrinsic::intrinsic(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::argument_list* args) : element(module, parent_scope, element_type_t::intrinsic),
                                         _arguments(args) {
    }

    bool intrinsic::can_fold() const {
        return false;
    }

    std::string intrinsic::name() const {
        return "intrinsic";
    }

    compiler::argument_list* intrinsic::arguments() {
        return _arguments;
    }

    void intrinsic::on_owned_elements(element_list_t& list) {
        if (_arguments != nullptr)
            list.emplace_back(_arguments);
    }

};