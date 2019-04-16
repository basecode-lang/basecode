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
#include "directive.h"
#include "unary_operator.h"
#include "type_reference.h"

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
            if (_element_type == element_type_t::type_reference) {
                auto ref = dynamic_cast<compiler::type_reference*>(this);
                result.types.emplace_back(ref->type(), ref);
            } else {
                result.types.emplace_back(
                    dynamic_cast<compiler::type*>(this),
                    nullptr);
            }
            return true;
        }
        return on_infer_type(session, result);
    }

    bool element::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        return false;
    }

    bool element::is_type() const {
        switch (_element_type) {
            case element_type_t::proc_type:
            case element_type_t::bool_type:
            case element_type_t::rune_type:
            case element_type_t::array_type:
            case element_type_t::tuple_type:
            case element_type_t::family_type:
            case element_type_t::module_type:
            case element_type_t::generic_type:
            case element_type_t::numeric_type:
            case element_type_t::pointer_type:
            case element_type_t::composite_type:
            case element_type_t::namespace_type:
            case element_type_t::type_reference:
                return true;
            default:
                return false;
        }
    }

    bool element::apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        return on_apply_fold_result(e, fold_result);
    }

    void element::make_non_owning() {
        _non_owning = true;
    }

    element_id_set_t& element::ids() {
        return _ids;
    }

    bool element::non_owning() const {
        return _non_owning;
    }

    common::id_t element::id() const {
        return _id;
    }

    bool element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        return false;
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

    block* element::parent_scope() const {
        return _parent_scope;
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

    bool element::is_pointer_dereference() const {
        if (_element_type != element_type_t::unary_operator)
            return false;

        auto unary_op = dynamic_cast<compiler::unary_operator*>(const_cast<compiler::element*>(this));
        return unary_op->operator_type() == operator_type_t::pointer_dereference;
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

    compiler::element* element::clone(compiler::session& session) {
        if (is_singleton())
            return this;
        return on_clone(session);
    }

    bool element::on_less_than_or_equal(const element& other) const {
        return false;
    }

    void element::location(const common::source_location& location) {
        _location = location;
    }

    bool element::as_identifier(compiler::identifier*& value) const {
        return on_as_identifier(value);
    }

    bool element::is_directive_of_type(directive_type_t type) const {
        return _element_type == element_type_t::directive
               && dynamic_cast<const compiler::directive*>(this)->type() == type;
    }

    compiler::element* element::on_clone(compiler::session& session) {
        return nullptr;
    }

    bool element::on_greater_than_or_equal(const element& other) const {
        return false;
    }

    bool element::on_as_identifier(compiler::identifier*& value) const {
        return false;
    }

    bool element::is_type_one_of(const element_type_set_t& types) const {
        return types.count(_element_type) > 0;
    }

    bool element::is_parent_type_one_of(const element_type_set_t& types) const {
        if (_parent_element == nullptr)
            return false;
        return _parent_element->is_type_one_of(types);
    }

}