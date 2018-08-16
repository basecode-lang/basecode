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
#include "symbol_element.h"

namespace basecode::compiler {

    cast::cast(
            compiler::module* module,
            block* parent_scope,
            compiler::type* type,
            element* expr) : element(module, parent_scope, element_type_t::cast),
                             _expression(expr),
                             _type(type) {
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
    bool cast::on_emit(compiler::session& session) {
        if (_expression == nullptr)
            return true;

        auto& assembler = session.assembler();
        auto target_reg = assembler.current_target_register();
        auto instruction_block = assembler.current_block();

        auto temp_reg = register_for(session, _expression);
        if (!temp_reg.valid)
            return false;

        assembler.push_target_register(temp_reg.reg);
        _expression->emit(session);
        assembler.pop_target_register();

        // XXX: this is a placeholder, need to check conditions above
        instruction_block->move_reg_to_reg(*target_reg, temp_reg.reg);

        instruction_block->current_entry()->comment(
            fmt::format("cast<{}>", _type->symbol()->name()),
            session.emit_context().indent);
        return true;
    }

    element* cast::expression() {
        return _expression;
    }

    compiler::type* cast::type() {
        return _type;
    }

    void cast::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    compiler::type* cast::on_infer_type(const compiler::session& session) {
        return _type;
    }

};