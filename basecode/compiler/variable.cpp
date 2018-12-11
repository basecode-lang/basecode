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

#include <vm/instruction_block.h>
#include "session.h"
#include "variable.h"
#include "elements/type.h"
#include "elements/identifier.h"
#include "elements/declaration.h"
#include "elements/pointer_type.h"
#include "elements/unary_operator.h"
#include "elements/symbol_element.h"
#include "elements/composite_type.h"
#include "elements/type_reference.h"
#include "elements/identifier_reference.h"

namespace basecode::compiler {

    variable_register_t::variable_register_t(vm::assembler* assembler) : assembler(assembler) {
    }

    bool variable_register_t::reserve() {
        allocated = assembler->allocate_reg(reg);
        return allocated;
    }

    void variable_register_t::release() {
        if (!allocated)
            return;
        if (no_release)
            return;
        assembler->free_reg(reg);
        allocated = false;
    }

    bool variable_register_t::matches(vm::register_t* other_reg) {
        if (other_reg == nullptr)
            return true;
        return (*other_reg).number == reg.number;
    }

    ///////////////////////////////////////////////////////////////////////////

    variable::variable(
        compiler::session& session,
        compiler::element* element) : _value(&session.assembler()),
                                      _session(session),
                                      _address(&session.assembler()),
                                      _element(element) {
    }

    bool variable::read() {
        if (flag(flags_t::f_read))
            return false;

        auto& assembler = _session.assembler();
        auto block = assembler.current_block();
        if (block == nullptr)
            return true;

        address();

        switch (_element->element_type()) {
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(_element);
                auto on_stack = var->usage() == identifier_usage_t::stack;

                auto fmt = "{}";
                if (on_stack)
                    fmt = "stack: {}";

                block->comment(
                    fmt::format(fmt, _rot.path),
                    vm::comment_location_t::after_instruction);

                auto offset = _rot.offset;

                // XXX: total hack, do not keep!
                if (on_stack) {
                    offset += 4;
                }

                vm::instruction_operand_t result_operand(_value.reg);
                _result.operands.emplace_back(result_operand);

                block->load(
                    result_operand,
                    vm::instruction_operand_t(on_stack ? vm::register_t::sp() : _address.reg),
                    vm::instruction_operand_t::offset(offset));
                break;
            }
            default: {
                _element->emit(_session, _context, _result);
                break;
            }
        }

        flag(flags_t::f_read, true);
        flag(flags_t::f_written, false);
        return true;
    }

    bool variable::field(
            const std::string& name,
            variable_handle_t& handle,
            compiler::element* element,
            bool activate) {
        compiler::type* base_type = nullptr;
        if (_type.inferred_type->is_pointer_type()) {
            auto pointer_type = dynamic_cast<compiler::pointer_type*>(_type.inferred_type);
            base_type = pointer_type->base_type_ref()->type();
        } else {
            base_type = _type.inferred_type;
        }

        if (!base_type->is_composite_type())
            return false;

        auto type = dynamic_cast<compiler::composite_type*>(base_type);
        auto field = type->fields().find_by_name(name);
        if (field == nullptr)
            return false;

        // XXX: think on this... not super happy with how pointer deref is being handled
        //      in this scenario.  also, what happens if we have multiple derefs?
        auto result = false;
        if (element != nullptr) {
            if (_session.variable(element, handle, activate)) {
                compiler::element* var = field->identifier();
                if (element->element_type() == element_type_t::unary_operator) {
                    auto unary_op = dynamic_cast<compiler::unary_operator*>(element);
                    if (unary_op->operator_type() == operator_type_t::pointer_dereference) {
                        var = unary_op->rhs();
                    } else {
                        // XXX: error!
                    }
                }

                variable_handle_t temp_handle{};
                result = _session.variable(var, temp_handle, false);
                if (result) {
                    temp_handle->_parent = this;
                    temp_handle->_field = field;
                }
            }
        } else {
            result = _session.variable(
                field->identifier(),
                handle,
                activate);
        }

        if (result) {
            handle->_parent = this;
            handle->_field = field;
            return true;
        }

        return false;
    }

    bool variable::field(
            compiler::element* element,
            variable_handle_t& handle,
            bool activate) {
        compiler::identifier_reference* var = nullptr;
        switch (element->element_type()) {
            case element_type_t::unary_operator: {
                auto unary_op = dynamic_cast<compiler::unary_operator*>(element);
                if (unary_op->operator_type() == operator_type_t::pointer_dereference) {
                    var = dynamic_cast<compiler::identifier_reference*>(unary_op->rhs());
                }
                break;
            }
            case element_type_t::identifier_reference: {
                var = dynamic_cast<compiler::identifier_reference*>(element);
                break;
            }
            default: {
                break;
            }
        }
        return var == nullptr ?
            false :
            field(var->symbol().name, handle, element, activate);
    }

    bool variable::write() {
        if (flag(flags_t::f_written))
            return false;

        address();

        auto block = _session.assembler().current_block();

        auto var = dynamic_cast<compiler::identifier*>(_element);
        auto on_stack = var->usage() == identifier_usage_t::stack;

        auto fmt = "{}";
        if (on_stack)
            fmt = "stack: {}";

        block->comment(
            fmt::format(fmt, _rot.path),
            vm::comment_location_t::after_instruction);

        auto offset = _rot.offset;

        // XXX: total hack, do not keep!
        if (on_stack) {
            offset += 4;
        }

        vm::instruction_operand_t dest_operand(on_stack ?
            vm::register_t::sp() :
            _address.reg);

        auto value_operand = emit_result().operands.back();
        value_operand.size(_value.reg.size);

        block->store(
            dest_operand,
            value_operand,
            vm::instruction_operand_t::offset(offset));

        flag(flags_t::f_written, true);
        flag(flags_t::f_read, false);
        return true;
    }

    bool variable::address(bool include_offset) {
        if (flag(flags_t::f_addressed))
            return false;

        auto& assembler = _session.assembler();
        auto block = assembler.current_block();
        if (block == nullptr)
            return true;

        compiler::identifier* var = nullptr;
        switch (_element->element_type()) {
            case element_type_t::identifier: {
                var = dynamic_cast<compiler::identifier*>(_element);
                break;
            }
            case element_type_t::unary_operator: {
                auto unary_op = dynamic_cast<compiler::unary_operator*>(_element);
                if (unary_op->operator_type() == operator_type_t::pointer_dereference) {
                    auto ref = dynamic_cast<compiler::identifier_reference*>(unary_op->rhs());
                    if (ref != nullptr)
                        var = ref->identifier();
                }
                break;
            }
            default: {
                break;
            }
        }

        if (walk_to_root_and_calculate_offset(_rot)) {
            var = _rot.identifier;
        }

        if (var != nullptr) {
            auto address_reg = _session.get_address_register(var->id());
            if (address_reg == nullptr) {
                _address.reg.size = vm::op_sizes::qword;
                _address.reg.type = vm::register_type_t::integer;
                _address.reserve();

                block->comment(
                    var->symbol()->name(),
                    vm::comment_location_t::after_instruction);

                block->move(
                    vm::instruction_operand_t(_address.reg),
                    vm::instruction_operand_t(assembler.make_label_ref(var->symbol()->name())),
                    vm::instruction_operand_t::offset(!include_offset ? 0 : _rot.offset));
            } else {
                _address.reg = *address_reg;
                _address.allocated = true;
                _address.no_release = true;
            }
        }

        flag(flags_t::f_addressed, true);
        return true;
    }

    bool variable::activate() {
        if (flag(flags_t::f_activated))
            return false;

        flag(flags_t::f_read, false);
        flag(flags_t::f_copied, false);
        flag(flags_t::f_written, false);
        flag(flags_t::f_activated, true);
        flag(flags_t::f_addressed, false);

        _value.reg.type = vm::register_type_t::integer;
        if (_type.inferred_type != nullptr) {
            if (_type.inferred_type->access_model() == type_access_model_t::value) {
                _value.reg.size = vm::op_size_for_byte_size(_type.inferred_type->size_in_bytes());
                if (_type.inferred_type->number_class() == type_number_class_t::floating_point) {
                    _value.reg.type = vm::register_type_t::floating_point;
                }
            } else {
                _value.reg.size = vm::op_sizes::qword;
            }
        }

        _value.reserve();

        return true;
    }

    bool variable::initialize() {
        return _element->infer_type(_session, _type);
    }

    bool variable::deactivate() {
        if (!flag(flags_t::f_activated))
            return false;

        flag(flags_t::f_read, false);
        flag(flags_t::f_copied, false);
        flag(flags_t::f_written, false);
        flag(flags_t::f_activated, false);
        flag(flags_t::f_addressed, false);

        _address.release();
        _value.release();

        _result.clear(_session.assembler());

        return true;
    }

    variable* variable::parent() {
        return _parent;
    }

    compiler::field* variable::field() {
        return _field;
    }

    bool variable::is_activated() const {
        return flag(flags_t::f_activated);
    }

    bool variable::write(
            vm::op_sizes size,
            uint64_t value) {
        if (flag(flags_t::f_written))
            return false;

        address();

        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        block->comment(
            _rot.path,
            vm::comment_location_t::after_instruction);

        block->store(
            vm::instruction_operand_t(_address.reg),
            vm::instruction_operand_t(static_cast<uint64_t>(value), size),
            vm::instruction_operand_t::offset(_rot.offset));

        flag(flags_t::f_written, true);
        return true;
    }

    bool variable::copy(variable* value, uint64_t size_in_bytes) {
        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        address(true);
        value->address(true);

        block->comment(
            _rot.path,
            vm::comment_location_t::after_instruction);

        block->copy(
            vm::op_sizes::byte,
            vm::instruction_operand_t(_address.reg),
            vm::instruction_operand_t(value->_address.reg),
            vm::instruction_operand_t(static_cast<uint64_t>(size_in_bytes)));

        return true;
    }

    bool variable::write(variable* value) {
        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        address();
        value->read();

        auto var = dynamic_cast<compiler::identifier*>(_element);
        auto on_stack = var->usage() == identifier_usage_t::stack;

        auto fmt = "{}";
        if (on_stack)
            fmt = "stack: {}";

        block->comment(
            fmt::format(fmt, _rot.path),
            vm::comment_location_t::after_instruction);

        auto offset = _rot.offset;

        // XXX: total hack, do not keep!
        if (on_stack) {
            offset += 4;
        }

        vm::instruction_operand_t dest_operand(on_stack ?
            vm::register_t::sp() :
            _address.reg);

        auto value_operand = value->emit_result().operands.back();
        value_operand.size(_value.reg.size);

        block->store(
            dest_operand,
            value_operand,
            vm::instruction_operand_t::offset(offset));

        return true;
    }

    compiler::element* variable::element() {
        return _element;
    }

    emit_context_t& variable::emit_context() {
        return _context;
    }

    bool variable::flag(variable::flags_t f) const {
        return (_flags & f) != 0;
    }

    const vm::register_t& variable::value_reg() const {
        return _value.reg;
    }

    const emit_result_t& variable::emit_result() const {
        return _result;
    }

    const vm::register_t& variable::address_reg() const {
        return _address.reg;
    }

    void variable::flag(variable::flags_t f, bool value) {
        if (value)
            _flags |= f;
        else
            _flags &= ~f;
    }

    const infer_type_result_t& variable::type_result() const {
        return _type;
    }

    bool variable::walk_to_root_and_calculate_offset(root_and_offset_t& rot) {
        std::stack<std::string> names {};

        _rot.offset = 0;
        _rot.path.clear();
        _rot.root = nullptr;
        _rot.identifier = nullptr;

        auto current = this;
        if (_parent != nullptr) {
            while (current->_field != nullptr) {
                rot.offset += current->_field->start_offset();
                names.push(current->_field->identifier()->symbol()->name());
                current = current->_parent;
            }
        }

        rot.root = current;
        switch (current->_element->element_type()) {
            case element_type_t::identifier: {
                rot.identifier = dynamic_cast<compiler::identifier*>(current->_element);
                break;
            }
            case element_type_t::unary_operator: {
                auto unary_op = dynamic_cast<compiler::unary_operator*>(current->_element);
                if (unary_op->operator_type() == operator_type_t::pointer_dereference) {
                    auto ref = dynamic_cast<compiler::identifier_reference*>(unary_op->rhs());
                    if (ref != nullptr)
                        rot.identifier = ref->identifier();
                }
                break;
            }
            default: {
                break;
            }
        }

        if (rot.identifier != nullptr) {
            names.push(rot.identifier->symbol()->name());
        }

        while (!names.empty()) {
            if (!rot.path.empty())
                rot.path += ".";
            rot.path += names.top();
            names.pop();
        }

        return true;
    }

};