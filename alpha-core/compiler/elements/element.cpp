// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <fmt/format.h>
#include <common/id_pool.h>
#include "type.h"
#include "element.h"
#include "float_literal.h"
#include "string_literal.h"
#include "integer_literal.h"
#include "boolean_literal.h"

namespace basecode::compiler {

    element::element(
            element* parent,
            element_type_t type) : _id(common::id_pool::instance()->allocate()),
                                   _parent(parent),
                                   _element_type(type) {
    }

    element::~element() {
    }

    element* element::parent() {
        return _parent;
    }

    common::id_t element::id() const {
        return _id;
    }

    bool element::is_constant() const {
        return on_is_constant();
    }

    bool element::on_is_constant() const {
        return false;
    }

    bool element::fold(common::result& r) {
        return on_fold(r);
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

    bool element::on_fold(common::result& r) {
        return true;
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

    element_type_t element::element_type() const {
        return _element_type;
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

    bool element::on_as_integer(uint64_t& value) const {
        return false;
    }

    bool element::on_as_string(std::string& value) const {
        return false;
    }

    bool element::emit(
            common::result& r,
            emit_context_t& context) {
        return on_emit(r, context);
    }

    bool element::on_emit(
            common::result& r,
            emit_context_t& context) {
        return true;
    }

    compiler::type* element::infer_type(const compiler::program* program) {
        switch (_element_type) {
            case element_type_t::any_type:
            case element_type_t::proc_type:
            case element_type_t::bool_type:
            case element_type_t::alias_type:
            case element_type_t::array_type:
            case element_type_t::string_type:
            case element_type_t::numeric_type:
            case element_type_t::composite_type:
            case element_type_t::namespace_type:
                return dynamic_cast<compiler::type*>(this);
            default:
                return on_infer_type(program);
        }
    }

    compiler::type* element::on_infer_type(const compiler::program* program) {
        return nullptr;
    }

};