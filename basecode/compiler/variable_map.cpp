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

    size_t variable_t::size_in_bytes() const {
        auto var = identifier;
        if (var != nullptr)
            return var->type_ref()->type()->size_in_bytes();
        return 0;
    }

    bool variable_t::flag(basecode::compiler::variable_t::flags_t f) const {
        return (state & f) == f;
    }

    void variable_t::flag(basecode::compiler::variable_t::flags_t f, bool value) {
        if (value)
            state |= f;
        else
            state &= ~f;
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
            compiler::identifier* identifier,
            vm::instruction_operand_t& result) {
        return true;
    }

    bool variable_context::deref(
            vm::basic_block** basic_block,
            vm::instruction_operand_t& expr,
            vm::instruction_operand_t& result) {
        return true;
    }

    bool variable_context::assign(
            vm::basic_block** basic_block,
            vm::instruction_operand_t& lhs,
            vm::instruction_operand_t& rhs) {
//        const auto& lhs_inferred = lhs_result.type_result.types.back();
//        const auto& rhs_inferred = rhs_result.type_result.types.back();
//
//        auto copy_required = false;
//        auto lhs_is_composite = lhs_inferred.type->is_composite_type();
//        auto rhs_is_composite = rhs_inferred.type->is_composite_type();
//
//        if (!lhs_inferred.type->is_pointer_type()) {
//            if (lhs_is_composite && !rhs_is_composite) {
//                _session.error(
//                    binary_op->module(),
//                    "X000",
//                    "cannot assign scalar to composite type.",
//                    binary_op->rhs()->location());
//                return false;
//            }
//
//            if (!lhs_is_composite && rhs_is_composite) {
//                _session.error(
//                    binary_op->module(),
//                    "X000",
//                    "cannot assign composite type to scalar.",
//                    binary_op->rhs()->location());
//                return false;
//            }
//
//            copy_required = lhs_is_composite && rhs_is_composite;
//        }

        return true;
    }

    bool variable_context::activate(
            vm::basic_block** basic_block,
            compiler::identifier* identifier) {
        return true;
    }

    bool variable_context::deactivate(
            vm::basic_block** basic_block,
            compiler::identifier* identifier) {
        return true;
    }

    bool variable_context::address_of(
            vm::basic_block** basic_block,
            vm::instruction_operand_t& expr,
            vm::instruction_operand_t& result) {
        return true;
    }

    bool variable_context::deactivate_scope(
            vm::basic_block** basic_block,
            compiler::block* scope) {
        for (auto& kvp : _variables) {
            if (kvp.second.type == variable_type_t::temporary)
                continue;
            auto& var = kvp.second;
            if (var.identifier->parent_scope() == scope) {
                if (!deactivate(basic_block, var.identifier)) {
                    // XXX
                    return false;
                }
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

        auto vit = _variables.insert(std::make_pair(var_info.label, var_info));
        entry.variable = &vit.first->second;

        auto it = _temps.insert(std::make_pair(entry.id, entry));
        return &it.first->second;
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

    void variable_context::release_temps(const temp_pool_entry_list_t& temps) {
        for (auto t : temps)
            release_temp(t);
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