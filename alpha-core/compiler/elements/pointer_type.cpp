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

#include <compiler/session.h>
#include "program.h"
#include "numeric_type.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    std::string pointer_type::name_for_pointer(compiler::type* base_type) {
        return fmt::format("__ptr_{}__", base_type->symbol()->name());
    }

    ///////////////////////////////////////////////////////////////////////////

    pointer_type::pointer_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* base_type) : compiler::type(
                                                        module,
                                                        parent_scope,
                                                        element_type_t::pointer_type,
                                                        nullptr),
                                                   _base_type_ref(base_type) {
    }

    bool pointer_type::is_composite_type() const {
        return _base_type_ref->type()->is_composite_type();
    }

    bool pointer_type::on_type_check(compiler::type* other) {
        if (other == nullptr)
            return false;

        switch (other->element_type()) {
            case element_type_t::numeric_type: {
                auto numeric_type = dynamic_cast<compiler::numeric_type*>(other);
                return numeric_type->size_in_bytes() == 8;
            }
            case element_type_t::pointer_type: {
                auto other_pointer_type = dynamic_cast<compiler::pointer_type*>(other);
                return _base_type_ref->type()->id() == other_pointer_type->base_type_ref()->type()->id();
            }
            default:
                return false;
        }
    }

    type_number_class_t pointer_type::on_number_class() const {
        return type_number_class_t::integer;
    }

    type_access_model_t pointer_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    bool pointer_type::on_initialize(compiler::session& session) {
        auto type_symbol = session.builder().make_symbol(
            parent_scope(),
            name_for_pointer(_base_type_ref->type()));
        symbol(type_symbol);
        type_symbol->parent_element(this);
        size_in_bytes(sizeof(uint64_t));
        return true;
    }

    compiler::type_reference* pointer_type::base_type_ref() const {
        return _base_type_ref;
    }

    std::string pointer_type::name(const std::string& alias) const {
        return fmt::format(
            "^{}",
            !alias.empty() ? alias : _base_type_ref->name());
    }

};
