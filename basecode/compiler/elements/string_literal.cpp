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
#include <common/string_support.h>
#include "program.h"
#include "pointer_type.h"
#include "string_literal.h"

namespace basecode::compiler {

    bool string_literal::escape(
            const std::string& value,
            std::string& result) {
        auto read_hex_digits = [&](
                size_t index,
                size_t length,
                std::string& result) {
            while (length > 0) {
                auto ch = value[index++];
                if (ch == '_')
                    continue;
                if (isxdigit(ch)) {
                    result += ch;
                    --length;
                } else {
                    return false;
                }
            }
            return true;
        };

        auto read_dec_digits = [&](
                size_t index,
                size_t length,
                std::string& result) {
            while (length > 0) {
                auto ch = value[index++];
                if (ch == '_')
                    continue;
                if (isdigit(ch)) {
                    result += ch;
                    --length;
                } else {
                    return false;
                }
            }
            return true;
        };

        syntax::token_t token {};
        std::stringstream stream {};
        for (size_t i = 0; i < value.size(); i++) {
            auto ch = value[i];
            if (ch == '\\') {
                ch = value[++i];
                switch (ch) {
                    case 'a': {
                        stream << (char) 0x07;
                        break;
                    }
                    case 'b': {
                        stream << (char) 0x08;
                        break;
                    }
                    case 'e': {
                        stream << (char) 0x1b;
                        break;
                    }
                    case 'n': {
                        stream << (char) 0x0a;
                        break;
                    }
                    case 'r': {
                        stream << (char) 0x0d;
                        break;
                    }
                    case 't': {
                        stream << (char) 0x09;
                        break;
                    }
                    case 'v': {
                        stream << (char) 0x0b;
                        break;
                    }
                    case '\\': {
                        stream << "\\";
                        break;
                    }
                    case '"': {
                        stream << '"';
                        break;
                    }
                    case 'x': {
                        std::string hex_value;
                        if (!read_hex_digits(i + 1, 2, hex_value))
                            return false;
                        token.radix = 16;
                        token.value = hex_value;
                        token.number_type = syntax::number_types_t::integer;
                        int64_t cp;
                        if (token.parse(cp) != syntax::conversion_result_t::success)
                            return false;
                        stream << static_cast<char>(cp);
                        i += 2;
                        break;
                    }
                    case 'u': {
                        std::string hex_value;
                        if (!read_hex_digits(i + 1, 4, hex_value))
                            return false;
                        token.radix = 16;
                        token.value = hex_value;
                        token.number_type = syntax::number_types_t::integer;
                        int64_t cp;
                        if (token.parse(cp) != syntax::conversion_result_t::success)
                            return false;
                        auto encode_result = common::utf8_encode(static_cast<common::rune_t>(cp));
                        for (size_t j = 0; j < encode_result.width; j++)
                            stream << static_cast<char>(encode_result.data[j]);
                        i += 4;
                        break;
                    }
                    case 'U': {
                        std::string hex_value;
                        if (!read_hex_digits(i + 1, 8, hex_value))
                            return false;
                        token.radix = 16;
                        token.value = hex_value;
                        token.number_type = syntax::number_types_t::integer;
                        int64_t cp;
                        if (token.parse(cp) != syntax::conversion_result_t::success)
                            return false;
                        auto encode_result = common::utf8_encode(static_cast<common::rune_t>(cp));
                        for (size_t j = 0; j < encode_result.width; j++)
                            stream << static_cast<char>(encode_result.data[j]);
                        i += 8;
                        break;
                    }
                    default: {
                        std::string octal_value;
                        if (!read_dec_digits(i, 3, octal_value))
                            return false;
                        token.radix = 8;
                        token.value = octal_value;
                        token.number_type = syntax::number_types_t::integer;
                        int64_t cp;
                        if (token.parse(cp) != syntax::conversion_result_t::success)
                            return false;
                        stream << static_cast<char>(cp);
                        i += octal_value.size();
                    }
                }
            } else {
                stream << ch;
            }
        }
        result = stream.str();
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    string_literal::string_literal(
            compiler::module* module,
            block* parent_scope,
            const std::string& value) : element(module, parent_scope, element_type_t::string_literal),
                                        _value(value) {
    }

//    bool string_literal::on_emit(
//            compiler::session& session,
//            compiler::emit_context_t& context,
//            compiler::emit_result_t& result) {
//        auto& assembler = session.assembler();
//        result.operands.emplace_back(vm::instruction_operand_t(
//            assembler.make_label_ref(session.byte_code_emitter().interned_string_data_label(id()))));
//        return true;
//    }

    bool string_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        auto base_type = scope_manager.find_type(qualified_symbol_t("u8"));
        auto type = scope_manager.find_pointer_type(base_type);
        if (type == nullptr) {
            type = builder.make_pointer_type(
                parent_scope(),
                qualified_symbol_t(),
                base_type);
        }
        result.inferred_type = type;

        return true;
    }

    std::string string_literal::value() const {
        return _value;
    }

    bool string_literal::on_is_constant() const {
        return true;
    }

    bool string_literal::on_as_string(std::string& value) const {
        value = _value;
        return true;
    }

}
