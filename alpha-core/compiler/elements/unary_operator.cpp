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
#include <vm/instruction_block.h>
#include "type.h"
#include "program.h"
#include "unary_operator.h"

namespace basecode::compiler {

    unary_operator::unary_operator(
            compiler::module* module,
            block* parent_scope,
            operator_type_t type,
            element* rhs) : operator_base(module, parent_scope, element_type_t::unary_operator, type),
                            _rhs(rhs) {
    }

    bool unary_operator::on_emit(compiler::session& session) {
        auto instruction_block = session.assembler().current_block();
        auto target_reg = session.assembler().current_target_register();

        auto rhs_reg = register_for(session, _rhs);
        if (!rhs_reg.valid)
            return false;

        session.assembler().push_target_register(rhs_reg.reg);
        _rhs->emit(session);
        session.assembler().pop_target_register();

        switch (operator_type()) {
            case operator_type_t::negate: {
                instruction_block->neg(*target_reg, rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_not: {
                instruction_block->not_op(*target_reg, rhs_reg.reg);
                break;
            }
            case operator_type_t::logical_not: {
                instruction_block->xor_reg_by_reg(
                    *target_reg,
                    rhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            default:
                break;
        }

        return true;
    }

    element* unary_operator::rhs() {
        return _rhs;
    }

    bool unary_operator::on_is_constant() const {
        return _rhs != nullptr && _rhs->is_constant();
    }

    element* unary_operator::on_fold(compiler::session& session) {
        switch (operator_type()) {
            case operator_type_t::negate: {
                break;
            }
            case operator_type_t::binary_not: {
                break;
            }
            case operator_type_t::logical_not: {
                break;
            }
            default:
                break;
        }

        return nullptr;
    }

    void unary_operator::on_owned_elements(element_list_t& list) {
        if (_rhs != nullptr)
            list.emplace_back(_rhs);
    }

    // XXX: this requires lots of future love
    compiler::type* unary_operator::on_infer_type(const compiler::program* program) {
        switch (operator_type()) {
            case operator_type_t::negate:
            case operator_type_t::binary_not: {
                return program->find_type({.name = "u64"});
            }
            case operator_type_t::logical_not: {
                return program->find_type({.name = "bool"});
            }
            default:
                return nullptr;
        }
    }

};