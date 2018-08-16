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

#include <fmt/format.h>
#include <common/id_pool.h>
#include <compiler/session.h>
#include "type.h"
#include "element.h"
#include "program.h"
#include "attribute.h"
#include "float_literal.h"
#include "string_literal.h"
#include "integer_literal.h"
#include "boolean_literal.h"

namespace basecode::compiler {

    element::element(
            compiler::module* module,
            block* parent_scope,
            element_type_t type,
            element* parent_element) : _id(common::id_pool::instance()->allocate()),
                                       _parent_scope(parent_scope),
                                       _parent_element(parent_element),
                                       _module(module),
                                       _element_type(type) {
    }

    element::~element() {
    }

    block* element::parent_scope() {
        return _parent_scope;
    }

    bool element::is_type() const {
        switch (_element_type) {
            case element_type_t::any_type:
            case element_type_t::proc_type:
            case element_type_t::bool_type:
            case element_type_t::type_info:
            case element_type_t::alias_type:
            case element_type_t::array_type:
            case element_type_t::tuple_type:
            case element_type_t::string_type:
            case element_type_t::module_type:
            case element_type_t::numeric_type:
            case element_type_t::pointer_type:
            case element_type_t::composite_type:
            case element_type_t::namespace_type:
                return true;
            default:
                return false;
        }
    }

    common::id_t element::id() const {
        return _id;
    }

    bool element::is_constant() const {
        return on_is_constant();
    }

    element* element::parent_element() {
        return _parent_element;
    }

    compiler::module* element::module() {
        return _module;
    }

    bool element::on_is_constant() const {
        return false;
    }

    attribute_map_t& element::attributes() {
        return _attributes;
    }

    std::string element::label_name() const {
        return fmt::format(
            "{}_{}",
            element_type_name(_element_type),
            _id);
    }

    bool element::as_bool(bool& value) const {
        return on_as_bool(value);
    }

    element_register_t element::register_for(
            compiler::session& session,
            element* e) {
        element_register_t result {
            .session = &session
        };

        auto var = session.emit_context().variable_for_element(e);
        if (var != nullptr) {
            var->make_live(session);

            result.var = var;
            result.var->read(
                session,
                session.assembler().current_block());
            result.valid = true;
            result.reg = var->value_reg.reg;

            if (var->address_reg.allocated
            &&  var->type->access_model() == type_access_model_t::pointer) {
                result.reg = var->address_reg.reg;
            } else {
                result.reg = result.var->value_reg.reg;
            }
        } else {
            auto type = e->infer_type(session);

            vm::register_t reg;
            reg.size = vm::op_size_for_byte_size(type->size_in_bytes());

            if (type->number_class() == type_number_class_t::floating_point)
                reg.type = vm::register_type_t::floating_point;
            else
                reg.type = vm::register_type_t::integer;

            if (!session.assembler().allocate_reg(reg)) {
                session.error(
                    e,
                    "P052",
                    "assembler registers exhausted.",
                    e->location());
            } else {
                result.reg = reg;
                result.valid = true;
                result.clean_up = true;
            }
        }

        return result;
    }

    bool element::on_as_bool(bool& value) const {
        return false;
    }

    bool element::as_float(double& value) const {
        return on_as_float(value);
    }

    void element::parent_element(element* value) {
        _parent_element = value;
    }

    element_type_t element::element_type() const {
        return _element_type;
    }

    void element::module(compiler::module* value) {
        _module = value;
    }

    bool element::emit(compiler::session& session) {
        return on_emit(session);
    }

    bool element::on_as_float(double& value) const {
        return false;
    }

    bool element::as_integer(uint64_t& value) const {
        return on_as_integer(value);
    }

    bool element::as_string(std::string& value) const {
        return on_as_string(value);
    }

    bool element::on_emit(compiler::session& session) {
        return true;
    }

    element* element::fold(compiler::session& session) {
        auto no_fold_attribute = find_attribute("no_fold");
        if (no_fold_attribute != nullptr)
            return nullptr;
        return on_fold(session);
    }

    bool element::on_as_integer(uint64_t& value) const {
        return false;
    }

    void element::owned_elements(element_list_t& list) {
        on_owned_elements(list);
    }

    bool element::on_as_string(std::string& value) const {
        return false;
    }

    bool element::is_parent_element(element_type_t type) {
        if (_parent_element == nullptr)
            return false;
        return _parent_element->element_type() == type;
    }

    element* element::on_fold(compiler::session& session) {
        return nullptr;
    }

    void element::on_owned_elements(element_list_t& list) {
    }

    const common::source_location& element::location() const {
        return _location;
    }

    attribute* element::find_attribute(const std::string& name) {
        auto current_element = this;
        while (current_element != nullptr) {
            auto attr = current_element->_attributes.find(name);
            if (attr != nullptr)
                return attr;
            current_element = current_element->parent_element();
        }
        return nullptr;
    }

    void element::location(const common::source_location& location) {
        _location = location;
    }

    compiler::type* element::infer_type(const compiler::session& session) {
        if (is_type())
            return dynamic_cast<compiler::type*>(this);
        return on_infer_type(session);
    }

    compiler::type* element::on_infer_type(const compiler::session& session) {
        return nullptr;
    }

};