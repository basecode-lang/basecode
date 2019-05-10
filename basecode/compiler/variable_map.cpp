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

#include <vm/assembler.h>
#include <vm/basic_block.h>
#include <compiler/byte_code_emitter.h>
#include "session.h"
#include "elements.h"
#include "element_map.h"
#include "variable_map.h"
#include "scope_manager.h"

namespace basecode::compiler {

    ///////////////////////////////////////////////////////////////////////////

    // Example stack frame for a procedure:
    //
    // +-------------+
    // | ...         |
    // | locals      | -offsets
    // | fp          | 0
    // | return addr | +offsets
    // | return slot |
    // | param 1     |
    // | param 2     |
    // +-------------+
    //

    ///////////////////////////////////////////////////////////////////////////

    // variable rules
    // -----------------------------------------------------------------------
    //
    // module: these variables are most similar to static variables in C/C++.
    //         however, unlike static variables, it's possible for other
    //         modules to access them through the module reference, e.g.
    //
    //         some_module :: module("some_module");
    //
    //         core::print("some_module = ${some_module::foo}");
    //
    //         module variables may live in any namespace. if you want to
    //         make a module variable "private", you can put it in a namespace
    //         you don't publish as part of the API.  (though, if people know it,
    //         they can still get to the variable).
    //
    //         these variables are reserved in the executable image. bss, ro_data, or data
    //         are the primary sections in play.  note: unlike some other languages,
    //         basecode does not "copy" values into the image; rather, the initialized
    //         values are persisted into the image.  if there is no initializer or
    //         the variable is explicitly uninitialized, only space is reserved in the image.
    //
    // locals: these live on the stack (negative offsets from FP) or
    //          registers if they're primitive root values.
    //
    // parameters: these live on the stack at positive offsets from FP
    //
    // return values: these live on the stack at positive offsets from FP
    //
    // temporary: these are always registers
    //
    //

    // state machine rules
    // ------------------------------------------------------------------------
    //
    // all variables start off life in the spilled state.  if a variable is
    // activated, then it is filled.  if it requires initialization, that is
    // performed.  in some cases, the act of filling is equivalent to initialization
    // and only one of these is emitted.
    //
    // spilled -+--> filled
    //          |
    //          +--> may need init
    //
    // if a block has exceeded the register limitation, then it cannot be
    // filled. read and write to variables works regardless of spilled/filled state.
    //

    ///////////////////////////////////////////////////////////////////////////

    bool variable_t::is_live() const {
        return (state & flags_t::live) == flags_t::live;
    }

    void variable_t::mark_modified() {
        state |= flags_t::modified;
    }

    bool variable_t::is_filled() const {
        return (state & flags_t::filled) == flags_t::filled;
    }

    bool variable_t::is_spilled() const {
        return (state & flags_t::spilled) == flags_t::spilled;
    }

    bool variable_t::is_excluded() const {
        return (state & flags_t::excluded) == flags_t::excluded;
    }

    bool variable_t::is_modified() const {
        return (state & flags_t::modified) == flags_t::modified;
    }

    void variable_t::transition_to_live() {
        state |= flags_t::live;
    }

    void variable_t::transition_to_filled() {
        state |= flags_t::filled;
        state &= ~flags_t::spilled;
    }

    void variable_t::transition_to_killed() {
        state &= ~flags_t::live;
    }

    void variable_t::transition_to_spilled() {
        state |= flags_t::spilled;
        state &= ~flags_t::filled;
        state &= ~flags_t::modified;
    }

    size_t variable_t::size_in_bytes() const {
        auto var = identifier;
        if (var != nullptr)
            return var->type_ref()->type()->size_in_bytes();
        return 0;
    }

    void variable_t::transition_to_excluded() {
        state |= flags_t::excluded;
    }

    ///////////////////////////////////////////////////////////////////////////

    variable_context::variable_context(
            compiler::session& session,
            common::id_t id,
            vm::basic_block* locals_block) : _id(id),
                                             _session(session),
                                             _locals_block(locals_block) {
    }

    bool variable_context::read(
            vm::basic_block** basic_block,
            compiler::identifier_reference* ref,
            vm::instruction_operand_t& result) {
        return read(basic_block, ref->identifier(), result, ref->field_offset());
    }

    bool variable_context::read(
            vm::basic_block** basic_block,
            compiler::identifier* var,
            vm::instruction_operand_t& result,
            const compiler::offset_result_t& offset_result) {
        auto label_name = get_variable_name(var, offset_result);

        auto var_info = find_variable(label_name);
        if (var_info == nullptr) {
            _session.error(
                var->module(),
                "X000",
                fmt::format("variable not found: {}", label_name),
                var->location());
            return false;
        }

        return fill(basic_block, var_info, result);
    }

    bool variable_context::fill(
            vm::basic_block** basic_block,
            variable_t* var_info,
            vm::instruction_operand_t& result) {
        auto& assembler = _session.assembler();
        auto current_block = *basic_block;

        if (!should_variable_fill(var_info))
            return true;

        fill_target_pair_t target_pair{};
        if (!prepare_fill_target_pair(var_info, target_pair))
            return false;

        result = vm::instruction_operand_t(target_pair.dest);
        if (var_info->is_filled())
            return true;

        const auto offset_operand = target_pair.offset != nullptr ?
            vm::instruction_operand_t(target_pair.offset) :
            vm::instruction_operand_t::empty();

        const auto source_operand = use_frame_pointer(var_info) ?
            vm::instruction_operand_t::fp() :
            vm::instruction_operand_t(assembler.make_named_ref(
                variable_type_to_named_ref_type(var_info->type),
                var_info->label));

        auto var_type = var_info->identifier->type_ref()->type();
        if (var_type->is_composite_type()) {
            current_block->move(result, source_operand, offset_operand);
        } else {
            current_block->load(result, source_operand, offset_operand);
        }

        var_info->transition_to_filled();

        return true;
    }

    bool variable_context::spill(
            vm::basic_block** basic_block,
            variable_t* var_info) {
        auto& assembler = _session.assembler();
        auto current_block = *basic_block;

        if (!should_variable_spill(var_info))
            return true;

        spill_target_pair_t target_pair{};
        if (!prepare_spill_target_pair(var_info, target_pair))
            return false;

        auto var_type = var_info->identifier->type_ref()->type();

        const auto source_operand = vm::instruction_operand_t(target_pair.src);

        const auto offset_operand = target_pair.offset != nullptr ?
            vm::instruction_operand_t(target_pair.offset) :
            vm::instruction_operand_t::empty();

        const auto target_operand = use_frame_pointer(var_info) ?
            vm::instruction_operand_t::fp() :
            vm::instruction_operand_t(assembler.make_named_ref(
                variable_type_to_named_ref_type(var_info->type),
                var_info->label));

        current_block->store(target_operand, source_operand, offset_operand);

        var_info->transition_to_spilled();
        return true;
    }

    bool variable_context::deref(
            vm::basic_block** basic_block,
            emit_result_t& expr,
            vm::instruction_operand_t& result) {
        return true;
    }

    bool variable_context::assign(
            vm::basic_block** basic_block,
            compiler::element* element,
            emit_result_t& lhs,
            emit_result_t& rhs) {
        if (lhs.operands.empty() || rhs.operands.empty()) {
            _session.error(
                element->module(),
                "X000",
                "lhs and rhs results must have operands.",
                element->location());
            return false;
        }

        auto lhs_named_ref = lhs.operands.front().data<vm::named_ref_with_offset_t>();

        auto var_info = find_variable(lhs_named_ref->ref->name);
        if (var_info == nullptr) {
            _session.error(
                element->module(),
                "X000",
                fmt::format("variable not found: {}", lhs_named_ref->ref->name),
                element->location());
            return false;
        }

        // N.B. if the rhs_result has a named_ref operand that
        //      matches the lhs_result's named_ref operand, then
        //      we'd emit a MOVE that has the same register for
        //      both operands, so we can safely skip it.
        auto& rhs_operand = rhs.operands.back();
        if (rhs_operand.type() == vm::instruction_operand_type_t::named_ref) {
            auto rhs_named_ref = rhs_operand.data<vm::named_ref_with_offset_t>();
            if (rhs_named_ref->ref->name == lhs_named_ref->ref->name) {
                var_info->mark_modified();
                return true;
            }
        }

        auto copy_required = false;
        const auto& lhs_inferred = lhs.type_result.types.back();
        const auto& rhs_inferred = rhs.type_result.types.back();

        if (!lhs_inferred.type->is_pointer_type()) {
            const auto lhs_is_composite = lhs_inferred.type->is_composite_type();
            const auto rhs_is_composite = rhs_inferred.type->is_composite_type();

            if (lhs_is_composite && !rhs_is_composite) {
                _session.error(
                    element->module(),
                    "X000",
                    "cannot assign scalar to composite type.",
                    element->location());
                return false;
            }

            if (!lhs_is_composite && rhs_is_composite) {
                _session.error(
                    element->module(),
                    "X000",
                    "cannot assign composite type to scalar.",
                    element->location());
                return false;
            }

            copy_required = lhs_is_composite && rhs_is_composite;
        }

        fill_target_pair_t target_pair{};
        if (!prepare_fill_target_pair(var_info, target_pair))
            return false;

        auto current_block = *basic_block;

        rhs_operand.size(target_pair.dest->size);
        current_block->move(vm::instruction_operand_t(target_pair.dest), rhs_operand);
        var_info->mark_modified();

        return true;
    }

    bool variable_context::activate(
            vm::basic_block** basic_block,
            compiler::identifier* var,
            const offset_result_t& offset_result) {
        auto type = var->type_ref()->type();
        if (var->is_constant() || type->is_proc_type())
            return false;

        auto label_name = get_variable_name(var, offset_result);
        auto existing_var_info = find_variable(label_name);
        if (existing_var_info != nullptr) {
            if (!existing_var_info->is_live()) {
                existing_var_info->transition_to_live();
                existing_var_info->transition_to_spilled();
            }
            vm::instruction_operand_t result{};
            return read(basic_block, var, result, offset_result);
        }

        size_t offset = 0;
        auto var_type = variable_type_t::module;
        if (var->usage() == identifier_usage_t::stack) {
            auto tag = field_tag_t::none;
            if (var->field() != nullptr)
                tag = var->field()->tag();

            switch (tag) {
                case field_tag_t::none: {
                    var_type = variable_type_t::local;
                    _local_offset += -type->size_in_bytes();
                    offset = _local_offset;
                    break;
                }
                case field_tag_t::parameter: {
                    var_type = variable_type_t::parameter;
                    offset = _parameter_offset;
                    _parameter_offset += common::align(type->size_in_bytes(), 8);
                    break;
                }
                case field_tag_t::return_parameter: {
                    var_type = variable_type_t::return_parameter;
                    offset = _return_offset;
                    _return_offset += common::align(type->size_in_bytes(), 8);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        variable_t var_info {};
        var_info.type = var_type;
        var_info.identifier = var;
        var_info.label = label_name;
        var_info.frame_offset = offset;
        var_info.field_offset = offset_result;
        var_info.number_class = type->number_class();
        var_info.op_size = vm::op_size_for_byte_size(type->size_in_bytes());

        var_info.transition_to_live();
        var_info.transition_to_spilled();

        // if we've added more locals than the maximum number of registers we're allowed per number class,
        // mark this variable as "excluded".  it will remained spilled.
        //
        // the code leaves room for 3 temporary variables so we have someplace to put our excluded values
        auto max_register_count = _config.max_register_count[static_cast<size_t>(var_info.number_class)] - 3;
        if (_variables.size() >= max_register_count) {
            var_info.transition_to_excluded();
        }

        if (var->is_initialized()
        ||  var->type_ref()->is_composite_type()
        ||  var->type_ref()->is_pointer_type()) {
            var_info.state |= variable_t::flags_t::initialize;
        }

        _locals_block->local(
            number_class_to_local_type(var_info.number_class),
            var_info.label,
            offset,
            std::string(variable_type_to_group(var_info.type)));

        _variables.insert(std::make_pair(var_info.label, var_info));

        vm::instruction_operand_t result{};
        return read(basic_block, var, result, offset_result);
    }

    bool variable_context::activate(
            vm::basic_block** basic_block,
            compiler::identifier_reference* ref) {
        return activate(basic_block, ref->identifier(), ref->field_offset());
    }

    bool variable_context::deactivate(
            vm::basic_block** basic_block,
            compiler::identifier* var,
            const offset_result_t& offset_result) {
        auto label_name = get_variable_name(var, offset_result);

        auto var_info = find_variable(label_name);
        if (var_info == nullptr) {
            _session.error(
                var->module(),
                "X000",
                fmt::format("variable not found: {}", label_name),
                var->location());
            return false;
        }

        if (!spill(basic_block, var_info))
            return false;

        if (var_info->temp != nullptr)
            release_temp(var_info->temp);

        var_info->transition_to_killed();

        return true;
    }

    bool variable_context::address_of(
            vm::basic_block** basic_block,
            emit_result_t& expr,
            vm::instruction_operand_t& result) {
        return true;
    }

    bool variable_context::deactivate_scope(
            vm::basic_block** basic_block,
            compiler::block* scope) {
        for (auto& kvp : _variables) {
            if (kvp.second.type == variable_type_t::temporary)
                continue;

            const auto& var = kvp.second;
            const auto matching_scope = var.identifier->parent_scope() == scope;
            const auto is_procedure_scoped = var.type == variable_type_t::local
                || var.type == variable_type_t::parameter;

            if (is_procedure_scoped && !matching_scope)
                continue;

            if (!deactivate(basic_block, var.identifier, var.field_offset)) {
                _session.error(
                    var.identifier->module(),
                    "X000",
                    fmt::format("variable deactivate failed: {}", kvp.first),
                    var.identifier->location());
                return false;
            }
        }
        return true;
    }

    common::id_t variable_context::id() const {
        return _id;
    }

    variable_list_t variable_context::temps() {
        variable_list_t list {};

        for (auto& kvp : _variables) {
            if (kvp.second.type == variable_type_t::temporary)
                list.emplace_back(&kvp.second);
        }

        return list;
    }

    void variable_context::apply_variable_range(
            const const_variable_list_t& list,
            vm::instruction_operand_list_t& operands,
        bool reverse) {
        if (list.empty())
            return;

        auto& assembler = _session.assembler();

        if (list.size() == 1) {
            operands.push_back(vm::instruction_operand_t(assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                list.front()->label)));
        } else {
            const variable_t* front = nullptr;
            const variable_t* back = nullptr;

            if (reverse) {
                front = list.back();
                back = list.front();
            } else {
                front = list.front();
                back = list.back();
            }

            auto begin = assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                front->label);
            auto end = assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                back->label);
            operands.push_back(vm::instruction_operand_t(vm::named_ref_range_t{
                begin,
                end}));
        }
    }

    void variable_context::save_locals_to_stack(
            vm::basic_block* basic_block,
            const group_variable_result_t& groups) {
        vm::instruction_operand_list_t operands {};

        for (const auto& list : groups.ints)
            apply_variable_range(list, operands, false);

        for (const auto& list : groups.floats)
            apply_variable_range(list, operands, false);

        if (!operands.empty())
            basic_block->pushm(operands);
    }

    variable_list_t variable_context::variables() {
        variable_list_t list {};

        for (auto& kvp : _variables)
            list.emplace_back(&kvp.second);

        return list;
    }

    size_t variable_context::locals_size() const {
        size_t size = 0;

        for (auto& kvp : _variables) {
            const auto& var = kvp.second;
            if (var.type == variable_type_t::local)
                size += var.size_in_bytes();
        }

        return common::align(size, 8);
    }

    std::string variable_context::get_variable_name(
            compiler::identifier* var,
            const offset_result_t& offset_result) {
        auto label_name = offset_result.label_name();
        if (label_name.empty())
            label_name = var->label_name();
        return label_name;
    }

    bool variable_context::prepare_fill_target_pair(
            variable_t* var_info,
            fill_target_pair_t& target_pair) {
        auto& assembler = _session.assembler();

        if (var_info->is_excluded()) {
            if (var_info->temp == nullptr) {
                var_info->temp = retain_temp(var_info->number_class);
                if (var_info->temp == nullptr) {
                    const auto max_register_count = _config.max_register_count[static_cast<size_t>(var_info->number_class)];
                    _session.error(
                        var_info->identifier->module(),
                        "X000",
                        fmt::format(
                            "unable to retain temporary; exceeded maximum register limit: {}",
                            max_register_count),
                        var_info->identifier->location());
                    return false;
                }
            }

            target_pair.dest = assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                var_info->temp->name(),
                var_info->op_size);
        } else {
            target_pair.dest = assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                var_info->label,
                var_info->op_size);
        }

        target_pair.offset = assembler.make_named_ref(
            vm::assembler_named_ref_type_t::offset,
            var_info->label,
            vm::op_sizes::word);

        return true;
    }

    bool variable_context::prepare_spill_target_pair(
            variable_t* var_info,
            spill_target_pair_t& target_pair) {
        auto& assembler = _session.assembler();

        if (var_info->is_excluded()) {
            if (var_info->temp == nullptr) {
                _session.error(
                    var_info->identifier->module(),
                    "X000",
                    "expected temporary for excluded variable",
                    var_info->identifier->location());
                return false;
            }

            target_pair.src = assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                var_info->temp->name(),
                var_info->op_size);
        } else {
            target_pair.src = assembler.make_named_ref(
                vm::assembler_named_ref_type_t::local,
                var_info->label,
                var_info->op_size);
        }

        target_pair.offset = assembler.make_named_ref(
            vm::assembler_named_ref_type_t::offset,
            var_info->label,
            vm::op_sizes::word);

        return true;
    }

    void variable_context::restore_locals_from_stack(
            vm::basic_block* basic_block,
            const group_variable_result_t& groups) {
        auto working_groups = groups;
        std::reverse(std::begin(working_groups.ints), std::end(working_groups.ints));
        std::reverse(std::begin(working_groups.floats), std::end(working_groups.floats));

        vm::instruction_operand_list_t operands {};

        for (const auto& list : working_groups.floats)
            apply_variable_range(list, operands, true);

        for (const auto& list : working_groups.ints)
            apply_variable_range(list, operands, true);

        if (!operands.empty())
            basic_block->popm(operands);
    }

    void variable_context::add_scope(compiler::block* scope) {
        _scope_blocks.push_back(scope);
    }

    void variable_context::release_temp(temp_pool_entry_t* entry) {
        if (entry == nullptr)
            return;
        auto it = _temps.find(entry->id);
        if (it == std::end(_temps))
            return;
        entry->available = true;
    }

    variable_t* variable_context::find_variable(const std::string& name) {
        auto it = _variables.find(name);
        if (it == std::end(_variables))
            return nullptr;
        return &it->second;
    }

    void variable_context::release_temps(const temp_pool_entry_list_t& temps) {
        for (auto t : temps)
            release_temp(t);
    }

    bool variable_context::use_frame_pointer(const variable_t* var_info) const {
        switch (var_info->type) {
            case variable_type_t::local:
            case variable_type_t::parameter:
            case variable_type_t::return_parameter:
                return true;
            default:
                return false;
        }
    }

    temp_pool_entry_t* variable_context::find_temp(number_class_t number_class) {
        for (const auto& kvp : _temps) {
            if (kvp.second.available
            &&  kvp.second.variable->type == variable_type_t::temporary
            &&  kvp.second.variable->number_class == number_class) {
                return const_cast<temp_pool_entry_t*>(&kvp.second);
            }
        }
        return nullptr;
    }

    temp_pool_entry_t* variable_context::retain_temp(number_class_t number_class) {
        auto temp = find_temp(number_class);
        if (temp != nullptr) {
            temp->available = false;
            return temp;
        }

        variable_t var_info {};
        var_info.number_class = number_class;
        var_info.type = variable_type_t::temporary;
        var_info.label = fmt::format("t{}", _temps.size() + 1);

        temp_pool_entry_t entry {};
        entry.available = false;
        entry.id = common::id_pool::instance()->allocate();

        _locals_block->local(
            number_class_to_local_type(var_info.number_class),
            var_info.label);

        auto vit = _variables.insert(std::make_pair(var_info.label, var_info));
        entry.variable = &vit.first->second;

        auto it = _temps.insert(std::make_pair(entry.id, entry));
        return &it.first->second;
    }

    bool variable_context::should_variable_fill(const variable_t* var_info) const {
        if (var_info->is_excluded())
            return false;

        switch (var_info->type) {
            case variable_type_t::return_parameter: {
                return false;
            }
            default: {
                return true;
            }
        }
    }

    bool variable_context::should_variable_spill(const variable_t* var_info) const {
        if (!var_info->is_modified())
            return false;

        switch (var_info->type) {
            case variable_type_t::local: {
                return var_info->field_offset.base_ref != nullptr;
            }
            case variable_type_t::module:
            case variable_type_t::return_parameter: {
                return true;
            }
            case variable_type_t::parameter: {
                return var_info->identifier->type_ref()->is_pointer_type();
            }
            default: {
                return false;
            }
        }
    }

    bool variable_context::is_related_to_type(const variable_t* var, variable_type_t type) {
        if (var->field_offset.base_ref == nullptr)
            return false;

        const auto& vars = variables();
        for (auto v : vars) {
            if (var->field_offset.base_ref->identifier() == v->identifier
            &&  v->type == type) {
                return true;
            }
        }

        return false;
    }

    group_variable_result_t variable_context::group_variables(const variable_set_t& excluded) {
        group_variable_result_t result {};
        result.ints.emplace_back();
        result.floats.emplace_back();

        for (const auto& kvp : _variables) {
            auto var = &kvp.second;
            auto list = var->number_class == number_class_t::integer ?
                        &result.ints :
                        &result.floats;
            if (excluded.count(const_cast<variable_t*>(var)) > 0) {
                list->emplace_back();
                continue;
            }
            list->back().emplace_back(var);
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////

    variable_map::variable_map(compiler::session& session) : _session(session) {
    }

    void variable_map::reset() {
        _contexts.clear();
    }

    bool variable_map::initialize() {
        create_sections();
        return group_module_variables_into_sections();
    }

    void variable_map::create_sections() {
        _module_variables.sections.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        _module_variables.sections.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        _module_variables.sections.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
    }

    identifier_by_section_t& variable_map::module_variables() {
        return _module_variables;
    }

    bool variable_map::group_module_variables_into_sections() {
        auto& scope_manager = _session.scope_manager();

        auto bss_list = _module_variables.variable_section(vm::section_t::bss);
        auto data_list = _module_variables.variable_section(vm::section_t::data);
        auto ro_list = _module_variables.variable_section(vm::section_t::ro_data);

        std::set<common::id_t> processed_identifiers {};

        const element_type_set_t parent_types {element_type_t::field};
        const element_type_set_t ignored_types {
            element_type_t::generic_type,
            element_type_t::namespace_type,
            element_type_t::module_reference,
        };

        auto identifier_refs = _session
            .elements()
            .find_by_type<compiler::identifier_reference>(element_type_t::identifier_reference);
        for (auto ref : identifier_refs) {
            auto var = ref->identifier();
            if (processed_identifiers.count(var->id()) > 0)
                continue;

            processed_identifiers.insert(var->id());

            if (scope_manager.within_local_scope(var->parent_scope()))
                continue;

            auto var_parent = var->parent_element();
            if (var_parent != nullptr
            &&  var_parent->is_parent_type_one_of(parent_types)) {
                continue;
            }

            auto var_type = var->type_ref()->type();
            if (var_type == nullptr) {
                // XXX: this is an error!
                return false;
            }

            if (var_type->is_type_one_of(ignored_types)) {
                continue;
            }

            auto init = var->initializer();
            if (init != nullptr) {
                switch (init->expression()->element_type()) {
                    case element_type_t::directive: {
                        auto directive = dynamic_cast<compiler::directive*>(init->expression());
                        if (!directive->is_valid_data())
                            continue;
                        break;
                    }
                    case element_type_t::proc_type:
                    case element_type_t::composite_type:
                    case element_type_t::type_reference:
                    case element_type_t::module_reference:
                        continue;
                    default:
                        break;
                }
            }

            auto has_initializers = var->is_initialized() || var_type->is_array_type();
            auto is_constant = var->is_constant()
                               && var_type->element_type() != element_type_t::tuple_type;

            if (is_constant) {
                ro_list->emplace_back(var);
            } else {
                _module_variables.identifiers.insert(var->id());

                if (!has_initializers) {
                    bss_list->emplace_back(var);
                } else {
                    data_list->emplace_back(var);
                }
            }
        }

        return true;
    }

    variable_context* variable_map::find_context(common::id_t id) {
        auto it = _contexts.find(id);
        if (it == std::end(_contexts))
            return nullptr;
        return &it->second;
    }

    variable_context* variable_map::make_context(vm::basic_block* locals_block) {
        auto id = common::id_pool::instance()->allocate();
        auto result = _contexts.insert(std::make_pair(
            id,
            variable_context{_session, id, locals_block}));
        if (!result.second)
            return nullptr;
        return &result.first->second;
    }

}