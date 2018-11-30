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

    bool element::fold(
            compiler::session& session,
            fold_result_t& result) {
        if (result.allow_no_fold_attribute) {
            auto no_fold_attribute = find_attribute("no_fold");
            if (no_fold_attribute != nullptr)
                return true;
        }
        return on_fold(session, result);
    }

    bool element::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        return true;
    }

    bool element::infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (is_type()) {
            result.inferred_type = dynamic_cast<compiler::type*>(this);
            return true;
        }
        return on_infer_type(session, result);
    }

    bool element::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        return false;
    }

    block* element::parent_scope() {
        return _parent_scope;
    }

    bool element::is_type() const {
        switch (_element_type) {
            case element_type_t::any_type:
            case element_type_t::map_type:
            case element_type_t::proc_type:
            case element_type_t::bool_type:
            case element_type_t::type_info:
            case element_type_t::rune_type:
            case element_type_t::array_type:
            case element_type_t::tuple_type:
            case element_type_t::string_type:
            case element_type_t::module_type:
            case element_type_t::generic_type:
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

    bool element::is_singleton() const {
        return false;
    }

    compiler::module* element::module() {
        return _module;
    }

    comment_list_t& element::comments() {
        return _comments;
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

    bool element::as_rune(common::rune_t& value) const {
        return on_as_rune(value);
    }

    bool element::on_as_integer(uint64_t& value) const {
        return false;
    }

    void element::owned_elements(element_list_t& list) {
        on_owned_elements(list);
    }

    bool element::on_equals(const element& other) const {
        return false;
    }

    bool element::on_as_string(std::string& value) const {
        return false;
    }

    bool element::is_parent_element(element_type_t type) {
        if (_parent_element == nullptr)
            return false;
        return _parent_element->element_type() == type;
    }

    uint64_t element::on_add(const element& other) const {
        return 0;
    }

    bool element::on_as_rune(common::rune_t& value) const {
        return false;
    }

    void element::on_owned_elements(element_list_t& list) {
    }

    bool element::on_less_than(const element& other) const {
        return false;
    }

    bool element::on_not_equals(const element& other) const {
        return false;
    }

    const common::source_location& element::location() const {
        return _location;
    }

    uint64_t element::on_subtract(const element& other) const {
        return 0;
    }

    uint64_t element::on_multiply(const element& other) const {
        return 0;
    }

    bool element::on_greater_than(const element& other) const {
        return false;
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

    bool element::on_less_than_or_equal(const element& other) const {
        return false;
    }

    void element::location(const common::source_location& location) {
        _location = location;
    }

    bool element::on_greater_than_or_equal(const element& other) const {
        return false;
    }

};