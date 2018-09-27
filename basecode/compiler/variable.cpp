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

//    bool variable_t::init(compiler::session& session) {
//        if (!live)
//            return false;
//
//        if (address_loaded)
//            return true;
//
//        if (usage == identifier_usage_t::heap) {
//            if (!address_reg.reserve())
//                return false;
//
//            auto& assembler = session.assembler();
//            auto block = assembler.current_block();
//
//            block->comment(
//                fmt::format(
//                    "identifier '{}' address (global)",
//                    name),
//                4);
//
//            auto label_ref = assembler.make_label_ref(name);
//            block->move_label_to_reg(address_reg.reg, label_ref);
//        }
//
//        value_reg.reg.type = vm::register_type_t::integer;
//        if (type != nullptr) {
//            if (type->access_model() == type_access_model_t::value) {
//                value_reg.reg.size = vm::op_size_for_byte_size(type->size_in_bytes());
//                if (type->number_class() == type_number_class_t::floating_point) {
//                    value_reg.reg.type = vm::register_type_t::floating_point;
//                }
//            } else {
//                value_reg.reg.size = vm::op_sizes::qword;
//            }
//        }
//
//        address_loaded = true;
//
//        return true;
//    }
//
//    bool variable_t::read(compiler::session& session) {
//        if (!live)
//            return false;
//
//        if (!init(session))
//            return false;
//
//        std::string type_name = "global";
//        if (requires_read) {
//            if (!value_reg.reserve())
//                return false;
//
//            auto& assembler = session.assembler();
//            auto block = assembler.current_block();
//
//            block->comment(
//                fmt::format(
//                    "load identifier '{}' value ({})",
//                    name,
//                    type_name),
//                4);
//
//            if (value_reg.reg.size != vm::op_sizes::qword)
//                block->clr(vm::op_sizes::qword, value_reg.reg);
//
//            if (usage == identifier_usage_t::stack) {
//                type_name = stack_frame_entry_type_name(frame_entry->type);
//                block->load_to_reg(
//                    value_reg.reg,
//                    vm::register_t::fp(),
//                    frame_entry->offset);
//            } else {
//                block->load_to_reg(value_reg.reg, address_reg.reg);
//            }
//
//            requires_read = false;
//        }
//
//        return true;
//    }
//
//    bool variable_t::write(compiler::session& session) {
//        auto& assembler = session.assembler();
//        auto block = assembler.current_block();
//
//        auto target_reg = assembler.current_target_register();
//        if (target_reg == nullptr)
//            return false;
//
//        block->store_from_reg(
//            address_reg.reg,
//            *target_reg,
//            frame_entry != nullptr ? frame_entry->offset : 0);
//
//        written = true;
//        requires_read = true;
//
//        return true;
//    }
//

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

        switch (_element->element_type()) {
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(_element);
                if (!flag(flags_t::f_addressed)) {
                    block->comment(
                        fmt::format(
                            "load global address: {}",
                            var->symbol()->name()),
                        4);

                    auto label_ref = assembler.make_label_ref(var->symbol()->name());
                    block->move_label_to_reg(_address.reg, label_ref);

                    flag(flags_t::f_addressed, true);
                }

                block->comment(
                    fmt::format(
                        "load global value: {}",
                        var->symbol()->name()),
                    4);

                if (_value.reg.size != vm::op_sizes::qword)
                    block->clr(vm::op_sizes::qword, _value.reg);

                block->load_to_reg(_value.reg, _address.reg);
                break;
            }
            default: {
                assembler.push_target_register(_address.reg);
                _element->emit(_session);
                assembler.pop_target_register();
                break;
            }
        }

        flag(flags_t::f_read, true);
        flag(flags_t::f_written, false);
        return true;
    }

    bool variable::write() {
        if (flag(flags_t::f_written))
            return false;

        auto& assembler = _session.assembler();
        auto block = _session.assembler().current_block();
        block->store_from_reg(
            _address.reg,
            *(assembler.current_target_register()));
        flag(flags_t::f_written, true);
        flag(flags_t::f_read, false);
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
        flag(flags_t::f_activated, false);
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
        auto block = _session.assembler().current_block();
        block->move_constant_to_reg(_value.reg, value);
        block->store_from_reg(_address.reg, _value.reg);
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

    variable* variable::field(const std::string& name) {
        if (!_type.inferred_type->is_composite_type())
            return nullptr;
        auto type = dynamic_cast<compiler::composite_type*>(_type.inferred_type);
        auto field = type->fields().find_by_name(name);
        if (field == nullptr)
            return nullptr;
        auto var = _session.variable(field->identifier());
        if (var->_parent == nullptr) {
            var->_parent = this;
            var->_field = field;
        }
        return var;
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