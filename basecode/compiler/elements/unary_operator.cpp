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
#include "float_literal.h"
#include "unary_operator.h"
#include "type_reference.h"
#include "integer_literal.h"
#include "boolean_literal.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    unary_operator::unary_operator(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::operator_type_t type,
            compiler::element* rhs) : operator_base(module, parent_scope, element_type_t::unary_operator, type),
                                      _rhs(rhs) {
    }

    bool unary_operator::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        if (!is_constant())
            return false;

        auto builder = session.builder();
        auto scope_manager = session.scope_manager();

        infer_type_result_t type_result {};
        if (!infer_type(session, type_result))
            return false;

        if (type_result.inferred_type->number_class() == type_number_class_t::integer) {
            uint64_t value;
            if (as_integer(value)) {
                result.element = builder.make_integer(
                    scope_manager.current_scope(),
                    value);
                return true;
            }
        } else if (type_result.inferred_type->number_class() == type_number_class_t::floating_point) {
            double value;
            if (as_float(value)) {
                result.element = builder.make_float(
                    scope_manager.current_scope(),
                    value);
                return true;
            }
        }

        bool bool_value;
        if (as_bool(bool_value)) {
            result.element = bool_value ?
                builder.true_literal() :
                builder.false_literal();
            return true;
        }

        return false;
    }

    compiler::element* unary_operator::rhs() {
        return _rhs;
    }

    bool unary_operator::on_is_constant() const {
        return _rhs != nullptr && _rhs->is_constant();
    }

    bool unary_operator::on_as_bool(bool& value) const {
        value = false;
        switch (operator_type()) {
            case operator_type_t::logical_not: {
                bool rhs_value;
                _rhs->as_bool(rhs_value);
                value = !rhs_value;
                break;
            }
            default:
                return false;
        }
        return true;
    }

    void unary_operator::rhs(compiler::element* element) {
        _rhs = element;
    }

    bool unary_operator::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();

        auto block = assembler.current_block();
        auto target_reg = assembler.current_target_register();

        variable_handle_t rhs_var;
        if (!session.variable(_rhs, rhs_var))
            return false;
        rhs_var->read();

        switch (operator_type()) {
            case operator_type_t::negate: {
                block->comment("negate", 4);
                block->neg(*target_reg, rhs_var->value_reg());
                break;
            }
            case operator_type_t::binary_not: {
                block->comment("binary not", 4);
                block->not_op(*target_reg, rhs_var->value_reg());
                break;
            }
            case operator_type_t::logical_not: {
                block->comment("logical not", 4);
                block->cmp(target_reg->size, rhs_var->value_reg(), 1);
                block->setnz(*target_reg);
                break;
            }
            case operator_type_t::pointer_dereference: {
                if (rhs_var->type_result().inferred_type->is_composite_type()) {
                    block->move_reg_to_reg(*target_reg, rhs_var->value_reg());
                    break;
                }

                block->comment("load primitive value from pointer", 4);
                block->clr(vm::op_sizes::qword, *target_reg);
                block->load_to_reg(*target_reg, rhs_var->value_reg());
                break;
            }
            default:
                break;
        }

        return true;
    }

    bool unary_operator::on_as_float(double& value) const {
        double rhs_value;
        if (!_rhs->as_float(rhs_value))
            return false;

        value = 0;

        switch (operator_type()) {
            case operator_type_t::negate: {
                value = -rhs_value;
                break;
            }
            default:
                return false;
        }

        return true;
    }

    bool unary_operator::on_as_integer(uint64_t& value) const {
        uint64_t rhs_value;
        if (!_rhs->as_integer(rhs_value))
            return false;

        value = 0;

        switch (operator_type()) {
            case operator_type_t::negate: {
                value = static_cast<uint64_t>(-static_cast<int64_t>(rhs_value));
                break;
            }
            case operator_type_t::binary_not: {
                value = ~rhs_value;
                break;
            }
            default:
                return false;
        }

        return true;
    }

    void unary_operator::on_owned_elements(element_list_t& list) {
        if (_rhs != nullptr)
            list.emplace_back(_rhs);
    }

    bool unary_operator::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto& scope_manager = session.scope_manager();
        switch (operator_type()) {
            case operator_type_t::negate:
            case operator_type_t::binary_not: {
                return _rhs->infer_type(session, result);
            }
            case operator_type_t::logical_not: {
                result.inferred_type = scope_manager.find_type({.name = "bool"});
                return true;
            }
            case operator_type_t::pointer_dereference: {
                if (!_rhs->infer_type(session, result))
                    return false;
                if (!result.inferred_type->is_pointer_type())
                    return false;
                auto type = dynamic_cast<compiler::pointer_type*>(result.inferred_type);
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