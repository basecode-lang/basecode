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
#include <compiler/element_builder.h>
#include <compiler/type_name_builder.h>
#include "generic_type.h"
#include "type_reference.h"
#include "symbol_element.h"

namespace basecode::compiler {

    std::string generic_type::name_for_generic_type(
            const type_reference_list_t& constraints) {
        type_name_builder builder{};
        builder.add_part("generic");
        for (auto c : constraints)
            builder.add_part(std::string(c->symbol()->name()));
        return builder.format();
    }

    generic_type::generic_type(
            compiler::module* module,
            compiler::block* parent_scope,
            const compiler::type_reference_list_t& constraints) : compiler::type(
                                                                       module,
                                                                       parent_scope,
                                                                       element_type_t::generic_type,
                                                                       nullptr),
                                                                  _constraints(constraints) {
    }

    bool generic_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        if (other != nullptr
        &&  other->element_type() == element_type_t::generic_type) {
            auto other_generic = dynamic_cast<compiler::generic_type*>(other);
            if (is_open() && other_generic->is_open())
                return true;
            else {
                // XXX: very tentative equality logic
                auto other_constraints = other_generic->constraints();

                if (other_constraints.size() != _constraints.size())
                    return false;

                for (size_t i = 0; i < _constraints.size(); i++) {
                    if (_constraints[i]->type() != other_constraints[i]->type())
                        return false;
                }

                return true;
            }
        }

        if (is_open())
            return true;

        for (auto constraint : _constraints) {
            if (constraint->type()->type_check(other, options))
                return true;
        }

        return false;
    }

    bool generic_type::is_open() const {
        return _constraints.empty();
    }

    bool generic_type::is_open_generic_type() const {
        return is_open();
    }

    bool generic_type::on_initialize(compiler::session& session) {
        auto it = session.strings().insert(name_for_generic_type(_constraints));
        symbol(session.builder().make_symbol(parent_scope(), *it.first));
        return true;
    }

    const compiler::type_reference_list_t& generic_type::constraints() const {
        return _constraints;
    }

}