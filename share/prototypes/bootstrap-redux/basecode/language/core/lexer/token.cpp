// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include "token.h"

namespace basecode::language::core::lexer {

    std::string_view token_type_to_name(token_type_t type) {
        switch (type) {
            case token_type_t::none:                            return "none"sv;
            case token_type_t::colon:                           return "colon"sv;
            case token_type_t::comma:                           return "comma"sv;
            case token_type_t::minus:                           return "minus"sv;
            case token_type_t::caret:                           return "caret"sv;
            case token_type_t::less_than:                       return "less_than"sv;
            case token_type_t::backslash:                       return "backslash"sv;
            case token_type_t::semicolon:                       return "semicolon"sv;
            case token_type_t::directive:                       return "directive"sv;
            case token_type_t::annotation:                      return "annotation"sv;
            case token_type_t::left_paren:                      return "left_paren"sv;
            case token_type_t::value_sink:                      return "value_sink"sv;
            case token_type_t::ns_keyword:                      return "ns_keyword"sv;
            case token_type_t::if_keyword:                      return "if_keyword"sv;
            case token_type_t::identifier:                      return "identifier"sv;
            case token_type_t::use_keyword:                     return "use_keyword"sv;
            case token_type_t::nil_keyword:                     return "nil_keyword"sv;
            case token_type_t::right_paren:                     return "right_paren"sv;
            case token_type_t::for_keyword:                     return "for_keyword"sv;
            case token_type_t::in_operator:                     return "in_operator"sv;
            case token_type_t::goto_keyword:                    return "goto_keyword"sv;
            case token_type_t::with_keyword:                    return "with_keyword"sv;
            case token_type_t::else_keyword:                    return "else_keyword"sv;
            case token_type_t::enum_keyword:                    return "enum_keyword"sv;
            case token_type_t::left_bracket:                    return "left_bracket"sv;
            case token_type_t::single_quote:                    return "single_quote"sv;
            case token_type_t::line_comment:                    return "line_comment"sv;
            case token_type_t::cast_keyword:                    return "cast_keyword"sv;
            case token_type_t::case_keyword:                    return "case_keyword"sv;
            case token_type_t::true_keyword:                    return "true_keyword"sv;
            case token_type_t::proc_keyword:                    return "proc_keyword"sv;
            case token_type_t::end_of_input:                    return "end_of_input"sv;
            case token_type_t::greater_than:                    return "greater_than"sv;
            case token_type_t::add_operator:                    return "add_operator"sv;
            case token_type_t::xor_operator:                    return "xor_operator"sv;
            case token_type_t::shl_operator:                    return "shl_operator"sv;
            case token_type_t::shr_operator:                    return "shr_operator"sv;
            case token_type_t::rol_operator:                    return "rol_operator"sv;
            case token_type_t::ror_operator:                    return "ror_operator"sv;
            case token_type_t::bind_operator:                   return "bind_operator"sv;
            case token_type_t::yield_keyword:                   return "yield_keyword"sv;
            case token_type_t::union_keyword:                   return "union_keyword"sv;
            case token_type_t::break_keyword:                   return "break_keyword"sv;
            case token_type_t::defer_keyword:                   return "defer_keyword"sv;
            case token_type_t::right_bracket:                   return "right_bracket"sv;
            case token_type_t::block_comment:                   return "block_comment"sv;
            case token_type_t::block_literal:                   return "block_literal"sv;
            case token_type_t::false_keyword:                   return "false_keyword"sv;
            case token_type_t::return_keyword:                  return "return_keyword"sv;
            case token_type_t::switch_keyword:                  return "switch_keyword"sv;
            case token_type_t::string_literal:                  return "string_literal"sv;
            case token_type_t::number_literal:                  return "number_literal"sv;
            case token_type_t::family_keyword:                  return "family_keyword"sv;
            case token_type_t::struct_keyword:                  return "struct_keyword"sv;
            case token_type_t::module_keyword:                  return "module_keyword"sv;
            case token_type_t::import_keyword:                  return "import_keyword"sv;
            case token_type_t::equal_operator:                  return "equal_operator"sv;
            case token_type_t::else_if_keyword:                 return "else_if_keyword"sv;
            case token_type_t::bitcast_keyword:                 return "bitcast_keyword"sv;
            case token_type_t::divide_operator:                 return "divide_operator"sv;
            case token_type_t::modulo_operator:                 return "modulo_operator"sv;
            case token_type_t::lambda_operator:                 return "lambda_operator"sv;
            case token_type_t::continue_keyword:                return "continue_keyword"sv;
            case token_type_t::left_curly_brace:                return "left_curly_brace"sv;
            case token_type_t::right_curly_brace:               return "right_curly_brace"sv;
            case token_type_t::exponent_operator:               return "exponent_operator"sv;
            case token_type_t::multiply_operator:               return "multiply_operator"sv;
            case token_type_t::ellipsis_operator:               return "ellipsis_operator"sv;
            case token_type_t::not_equal_operator:              return "not_equal_operator"sv;
            case token_type_t::binary_or_operator:              return "binary_or_operator"sv;
            case token_type_t::fallthrough_keyword:             return "fallthrough_keyword"sv;
            case token_type_t::binary_not_operator:             return "binary_not_operator"sv;
            case token_type_t::binary_and_operator:             return "binary_and_operator"sv;
            case token_type_t::logical_or_operator:             return "logical_or_operator"sv;
            case token_type_t::assignment_operator:             return "assignment_operator"sv;
            case token_type_t::logical_not_operator:            return "logical_not_operator"sv;
            case token_type_t::logical_and_operator:            return "logical_and_operator"sv;
            case token_type_t::associative_operator:            return "associative_operator"sv;
            case token_type_t::member_select_operator:          return "member_select_operator"sv;
            case token_type_t::add_assignment_operator:         return "add_assignment_operator"sv;
            case token_type_t::less_than_equal_operator:        return "less_than_equal_operator"sv;
            case token_type_t::inclusive_range_operator:        return "inclusive_range_operator"sv;
            case token_type_t::exclusive_range_operator:        return "inclusive_range_operator"sv;
            case token_type_t::modulo_assignment_operator:      return "modulo_assignment_operator"sv;
            case token_type_t::divide_assignment_operator:      return "divide_assignment_operator"sv;
            case token_type_t::greater_than_equal_operator:     return "greater_than_equal_operator"sv;
            case token_type_t::multiply_assignment_operator:    return "multiply_assignment_operator"sv;
            case token_type_t::subtract_assignment_operator:    return "subtract_assignment_operator"sv;
            case token_type_t::binary_or_assignment_operator:   return "binary_or_assignment_operator"sv;
            case token_type_t::binary_and_assignment_operator:  return "binary_and_assignment_operator"sv;
            default:
                return "unknown"sv;
        }
    }

}
