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
#include "identifier.h"
#include "symbol_element.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    identifier_reference::identifier_reference(
            compiler::module* module,
            block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier) : element(module, parent_scope, element_type_t::identifier_reference),
                                                _symbol(symbol),
                                                _identifier(identifier) {
    }

    bool identifier_reference::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (_identifier != nullptr)
            return _identifier->infer_type(session, result);
        return false;
    }

    bool identifier_reference::resolved() const {
        return _identifier != nullptr;
    }

    bool identifier_reference::on_is_constant() const {
        if (_identifier == nullptr)
            return false;
        return _identifier->is_constant();
    }

    bool identifier_reference::on_as_bool(bool& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_bool(value);
    }

    compiler::identifier* identifier_reference::identifier() {
        return _identifier;
    }

    bool identifier_reference::on_as_float(double& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_float(value);
    }

    const qualified_symbol_t& identifier_reference::symbol() const {
        return _symbol;
    }

    bool identifier_reference::on_emit(compiler::session& session) {
//        if (_identifier == nullptr)
//            return false;
//        if (_identifier->emit(session)) {
//            auto var = session.variable_for_element(_identifier);
//            if (var == nullptr) {
//                session.error(
//                    _identifier,
//                    "P051",
//                    fmt::format(
//                        "missing assembler variable for {}.",
//                        _identifier->label_name()),
//                    _identifier->location());
//                return false;
//            }
//            defer({
//                var->make_dormant(session);
//            });
//
//            var->make_live(session);
//            if (var->read(session)) {
//                auto target_reg = session.assembler().current_target_register();
//                if (!var->value_reg.matches(target_reg)) {
//                    auto block = session.assembler().current_block();
//                    block->move_reg_to_reg(*target_reg, var->value_reg.reg);
//                }
//                return true;
//            }
//        }
        return true;
    }

    bool identifier_reference::on_as_integer(uint64_t& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_integer(value);
    }

    bool identifier_reference::on_as_string(std::string& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_string(value);
    }

    void identifier_reference::identifier(compiler::identifier* value) {
        _identifier = value;
    }

    bool identifier_reference::on_as_rune(common::rune_t& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_rune(value);
    }

};