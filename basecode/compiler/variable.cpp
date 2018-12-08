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

                root_and_offset_t rot {};
                if (walk_to_root_and_calculate_offset(rot)) {
                    block->comment(
                        fmt::format(
                            "load field value: {}",
                            rot.path),
                        4);
                } else {
                    if (on_stack) {
                        block->comment(
                            fmt::format(
                                "load stack local: {}",
                                var->symbol()->name()),
                            4);
                        // XXX: total hack, do not keep!
                        rot.offset += 4;
                    } else {
                        block->comment(
                            fmt::format(
                                "load global value: {}",
                                var->symbol()->name()),
                            4);
                    }
                }

                vm::instruction_operand_t result_operand(_value.reg);
                _result.operands.emplace_back(result_operand);

                if (_value.reg.size != vm::op_sizes::qword)
                    block->clr(vm::op_sizes::qword, result_operand);

                block->load(
                    result_operand,
                    vm::instruction_operand_t(on_stack ? vm::register_t::sp() : _address.reg),
                    rot.offset != 0 ?
                        vm::instruction_operand_t(rot.offset, vm::op_sizes::qword) :
                        vm::instruction_operand_t::empty());
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

        root_and_offset_t rot {};
        if (walk_to_root_and_calculate_offset(rot)) {
            block->comment(
                fmt::format(
                    "store field value: {}",
                    rot.path),
                4);
        } else {
            block->comment(
                fmt::format(
                    "store global value: {}",
                    var->symbol()->name()),
                4);
        }

        // XXX: total hack, do not keep!
        if (on_stack) {
            rot.offset += 4;
        }

        vm::instruction_operand_t dest_operand(
            on_stack ? vm::register_t::sp() : _address.reg);
        vm::instruction_operand_t offset_operand(rot.offset);

        block->store(
            dest_operand,
            emit_result().operands.back(),
            rot.offset != 0 ? offset_operand : vm::instruction_operand_t::empty());

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
        if (_element->element_type() == element_type_t::identifier) {
            var = dynamic_cast<compiler::identifier*>(_element);
        }

        root_and_offset_t rot {};
        if (walk_to_root_and_calculate_offset(rot)) {
            var = rot.identifier;
        }

        if (var != nullptr) {
            if (var->usage() == identifier_usage_t::stack) {
                block->comment(
                    fmt::format(
                        "stack local: {}",
                        var->symbol()->name()),
                    4);
            } else {
                block->comment(
                    fmt::format(
                        "load global address: {}",
                        var->symbol()->name()),
                    4);
                vm::instruction_operand_t offset_operand(rot.offset);

                block->move(
                    vm::instruction_operand_t(_address.reg),
                    vm::instruction_operand_t(assembler.make_label_ref(var->symbol()->name())),
                    !include_offset ? vm::instruction_operand_t::empty() : offset_operand);
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

        _address.reg.size = vm::op_sizes::qword;
        _address.reg.type = vm::register_type_t::integer;
        _address.reserve();

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
        if (!_element->infer_type(_session, _type))
            return false;

        return true;
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

    bool variable::write(uint64_t value) {
        if (flag(flags_t::f_written))
            return false;

        address();

        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        root_and_offset_t rot {};
        if (walk_to_root_and_calculate_offset(rot)) {
            block->comment(
                fmt::format(
                    "store field value: {}",
                    rot.path),
                4);
        } else {
            auto var = dynamic_cast<compiler::identifier*>(_element);
            block->comment(
                fmt::format(
                    "store global value: {}",
                    var->symbol()->name()),
                4);
        }

        vm::instruction_operand_t offset_operand(rot.offset);

        block->store(
            vm::instruction_operand_t(_address.reg),
            vm::instruction_operand_t(static_cast<uint64_t>(value), vm::op_sizes::qword),
            rot.offset != 0 ? offset_operand : vm::instruction_operand_t::empty());

        flag(flags_t::f_written, true);
        return true;
    }

    bool variable::copy(variable* value, uint64_t size_in_bytes) {
        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        address(true);
        value->address(true);

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

        root_and_offset_t rot {};
        if (walk_to_root_and_calculate_offset(rot)) {
            block->comment(
                fmt::format(
                    "store field value: {}",
                    rot.path),
                4);
        } else {
            block->comment(
                fmt::format(
                    "store global value: {}",
                    var->symbol()->name()),
                4);
        }

        // XXX: total hack, do not keep!
        if (on_stack) {
            rot.offset += 4;
        }

        vm::instruction_operand_t dest_operand(on_stack ?
            vm::register_t::sp() :
            _address.reg);
        vm::instruction_operand_t offset_operand(rot.offset);

        block->store(
            dest_operand,
            value->emit_result().operands.back(),
            rot.offset != 0 ? offset_operand : vm::instruction_operand_t::empty());

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
        if (_parent == nullptr)
            return false;

        std::stack<std::string> names {};

        auto current = this;
        while (current->_field != nullptr) {
            rot.offset += current->_field->start_offset();
            names.push(current->_field->identifier()->symbol()->name());
            current = current->_parent;
        }

        rot.root = current;
        rot.identifier = dynamic_cast<compiler::identifier*>(current->_element);
        while (!names.empty()) {
            if (!rot.path.empty())
                rot.path += ".";
            rot.path += names.top();
            names.pop();
        }

        return true;
    }

};