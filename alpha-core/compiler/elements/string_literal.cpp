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
#include "program.h"
#include "string_literal.h"

namespace basecode::compiler {

    string_literal::string_literal(
            compiler::module* module,
            block* parent_scope,
            const std::string& value) : element(module, parent_scope, element_type_t::string_literal),
                                        _value(value) {
    }

    bool string_literal::on_infer_type(
            const compiler::session& session,
            type_inference_result_t& result) {
        result.type = session.scope_manager().find_type({.name = "string"});
        return result.type != nullptr;
    }

    bool string_literal::on_is_constant() const {
        return true;
    }

    std::string string_literal::escaped_value() const {
        std::stringstream stream;
        bool escaped = false;
        for (auto& ch : _value) {
            if (ch == '\\') {
                escaped = true;
            } else {
                if (escaped) {
                    if (ch == 'n')
                        stream << "\n";
                    else if (ch == 'r')
                        stream << "\r";
                    else if (ch == 't')
                        stream << "\t";
                    else if (ch == '\\')
                        stream << "\\";
                    else if (ch == '0')
                        stream << '\0';

                    escaped = false;
                } else {
                    stream << ch;
                }
            }
        }
        return stream.str();
    }

    bool string_literal::on_emit(compiler::session& session) {
//        auto instruction_block = context.assembler->current_block();
//        auto target_reg = instruction_block->current_target_register();
//        instruction_block->move_label_to_ireg_with_offset(
//            target_reg->reg.i,
//            label_name(),
//            4);
        return true;
    }

    bool string_literal::on_as_string(std::string& value) const {
        value = _value;
        return true;
    }

}
