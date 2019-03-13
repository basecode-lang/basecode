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

#include "session.h"
#include "elements.h"
#include "element_map.h"
#include "variable_map.h"
#include "scope_manager.h"

namespace basecode::compiler {

    variable_t::~variable_t() {

    }

    ///////////////////////////////////////////////////////////////////////////

    variable_map::variable_map(compiler::session& session) : _session(session) {
    }

    void variable_map::reset() {
        _variables.clear();
    }

    bool variable_map::build(compiler::block* block) {
        if (!find_referenced_module_variables(block))
            return false;

        return true;
    }

    identifier_by_section_t& variable_map::module_variables() {
        return _module_variables;
    }

    bool variable_map::emit_prologue(vm::basic_block** basic_block) {
        return false;
    }

    bool variable_map::emit_epilogue(vm::basic_block** basic_block) {
        return false;
    }

    bool variable_map::find_referenced_module_variables(compiler::block* block) {
        auto& scope_manager = _session.scope_manager();

        const element_type_set_t excluded_types = {
            element_type_t::proc_type,
            element_type_t::module_type,
            element_type_t::namespace_type,
        };

        element_id_set_t processed {};

        return scope_manager.visit_child_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                for (auto ref_id : scope->references().as_list()) {
                    auto e = _session.elements().find(ref_id);
                    auto ref = dynamic_cast<compiler::identifier_reference*>(e);
                    if (ref == nullptr)
                        continue;

                    auto var = ref->identifier();
                    auto type = var->type_ref()->type();
                    if (type->is_type_one_of(excluded_types)
                    ||  processed.count(var->id()) > 0) {
                        continue;
                    }

                    processed.insert(var->id());

                    if (var->usage() == identifier_usage_t::stack
                    ||  var->parent_scope()->has_stack_frame()) {
                        continue;
                    }

                    auto offset_result = ref->field_offset();

                    variable_t var_info {
                        .offset = offset_result.value,
                        .path = offset_result.path,
                        .label = var->label_name(),
                        .type = variable_type_t::module,
                        .base_label = offset_result.base_label,
                        .state = variable_state_t::unknown,
                        .map = this,
                        .number_class = type->number_class(),
                        .identifier = var,
                    };
                    _variables.insert(std::make_pair(var->label_name(), var_info));
                }

                return true;
            },
            block);
    }

}