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

#pragma once

#include <basecode/types.h>
#include <basecode/language/common.h>

namespace basecode::language::core::lexer {

    using namespace std::literals;

    enum class token_type_t {
        none,
        comma,
        minus,
        caret,
        colon,
        less_than,
        backslash,
        semicolon,
        directive,
        annotation,
        left_paren,
        value_sink,
        ns_keyword,
        if_keyword,
        identifier,
        use_keyword,
        nil_keyword,
        right_paren,
        for_keyword,
        in_operator,
        goto_keyword,
        with_keyword,
        else_keyword,
        enum_keyword,
        left_bracket,
        single_quote,
        line_comment,
        cast_keyword,
        case_keyword,
        true_keyword,
        proc_keyword,
        end_of_input,
        greater_than,
        add_operator,
        xor_operator,
        shl_operator,
        shr_operator,
        rol_operator,
        ror_operator,
        uninitialized,
        yield_keyword,
        union_keyword,
        break_keyword,
        defer_keyword,
        right_bracket,
        block_comment,
        block_literal,
        false_keyword,
        bind_operator,
        equal_operator,
        return_keyword,
        switch_keyword,
        string_literal,
        number_literal,
        family_keyword,
        struct_keyword,
        module_keyword,
        import_keyword,
        else_if_keyword,
        bitcast_keyword,
        divide_operator,
        modulo_operator,
        lambda_operator,
        continue_keyword,
        left_curly_brace,
        right_curly_brace,
        exponent_operator,
        multiply_operator,
        ellipsis_operator,
        not_equal_operator,
        binary_or_operator,
        fallthrough_keyword,
        binary_not_operator,
        binary_and_operator,
        logical_or_operator,
        assignment_operator,
        logical_not_operator,
        logical_and_operator,
        associative_operator,
        member_select_operator,
        add_assignment_operator,
        less_than_equal_operator,
        inclusive_range_operator,
        exclusive_range_operator,
        modulo_assignment_operator,
        divide_assignment_operator,
        greater_than_equal_operator,
        multiply_assignment_operator,
        subtract_assignment_operator,
        binary_or_assignment_operator,
        binary_and_assignment_operator,
    };

    std::string_view token_type_to_name(token_type_t type);

    using token_t = basic_token_t<token_type_t>;

}
