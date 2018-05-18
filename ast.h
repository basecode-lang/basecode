#pragma once

#include <map>
#include <stack>
#include <memory>
#include <vector>
#include <string>
#include <cerrno>
#include <variant>
#include <functional>
#include "token.h"

namespace basecode {

    struct ast_node_t;

    using ast_node_shared_ptr = std::shared_ptr<ast_node_t>;
    using ast_node_list = std::vector<ast_node_shared_ptr>;

    enum class ast_node_types_t {
        program,
        statement,
        attribute,
        assignment,
        expression,
        line_comment,
        block_comment,
        null_literal,
        number_literal,
        string_literal,
        character_literal,
        boolean_literal,
        if_expression,
        map_constructor,
        array_constructor,
        elseif_expression,
        else_expression,
        for_statement,
        while_statement,
        continue_statement,
        alias_statement,
        extend_statement,
        break_statement,
        switch_expression,
        with_expression,
        type_identifier,
        pointer_type_identifier,
        variable_reference,
        variable_declaration,
        namespace_statement,
        function_call,
        function_expression,
        basic_block,
        unary_operator,
        binary_operator,
    };

    // foo := 5 + 5;
    //
    //  program_node (root)
    //          |
    //          |
    //          | statement_node
    //          +---> lhs := variable (token = "foo")
    //                         |
    //                rhs := expression (token is unknown)
    //                         |
    //                         +--children
    //                              |
    //                           (0)+--> binary_operator (token = "+")
    //                                      lhs := number_literal (token = "5")
    //
    //                                      rhs := number_literal (token = "5")
    //

    struct ast_node_t {
        using flags_value_t = uint8_t;
        enum flags_t : uint8_t {
            none    = 0b00000000,
            pointer = 0b00000001,
            array   = 0b00000010,
        };

        inline bool is_array() const {
            return ((flags & flags_t::array) != 0);
        }

        inline bool is_pointer() const {
            return ((flags & flags_t::pointer) != 0);
        }

        token_t token;
        ast_node_types_t type;
        ast_node_list children;
        ast_node_shared_ptr lhs = nullptr;
        ast_node_shared_ptr rhs = nullptr;
        flags_value_t flags = flags_t::none;
        ast_node_shared_ptr parent = nullptr;
    };

    class ast_builder {
    public:
        ast_builder();

        virtual ~ast_builder();

        ast_node_shared_ptr end_scope();

        ast_node_shared_ptr pop_scope();

        ast_node_shared_ptr begin_scope();

        ast_node_t* current_scope() const;

        ast_node_shared_ptr program_node();

        ast_node_shared_ptr statement_node();

        ast_node_shared_ptr assignment_node();

        ast_node_shared_ptr basic_block_node();

        void push_scope(const ast_node_shared_ptr& node);

        ast_node_shared_ptr attribute_node(const token_t& token);

        ast_node_shared_ptr null_literal_node(const token_t& token);

        ast_node_shared_ptr line_comment_node(const token_t& token);

        ast_node_shared_ptr block_comment_node(const token_t& token);

        ast_node_shared_ptr number_literal_node(const token_t& token);

        ast_node_shared_ptr string_literal_node(const token_t& token);

        ast_node_shared_ptr type_identifier_node(const token_t& token);

        ast_node_shared_ptr boolean_literal_node(const token_t& token);

        ast_node_shared_ptr character_literal_node(const token_t& token);

        ast_node_shared_ptr variable_reference_node(const token_t& token);

        ast_node_shared_ptr variable_declaration_node(const token_t& token);

    private:
        std::stack<ast_node_shared_ptr> _scope_stack {};
    };
}