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
#include "token.h"

namespace basecode::syntax {

    struct ast_node_t;

    using ast_node_shared_ptr = std::shared_ptr<ast_node_t>;
    using ast_node_list = std::vector<ast_node_shared_ptr>;

    enum class ast_node_types_t {
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
        line_comment,
        block_comment,
        argument_list,
        if_expression,
        parameter_list,
        number_literal,
        string_literal,
        unary_operator,
        statement_body,
        new_expression,
        map_expression,
        type_parameter,
        spread_operator,
        proc_expression,
        binary_operator,
        boolean_literal,
        else_expression,
        while_statement,
        break_statement,
        with_expression,
        enum_expression,
        cast_expression,
        from_expression,
        type_declaration,
        symbol_reference,
        return_statement,
        for_in_statement,
        union_expression,
        defer_expression,
        tuple_expression,
        array_expression,
        module_expression,
        character_literal,
        elseif_expression,
        switch_expression,
        struct_expression,
        import_expression,
        proc_call_binding,
        continue_statement,
        subscript_operator,
        with_member_access,
        pointer_declaration,
        type_parameter_list,
        constant_assignment,
        namespace_expression,
        return_argument_list,
        array_subscript_list,
        transmute_expression,
        subscript_declaration,
        assignment_source_list,
        assignment_target_list,
    };

    static inline std::unordered_map<ast_node_types_t, std::string> s_node_type_names = {
        {ast_node_types_t::pair, "pair"},
        {ast_node_types_t::label, "label"},
        {ast_node_types_t::module, "module"},
        {ast_node_types_t::symbol,  "symbol"},
        {ast_node_types_t::raw_block, "raw_block"},
        {ast_node_types_t::type_list, "type_list"},
        {ast_node_types_t::proc_call, "proc_call"},
        {ast_node_types_t::statement, "statement"},
        {ast_node_types_t::attribute, "attribute"},
        {ast_node_types_t::directive, "directive"},
        {ast_node_types_t::assignment, "assignment"},
        {ast_node_types_t::expression, "expression"},
        {ast_node_types_t::proc_types, "proc_types"},
        {ast_node_types_t::basic_block, "basic_block"},
        {ast_node_types_t::symbol_part, "symbol_part"},
        {ast_node_types_t::nil_literal, "nil_literal"},
        {ast_node_types_t::line_comment, "line_comment"},
        {ast_node_types_t::block_comment, "block_comment"},
        {ast_node_types_t::argument_list, "argument_list"},
        {ast_node_types_t::if_expression, "if_expression"},
        {ast_node_types_t::type_parameter, "type_parameter"},
        {ast_node_types_t::parameter_list, "parameter_list"},
        {ast_node_types_t::statement_body, "statement_body"},
        {ast_node_types_t::number_literal, "number_literal"},
        {ast_node_types_t::string_literal, "string_literal"},
        {ast_node_types_t::unary_operator, "unary_operator"},
        {ast_node_types_t::new_expression, "new_expression"},
        {ast_node_types_t::map_expression, "map_expression"},
        {ast_node_types_t::spread_operator, "spread_operator"},
        {ast_node_types_t::cast_expression, "cast_expression"},
        {ast_node_types_t::from_expression, "from_expression"},
        {ast_node_types_t::proc_expression, "proc_expression"},
        {ast_node_types_t::enum_expression, "enum_expression"},
        {ast_node_types_t::binary_operator, "binary_operator"},
        {ast_node_types_t::boolean_literal, "boolean_literal"},
        {ast_node_types_t::else_expression, "else_expression"},
        {ast_node_types_t::while_statement, "while_statement"},
        {ast_node_types_t::break_statement, "break_statement"},
        {ast_node_types_t::with_expression, "with_expression"},
        {ast_node_types_t::type_declaration, "type_declaration"},
        {ast_node_types_t::tuple_expression, "tuple_expression"},
        {ast_node_types_t::defer_expression, "defer_expression"},
        {ast_node_types_t::union_expression, "union_expression"},
        {ast_node_types_t::return_statement, "return_statement"},
        {ast_node_types_t::symbol_reference, "symbol_reference"},
        {ast_node_types_t::for_in_statement, "for_in_statement"},
        {ast_node_types_t::switch_expression, "switch_statement"},
        {ast_node_types_t::array_expression,  "array_expression"},
        {ast_node_types_t::import_expression, "import_expression"},
        {ast_node_types_t::struct_expression, "struct_expression"},
        {ast_node_types_t::character_literal, "character_literal"},
        {ast_node_types_t::module_expression, "module_expression"},
        {ast_node_types_t::proc_call_binding, "proc_call_binding"},
        {ast_node_types_t::elseif_expression, "elseif_expression"},
        {ast_node_types_t::with_member_access, "with_member_access"},
        {ast_node_types_t::subscript_operator, "subscript_operator"},
        {ast_node_types_t::continue_statement, "continue_statement"},
        {ast_node_types_t::pointer_declaration, "pointer_declaration"},
        {ast_node_types_t::type_parameter_list, "type_parameter_list"},
        {ast_node_types_t::constant_assignment, "constant_assignment"},
        {ast_node_types_t::transmute_expression, "transmute_expression"},
        {ast_node_types_t::namespace_expression, "namespace_expression"},
        {ast_node_types_t::return_argument_list, "return_argument_list"},
        {ast_node_types_t::array_subscript_list, "array_subscript_list"},
        {ast_node_types_t::subscript_declaration, "subscript_declaration"},
        {ast_node_types_t::assignment_source_list, "assignment_source_list"},
        {ast_node_types_t::assignment_target_list, "assignment_target_list"},
    };

    static inline std::string ast_node_type_name(ast_node_types_t type) {
        auto it = s_node_type_names.find(type);
        if (it == s_node_type_names.end())
            return "unknown";
        return it->second;
    }

    struct ast_node_t {
        bool is_label() const {
            return type == ast_node_types_t::label;
        }

        std::string name() const {
            return ast_node_type_name(type);
        }

        bool is_attribute() const {
            return type == ast_node_types_t::attribute;
        }

        bool operator != (const ast_node_t& other) const {
            return this->token.value != other.token.value;
        }

        bool operator == (const ast_node_t& other) const {
            return this->token.value == other.token.value;
        }

        uint32_t id;
        token_t token;
        ast_node_types_t type;
        ast_node_list children;
        ast_node_list labels {};
        ast_node_list comments {};
        ast_node_list attributes {};
        common::source_location location {};
        ast_node_shared_ptr lhs = nullptr;
        ast_node_shared_ptr rhs = nullptr;
        ast_node_shared_ptr parent = nullptr;
    };

    class ast_builder {
    public:
        ast_builder() = default;

        void reset();

        // with stack
        ast_node_shared_ptr pop_with();

        ast_node_shared_ptr current_with() const;

        void push_with(const ast_node_shared_ptr& node);

        // scope/block stack
        ast_node_shared_ptr end_scope();

        ast_node_shared_ptr pop_scope();

        ast_node_shared_ptr begin_scope();

        ast_node_t* current_scope() const;

        //
        ast_node_shared_ptr pair_node();

        ast_node_shared_ptr symbol_node();

        ast_node_shared_ptr module_node();

        ast_node_shared_ptr type_list_node();

        ast_node_shared_ptr proc_call_node();

        ast_node_shared_ptr statement_node();

        ast_node_shared_ptr assignment_node();

        ast_node_shared_ptr expression_node();

        ast_node_shared_ptr proc_types_node();

        ast_node_shared_ptr basic_block_node();

        ast_node_shared_ptr argument_list_node();

        ast_node_shared_ptr binary_operator_node(
            const ast_node_shared_ptr& lhs,
            const token_t& token,
            const ast_node_shared_ptr& rhs);

        ast_node_shared_ptr type_parameter_node();

        ast_node_shared_ptr new_expression_node();

        ast_node_shared_ptr map_expression_node();

        ast_node_shared_ptr parameter_list_node();

        ast_node_shared_ptr statement_body_node();

        ast_node_shared_ptr type_declaration_node();

        ast_node_shared_ptr tuple_expression_node();

        ast_node_shared_ptr array_expression_node();

        ast_node_shared_ptr proc_call_binding_node();

        ast_node_shared_ptr subscript_operator_node();

        ast_node_shared_ptr with_member_access_node();

        ast_node_shared_ptr pointer_declaration_node();

        ast_node_shared_ptr constant_assignment_node();

        ast_node_shared_ptr type_parameter_list_node();

        ast_node_shared_ptr return_argument_list_node();

        ast_node_shared_ptr array_subscript_list_node();

        ast_node_shared_ptr subscript_declaration_node();

        void push_scope(const ast_node_shared_ptr& node);

        ast_node_shared_ptr assignment_source_list_node();

        ast_node_shared_ptr assignment_target_list_node();

        ast_node_shared_ptr if_node(const token_t& token);

        ast_node_shared_ptr else_node(const token_t& token);

        ast_node_shared_ptr from_node(const token_t& token);

        ast_node_shared_ptr cast_node(const token_t& token);

        ast_node_shared_ptr with_node(const token_t& token);

        ast_node_shared_ptr enum_node(const token_t& token);

        ast_node_shared_ptr while_node(const token_t& token);

        ast_node_shared_ptr label_node(const token_t& token);

        ast_node_shared_ptr break_node(const token_t& token);

        ast_node_shared_ptr union_node(const token_t& token);

        ast_node_shared_ptr defer_node(const token_t& token);

        ast_node_shared_ptr struct_node(const token_t& token);

        ast_node_shared_ptr import_node(const token_t& token);

        ast_node_shared_ptr return_node(const token_t& token);

        ast_node_shared_ptr for_in_node(const token_t& token);

        ast_node_shared_ptr else_if_node(const token_t& token);

        ast_node_shared_ptr continue_node(const token_t& token);

        ast_node_shared_ptr proc_expression_node(token_t& token);

        ast_node_shared_ptr directive_node(const token_t& token);

        ast_node_shared_ptr attribute_node(const token_t& token);

        ast_node_shared_ptr namespace_node(const token_t& token);

        ast_node_shared_ptr transmute_node(const token_t& token);

        ast_node_shared_ptr raw_block_node(const token_t& token);

        ast_node_shared_ptr symbol_part_node(const token_t& token);

        ast_node_shared_ptr nil_literal_node(const token_t& token);

        ast_node_shared_ptr line_comment_node(const token_t& token);

        ast_node_shared_ptr block_comment_node(const token_t& token);

        ast_node_shared_ptr number_literal_node(const token_t& token);

        ast_node_shared_ptr string_literal_node(const token_t& token);

        ast_node_shared_ptr unary_operator_node(const token_t& token);

        ast_node_shared_ptr boolean_literal_node(const token_t& token);

        ast_node_shared_ptr spread_operator_node(const token_t& token);

        ast_node_shared_ptr character_literal_node(const token_t& token);

        ast_node_shared_ptr module_expression_node(const token_t& token);

    private:
        void configure_node(
            const ast_node_shared_ptr& node,
            const token_t& token,
            ast_node_types_t type);

    private:
        uint32_t _id = 0;
        std::stack<ast_node_shared_ptr> _with_stack {};
        std::stack<ast_node_shared_ptr> _scope_stack {};
    };

}