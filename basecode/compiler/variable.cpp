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

        address();

        if (_parent != nullptr) {
            if (_parent->_type.inferred_type->is_pointer_type()) {
                _parent->read();
            }
        }

        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        switch (_element->element_type()) {
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(_element);
                if (_field == nullptr) {
                    block->comment(
                        fmt::format(
                            "load global value: {}",
                            var->symbol()->name()),
                        4);
                } else {
                    block->comment(
                        fmt::format(
                            "load field value: {}",
                            var->symbol()->name()),
                        4);
                }

                if (_value.reg.size != vm::op_sizes::qword)
                    block->clr(vm::op_sizes::qword, _value.reg);

                block->load_to_reg(
                    _value.reg,
                    //_field != nullptr ? _parent->_address.reg : _address.reg,
                    _address.reg);
                    //_field != nullptr ? _field->start_offset() : 0);
                break;
            }
            default: {
                assembler.push_target_register(_value.reg);
                _element->emit(_session);
                assembler.pop_target_register();
                break;
            }
        }

        flag(flags_t::f_read, true);
        flag(flags_t::f_written, false);
        return true;
    }

    bool variable::field(
            const std::string& name,
            variable_handle_t& handle) {
        if (!_type.inferred_type->is_composite_type())
            return false;

        auto type = dynamic_cast<compiler::composite_type*>(_type.inferred_type);
        auto field = type->fields().find_by_name(name);
        if (field == nullptr)
            return false;

        if (_session.variable(field->identifier(), handle)) {
            handle->_parent = this;
            handle->_field = field;
            return true;
        }

        return false;
    }

    bool variable::field(
            compiler::element* element,
            variable_handle_t& handle) {
        auto var = dynamic_cast<compiler::identifier_reference*>(element);
        return field(var->symbol().name, handle);
    }

    bool variable::write() {
        if (flag(flags_t::f_written))
            return false;

        address();

        auto& assembler = _session.assembler();
        auto block = _session.assembler().current_block();

        auto target_register = assembler.current_target_register();
        if (target_register == nullptr) {
            target_register = &_value.reg;
        }

        block->store_from_reg(
            _address.reg,
            //_field != nullptr ? _parent->_address.reg : _address.reg,
            *target_register);
            //_field != nullptr ? _field->start_offset() : 0);

        flag(flags_t::f_written, true);
        flag(flags_t::f_read, false);
        return true;
    }

    bool variable::address() {
        if (flag(flags_t::f_addressed))
            return false;

        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        compiler::identifier* var = nullptr;
        if (_element->element_type() == element_type_t::identifier) {
            var = dynamic_cast<compiler::identifier*>(_element);
        }

        if (_parent != nullptr) {
            _parent->address();
            block->comment(
                fmt::format(
                    "load field address with offset: {}",
                    var->symbol()->name()),
                4);
            block->move_reg_to_reg(
                _address.reg,
                _parent->_address.reg,
                _field->start_offset());
        } else {
            if (var != nullptr) {
                block->comment(
                    fmt::format(
                        "load global address: {}",
                        var->symbol()->name()),
                    4);
                auto label_ref = assembler.make_label_ref(var->symbol()->name());
                block->move_label_to_reg(_address.reg, label_ref);
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

        auto block = _session.assembler().current_block();
        block->move_constant_to_reg(_value.reg, value);
        block->store_from_reg(
            _address.reg,
            //_field != nullptr ? _parent->_address.reg : _address.reg,
            _value.reg);
            //_field != nullptr ? _field->start_offset() : 0);

        flag(flags_t::f_written, true);
        return true;
    }

    bool variable::write(variable* value) {
        auto& assembler = _session.assembler();
        auto block = assembler.current_block();

        address();
        value->read();
        block->store_from_reg(_address.reg, value->value_reg());

        return true;
    }

    compiler::element* variable::element() {
        return _element;
    }

    bool variable::flag(variable::flags_t f) const {
        return (_flags & f) != 0;
    }

    const vm::register_t& variable::value_reg() const {
        return _value.reg;
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

};