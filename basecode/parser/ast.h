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

#pragma once

#include <map>
#include <stack>
#include <memory>
#include <vector>
#include <string>
#include <cerrno>
#include <functional>
#include <string_view>
#include <common/id_pool.h>
#include <common/memory_pool.h>
#include "token.h"

namespace basecode::syntax {

    using namespace std::literals;

    struct ast_node_t;

    using ast_node_list_t = std::vector<ast_node_t*>;

    enum class ast_node_type_t : uint8_t {
        pair,
        label,
        symbol,
        module,
        raw_block,
        proc_call,
        statement,
        attribute,
        directive,
        type_list,
        assignment,
        expression,
        proc_types,
        basic_block,
        symbol_part,
        nil_literal,
        new_literal,
        line_comment,
        block_comment,
        argument_list,
        if_expression,
        array_literal,
        tuple_literal,
        parameter_list,
        number_literal,
        string_literal,
        unary_operator,
        statement_body,
        type_parameter,
        assignment_set,
        spread_operator,
        proc_expression,
        binary_operator,
        boolean_literal,
        else_expression,
        while_statement,
        break_statement,
        with_expression,
        enum_expression,
        from_expression,
        cast_expression,
        case_expression,
        yield_expression,
        type_declaration,
        symbol_reference,
        return_statement,
        for_in_statement,
        union_expression,
        defer_expression,
        lambda_expression,
        module_expression,
        character_literal,
        elseif_expression,
        switch_expression,
        struct_expression,
        import_expression,
        proc_call_binding,
        family_expression,
        continue_statement,
        subscript_operator,
        with_member_access,
        type_tagged_symbol,
        value_sink_literal,
        pointer_declaration,
        type_parameter_list,
        constant_assignment,
        transmute_expression,
        namespace_expression,
        return_argument_list,
        array_subscript_list,
        uninitialized_literal,
        fallthrough_statement,
        subscript_declaration,
        assignment_source_list,
        assignment_target_list,
    };

    static inline std::unordered_map<ast_node_type_t, std::string_view> s_node_type_names = {
        {ast_node_type_t::pair, "pair"sv},
        {ast_node_type_t::label, "label"sv},
        {ast_node_type_t::module, "module"sv},
        {ast_node_type_t::symbol,  "symbol"sv},
        {ast_node_type_t::raw_block, "raw_block"sv},
        {ast_node_type_t::type_list, "type_list"sv},
        {ast_node_type_t::proc_call, "proc_call"sv},
        {ast_node_type_t::statement, "statement"sv},
        {ast_node_type_t::attribute, "attribute"sv},
        {ast_node_type_t::directive, "directive"sv},
        {ast_node_type_t::assignment, "assignment"sv},
        {ast_node_type_t::expression, "expression"sv},
        {ast_node_type_t::proc_types, "proc_types"sv},
        {ast_node_type_t::basic_block, "basic_block"sv},
        {ast_node_type_t::symbol_part, "symbol_part"sv},
        {ast_node_type_t::nil_literal, "nil_literal"sv},
        {ast_node_type_t::new_literal, "new_literal"sv},
        {ast_node_type_t::line_comment, "line_comment"sv},
        {ast_node_type_t::tuple_literal, "tuple_literal"sv},
        {ast_node_type_t::block_comment, "block_comment"sv},
        {ast_node_type_t::argument_list, "argument_list"sv},
        {ast_node_type_t::if_expression, "if_expression"sv},
        {ast_node_type_t::array_literal, "array_literal"sv},
        {ast_node_type_t::assignment_set, "assignment_set"sv},
        {ast_node_type_t::type_parameter, "type_parameter"sv},
        {ast_node_type_t::parameter_list, "parameter_list"sv},
        {ast_node_type_t::statement_body, "statement_body"sv},
        {ast_node_type_t::number_literal, "number_literal"sv},
        {ast_node_type_t::string_literal, "string_literal"sv},
        {ast_node_type_t::unary_operator, "unary_operator"sv},
        {ast_node_type_t::cast_expression, "cast_expression"sv},
        {ast_node_type_t::spread_operator, "spread_operator"sv},
        {ast_node_type_t::from_expression, "from_expression"sv},
        {ast_node_type_t::proc_expression, "proc_expression"sv},
        {ast_node_type_t::enum_expression, "enum_expression"sv},
        {ast_node_type_t::binary_operator, "binary_operator"sv},
        {ast_node_type_t::boolean_literal, "boolean_literal"sv},
        {ast_node_type_t::else_expression, "else_expression"sv},
        {ast_node_type_t::while_statement, "while_statement"sv},
        {ast_node_type_t::break_statement, "break_statement"sv},
        {ast_node_type_t::with_expression, "with_expression"sv},
        {ast_node_type_t::case_expression, "case_expression"sv},
        {ast_node_type_t::yield_expression, "yield_expression"sv},
        {ast_node_type_t::type_declaration, "type_declaration"sv},
        {ast_node_type_t::defer_expression, "defer_expression"sv},
        {ast_node_type_t::union_expression, "union_expression"sv},
        {ast_node_type_t::return_statement, "return_statement"sv},
        {ast_node_type_t::symbol_reference, "symbol_reference"sv},
        {ast_node_type_t::for_in_statement, "for_in_statement"sv},
        {ast_node_type_t::switch_expression, "switch_statement"sv},
        {ast_node_type_t::family_expression, "family_expression"sv},
        {ast_node_type_t::lambda_expression, "lambda_expression"sv},
        {ast_node_type_t::import_expression, "import_expression"sv},
        {ast_node_type_t::struct_expression, "struct_expression"sv},
        {ast_node_type_t::character_literal, "character_literal"sv},
        {ast_node_type_t::module_expression, "module_expression"sv},
        {ast_node_type_t::proc_call_binding, "proc_call_binding"sv},
        {ast_node_type_t::elseif_expression, "elseif_expression"sv},
        {ast_node_type_t::with_member_access, "with_member_access"sv},
        {ast_node_type_t::subscript_operator, "subscript_operator"sv},
        {ast_node_type_t::continue_statement, "continue_statement"sv},
        {ast_node_type_t::value_sink_literal, "value_sink_literal"sv},
        {ast_node_type_t::type_tagged_symbol, "type_tagged_symbol"sv},
        {ast_node_type_t::pointer_declaration, "pointer_declaration"sv},
        {ast_node_type_t::type_parameter_list, "type_parameter_list"sv},
        {ast_node_type_t::constant_assignment, "constant_assignment"sv},
        {ast_node_type_t::transmute_expression, "transmute_expression"sv},
        {ast_node_type_t::namespace_expression, "namespace_expression"sv},
        {ast_node_type_t::return_argument_list, "return_argument_list"sv},
        {ast_node_type_t::array_subscript_list, "array_subscript_list"sv},
        {ast_node_type_t::uninitialized_literal, "uninitialized_literal"sv},
        {ast_node_type_t::fallthrough_statement, "fallthrough_statement"sv},
        {ast_node_type_t::subscript_declaration, "subscript_declaration"sv},
        {ast_node_type_t::assignment_source_list, "assignment_source_list"sv},
        {ast_node_type_t::assignment_target_list, "assignment_target_list"sv},
    };

    static inline std::string_view ast_node_type_name(ast_node_type_t type) {
        auto it = s_node_type_names.find(type);
        if (it == s_node_type_names.end())
            return "unknown"sv;
        return it->second;
    }

    class ast_builder;

    struct ast_node_data_t {
        common::id_t id;
        ast_node_list_t labels {};
        ast_node_list_t comments {};
        ast_node_list_t attributes {};
        bool is_uniform_function_call = false;
    };

    struct ast_node_t {
        bool is_label() const {
            return type == ast_node_type_t::label;
        }

        bool has_data() const {
            return data_id != 0;
        }

        bool is_attribute() const {
            return type == ast_node_type_t::attribute;
        }

        std::string_view name() const {
            return ast_node_type_name(type);
        }

        ast_node_data_t* get_data(ast_builder* builder);

        bool operator != (const ast_node_t& other) const {
            if (token == nullptr || other.token == nullptr)
                return false;
            return token->value != other.token->value;
        }

        bool operator == (const ast_node_t& other) const {
            if (token == nullptr || other.token == nullptr)
                return false;
            return token->value == other.token->value;
        }

        bool has_attribute(ast_builder* builder, const std::string_view& name) const;

        common::id_t id;
        common::id_t data_id;
        ast_node_type_t type;
        ast_node_list_t children;
        ast_node_t* lhs = nullptr;
        ast_node_t* rhs = nullptr;
        const token_t* token = nullptr;
        common::source_location location {};
    };

    class ast_builder {
    public:
        ast_builder();

        void reset();

        //
        ast_node_data_t* make_node_data();

        //
        ast_node_t* find_node(common::id_t id);

        ast_node_data_t* find_data(common::id_t id);

        //
        ast_node_t* clone(const ast_node_t* other);

        ast_node_data_t* clone_data(common::id_t id);

        // with stack
        ast_node_t* pop_with();

        ast_node_t* current_with() const;

        void push_with(ast_node_t* node);

        // case stack
        ast_node_t* pop_case();

        ast_node_t* current_case() const;

        void push_case(ast_node_t* node);

        // switch stack
        ast_node_t* pop_switch();

        ast_node_t* current_switch() const;

        void push_switch(ast_node_t* node);

        // member access stack
        ast_node_t* pop_member_access();

        ast_node_t* current_member_access() const;

        void push_member_access(ast_node_t* node);

        // scope/block stack
        ast_node_t* end_scope();

        ast_node_t* pop_scope();

        ast_node_t* begin_scope();

        void push_scope(ast_node_t* node);

        ast_node_t* current_scope() const;

        //
        ast_node_t* pair_node();

        ast_node_t* symbol_node();

        ast_node_t* module_node();

        ast_node_t* type_list_node();

        ast_node_t* proc_call_node();

        ast_node_t* statement_node();

        ast_node_t* assignment_node();

        ast_node_t* expression_node();

        ast_node_t* proc_types_node();

        ast_node_t* basic_block_node();

        ast_node_t* argument_list_node();

        ast_node_t* binary_operator_node(
            ast_node_t* lhs,
            const token_t* token,
            ast_node_t* rhs);

        ast_node_t* type_parameter_node();

        ast_node_t* parameter_list_node();

        ast_node_t* statement_body_node();

        ast_node_t* assignment_set_node();

        ast_node_t* type_declaration_node();

        ast_node_t* proc_call_binding_node();

        ast_node_t* type_tagged_symbol_node();

        ast_node_t* subscript_operator_node();

        ast_node_t* with_member_access_node();

        ast_node_t* pointer_declaration_node();

        ast_node_t* constant_assignment_node();

        ast_node_t* type_parameter_list_node();

        ast_node_t* return_argument_list_node();

        ast_node_t* array_subscript_list_node();

        ast_node_t* subscript_declaration_node();

        ast_node_t* assignment_source_list_node();

        ast_node_t* assignment_target_list_node();

        ast_node_t* if_node(const token_t* token);

        ast_node_t* case_node(const token_t* token);

        ast_node_t* else_node(const token_t* token);

        ast_node_t* from_node(const token_t* token);

        ast_node_t* with_node(const token_t* token);

        ast_node_t* enum_node(const token_t* token);

        ast_node_t* cast_node(const token_t* token);

        ast_node_t* while_node(const token_t* token);

        ast_node_t* label_node(const token_t* token);

        ast_node_t* break_node(const token_t* token);

        ast_node_t* union_node(const token_t* token);

        ast_node_t* defer_node(const token_t* token);

        ast_node_t* yield_node(const token_t* token);

        ast_node_t* family_node(const token_t* token);

        ast_node_t* switch_node(const token_t* token);

        ast_node_t* struct_node(const token_t* token);

        ast_node_t* import_node(const token_t* token);

        ast_node_t* return_node(const token_t* token);

        ast_node_t* for_in_node(const token_t* token);

        ast_node_t* else_if_node(const token_t* token);

        ast_node_t* continue_node(const token_t* token);

        ast_node_t* directive_node(const token_t* token);

        ast_node_t* attribute_node(const token_t* token);

        ast_node_t* namespace_node(const token_t* token);

        ast_node_t* raw_block_node(const token_t* token);

        ast_node_t* transmute_node(const token_t* token);

        ast_node_t* fallthrough_node(const token_t* token);

        ast_node_t* symbol_part_node(const token_t* token);

        ast_node_t* new_literal_node(const token_t* token);

        ast_node_t* nil_literal_node(const token_t* token);

        ast_node_t* line_comment_node(const token_t* token);

        ast_node_t* block_comment_node(const token_t* token);

        ast_node_t* array_literal_node(const token_t* token);

        ast_node_t* tuple_literal_node(const token_t* token);

        ast_node_t* number_literal_node(const token_t* token);

        ast_node_t* string_literal_node(const token_t* token);

        ast_node_t* unary_operator_node(const token_t* token);

        ast_node_t* boolean_literal_node(const token_t* token);

        ast_node_t* spread_operator_node(const token_t* token);

        ast_node_t* proc_expression_node(const token_t* token);

        ast_node_t* lambda_expression_node(const token_t* token);

        ast_node_t* character_literal_node(const token_t* token);

        ast_node_t* module_expression_node(const token_t* token);

        ast_node_t* value_sink_literal_node(const token_t* token);

        ast_node_t* uninitialized_literal_node(const token_t* token);

    private:
        ast_node_t* make_node(ast_node_type_t type, const token_t* token = nullptr);

    private:
        std::stack<ast_node_t*> _case_stack {};
        std::stack<ast_node_t*> _with_stack {};
        std::stack<ast_node_t*> _scope_stack {};
        std::stack<ast_node_t*> _switch_stack {};
        common::memory_pool<ast_node_t> _node_storage;
        std::stack<ast_node_t*> _member_access_stack {};
        common::memory_pool<ast_node_data_t> _data_storage;
        std::unordered_map<common::id_t, ast_node_t*> _nodes {};
        std::unordered_map<common::id_t, ast_node_data_t*> _datas {};
    };

}