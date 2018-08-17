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
#include "cast.h"
#include "numeric_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    enum class cast_mode_t : uint8_t {
        noop,
        integer_truncate,
        integer_sign_extend,
        integer_zero_extend,
        float_extend,
        float_truncate,
        float_to_integer,
        integer_to_float,
    };

    cast::cast(
            compiler::module* module,
            block* parent_scope,
            compiler::type* type,
            element* expr) : element(module, parent_scope, element_type_t::cast),
                             _expression(expr),
                             _type(type) {
    }

    element* cast::expression() {
        return _expression;
    }

    compiler::type* cast::type() {
        return _type;
    }

    //
    // numeric casts
    // ------------------------------------------------------------------------
    // casting between two integers of the same size (s32 -> u32) is a no-op
    // casting from a larger integer to a smaller integer (u32 -> u8) will truncate via move
    // casting from smaller integer to larger integer (u8 -> u32) will:
    //  - zero-extend if the source is unsigned
    //  - sign-extend if the source is signed
    // casting from float to an integer will round the float towards zero
    // casting from an integer to a float will produce the floating point representation of the
    //    integer, rounded if necessary
    // casting from f32 to f64 is lossless
    // casting from f64 to f32 will produce the closest possible value, rounded if necessary
    // casting bool to and integer type will yield 1 or 0
    // casting any integer type whose LSB is set will yield true; otherwise, false
    //
    // pointer casts
    // ------------------------------------------------------------------------
    // integer to pointer type:
    //
    bool cast::on_emit(compiler::session& session) {
        if (_expression == nullptr)
            return true;

        cast_mode_t mode;
        auto source_type = _expression->infer_type(session);
        auto source_number_class = source_type->number_class();
        auto source_size = source_type->size_in_bytes();
        auto target_number_class = _type->number_class();
        auto target_size = _type->size_in_bytes();

        if (source_number_class == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot cast from type: {}", source_type->symbol()->name()),
                _expression->location());
            return false;
        } else if (target_number_class == type_number_class_t::none) {
            session.error(
                this,
                "C073",
                fmt::format("cannot cast to type: {}", _type->symbol()->name()),
                _type_location);
            return false;
        }

        auto& assembler = session.assembler();
        auto target_reg = assembler.current_target_register();
        auto instruction_block = assembler.current_block();

        if (source_number_class == type_number_class_t::integer
        &&  target_number_class == type_number_class_t::integer) {
            if (source_size == target_size) {
                mode = cast_mode_t::integer_truncate;
            } else if (source_size > target_size) {
                mode = cast_mode_t::integer_truncate;
            } else {
                auto source_numeric_type = dynamic_cast<compiler::numeric_type*>(source_type);
                if (source_numeric_type->is_signed()) {
                    mode = cast_mode_t::integer_sign_extend;
                } else {
                    mode = cast_mode_t::integer_zero_extend;
                }
            }
        } else if (source_number_class == type_number_class_t::floating_point
               &&  target_number_class == type_number_class_t::floating_point) {
            if (source_size == target_size) {
                mode = cast_mode_t::float_truncate;
            } else if (source_size > target_size) {
                mode = cast_mode_t::float_truncate;
            } else {
                mode = cast_mode_t::float_extend;
            }
        } else {
            if (source_number_class == type_number_class_t::integer) {
                mode = cast_mode_t::integer_to_float;
            } else {
                mode = cast_mode_t::float_to_integer;
            }
        }

        auto temp_reg = register_for(session, _expression);
        if (!temp_reg.valid)
            return false;

        assembler.push_target_register(temp_reg.reg);
        _expression->emit(session);
        assembler.pop_target_register();

        switch (mode) {
            case cast_mode_t::integer_truncate: {
                instruction_block->move_reg_to_reg(*target_reg, temp_reg.reg);
                break;
            }
            case cast_mode_t::integer_sign_extend: {
                instruction_block->moves_reg_to_reg(*target_reg, temp_reg.reg);
                break;
            }
            case cast_mode_t::integer_zero_extend: {
                instruction_block->movez_reg_to_reg(*target_reg, temp_reg.reg);
                break;
            }
            case cast_mode_t::float_extend: {
                break;
            }
            case cast_mode_t::float_truncate: {
                break;
            }
            case cast_mode_t::float_to_integer: {
                break;
            }
            case cast_mode_t::integer_to_float: {
                break;
            }
            default: {
                break;
            }
        }

        instruction_block->current_entry()->comment(
            fmt::format(
                "cast<{}> from type {}",
                _type->symbol()->name(),
                source_type->symbol()->name()),
            session.emit_context().indent);

        return true;
    }

    void cast::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    void cast::type_location(const common::source_location& loc) {
        _type_location = loc;
    }

    compiler::type* cast::on_infer_type(const compiler::session& session) {
        return _type;
    }

};