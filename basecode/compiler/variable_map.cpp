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

    ///////////////////////////////////////////////////////////////////////////

    variable_map::variable_map(compiler::session& session) : _session(session) {
    }

    void variable_map::reset() {
        _variables.clear();
    }

    variable_list_t variable_map::variables() {
        variable_list_t list {};

        for (auto& kvp : _variables)
            list.emplace_back(&kvp.second);

        return list;
    }

    bool variable_map::build(compiler::block* block) {
        if (!find_referenced_module_variables(block))
            return false;

        if (!find_local_variables(block))
            return false;

        return true;
    }

    variable_t* variable_map::find(const std::string& name) {
        auto it = _variables.find(name);
        if (it == std::end(_variables))
            return nullptr;
        return &it->second;
    }

    void variable_map::release_temp(temp_pool_entry_t* entry) {
        if (entry == nullptr)
            return;
        auto it = _temps.find(entry->id);
        if (it == std::end(_temps))
            return;
        entry->available = true;
    }

    identifier_by_section_t& variable_map::module_variables() {
        return _module_variables;
    }

    bool variable_map::find_local_variables(compiler::block* block) {
        auto& scope_manager = _session.scope_manager();

        int64_t offset = 0;
        element_id_set_t processed {};

        return scope_manager.visit_child_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                if (scope->is_parent_type_one_of({element_type_t::proc_type}))
                    return true;

                for (auto ref_id : scope->references().as_list()) {
                    auto e = _session.elements().find(ref_id);
                    auto ref = dynamic_cast<compiler::identifier_reference*>(e);
                    if (ref == nullptr)
                        continue;

                    auto var = ref->identifier();
                    if (var->is_constant()
                    ||  processed.count(var->id()) > 0) {
                        continue;
                    }

                    auto type = var->type_ref()->type();
                    if (type->is_proc_type())
                        continue;

                    auto offset_result = ref->field_offset();
                    if (!var->parent_scope()->has_stack_frame()) {
                        if (offset_result.base_ref == nullptr
                        || !offset_result.base_ref->parent_scope()->has_stack_frame()) {
                            continue;
                        }
                    }

                    processed.insert(var->id());

                    offset += -type->size_in_bytes();

                    variable_t var_info {};
                    var_info.identifier = var;
                    var_info.frame_offset = offset;
                    var_info.label = var->label_name();
                    var_info.field_offset = offset_result;
                    var_info.type = variable_type_t::local;
                    var_info.state = variable_state_t::unknown;
                    var_info.number_class = type->number_class();

                    _variables.insert(std::make_pair(var->label_name(), var_info));
                }

                return true;
            },
            block);
    }

    temp_pool_entry_t* variable_map::retain_temp(number_class_t number_class) {
        auto temp = find_available_temp(number_class);
        if (temp != nullptr) {
            temp->available = false;
            return temp;
        }

        variable_t var_info {};
        var_info.number_class = number_class;
        var_info.type = variable_type_t::temporary;
        var_info.label = fmt::format("temp{}", _temps.size() + 1);

        temp_pool_entry_t entry {};
        entry.available = false;
        entry.id = common::id_pool::instance()->allocate();

        auto vit = _variables.insert(std::make_pair(var_info.label, var_info));
        entry.variable = &vit.first->second;

        auto it = _temps.insert(std::make_pair(entry.id, entry));
        return &it.first->second;
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

                    variable_t var_info {};
                    var_info.identifier = var;
                    var_info.label = var->label_name();
                    var_info.type = variable_type_t::module;
                    var_info.state = variable_state_t::unknown;
                    var_info.field_offset = ref->field_offset();
                    var_info.number_class = type->number_class();

                    _variables.insert(std::make_pair(var->label_name(), var_info));
                }

                return true;
            },
            block);
    }

    temp_pool_entry_t* variable_map::find_available_temp(number_class_t number_class) {
        for (const auto& kvp : _temps) {
            if (kvp.second.available
            &&  kvp.second.variable->number_class == number_class) {
                return const_cast<temp_pool_entry_t*>(&kvp.second);
            }
        }
        return nullptr;
    }

}