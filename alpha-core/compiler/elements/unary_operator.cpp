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
#include "identifier.h"
#include "pointer_type.h"
#include "unary_operator.h"
#include "type_reference.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    unary_operator::unary_operator(
            compiler::module* module,
            block* parent_scope,
            operator_type_t type,
            element* rhs) : operator_base(module, parent_scope, element_type_t::unary_operator, type),
                            _rhs(rhs) {
    }

    bool unary_operator::on_fold(
            compiler::session& session,
            fold_result_t& result) {
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

        return true;
    }

    element* unary_operator::rhs() {
        return _rhs;
    }

    bool unary_operator::on_is_constant() const {
        return _rhs != nullptr && _rhs->is_constant();
    }

    void unary_operator::rhs(compiler::element* element) {
        _rhs = element;
    }

    bool unary_operator::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();

        auto block = assembler.current_block();
        auto target_reg = assembler.current_target_register();

        auto rhs_reg = register_for(session, _rhs);
        if (!rhs_reg.valid)
            return false;

        assembler.push_target_register(rhs_reg.reg);
        _rhs->emit(session);
        assembler.pop_target_register();

        switch (operator_type()) {
            case operator_type_t::negate: {
                block->neg(*target_reg, rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_not: {
                block->not_op(*target_reg, rhs_reg.reg);
                break;
            }
            case operator_type_t::logical_not: {
                block->cmp(target_reg->size, rhs_reg.reg, 1);
                block->setnz(*target_reg);
                break;
            }
            case operator_type_t::pointer_dereference: {
                block->comment("XXX: implement pointer dereference", 4);
                block->nop();
                break;
            }
            default:
                break;
        }

        return true;
    }

    void unary_operator::on_owned_elements(element_list_t& list) {
        if (_rhs != nullptr)
            list.emplace_back(_rhs);
    }

    bool unary_operator::on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) {
        auto& scope_manager = session.scope_manager();
        switch (operator_type()) {
            case operator_type_t::negate:
            case operator_type_t::binary_not: {
                result.inferred_type = scope_manager.find_type({.name = "u64"});
                return true;
            }
            case operator_type_t::logical_not: {
                result.inferred_type = scope_manager.find_type({.name = "bool"});
                return true;
            }
            case operator_type_t::pointer_dereference: {
                auto identifier_ref = dynamic_cast<compiler::identifier_reference*>(_rhs);
                auto type_ref = identifier_ref->identifier()->type_ref();
                if (type_ref->type()->element_type() != element_type_t::pointer_type) {
                    return false;
                }
                auto type = dynamic_cast<compiler::pointer_type*>(type_ref->type());
                result.inferred_type = type->base_type_ref()->type();
                result.reference = type->base_type_ref();
                return true;
            }
            default: {
                return false;
            }
        }
    }

};