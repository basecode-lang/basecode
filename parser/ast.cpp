#include <set>
#include <fmt/format.h>
#include "ast.h"

namespace basecode::syntax {

    ///////////////////////////////////////////////////////////////////////////

    ast_formatter::ast_formatter(
            const ast_node_shared_ptr& root,
            FILE* file) : _file(file),
                          _root(root) {
    }

    void ast_formatter::format_text() {
        format_text_node(_root, 0);
    }

    void ast_formatter::format_text_node(
            const ast_node_shared_ptr& node,
            uint32_t level) {
        if (node == nullptr) {
            fmt::print(_file, "nullptr");
            return;
        }
        fmt::print(
            _file,
            "[type: {} | token: {}]\n",
            node->name(),
            node->token.name());
        fmt::print(_file, "{1:{0}}     value: '{2}'\n", level, "", node->token.value);
        if (node->token.is_numeric()) {
            fmt::print("_file, {1:{0}}     radix: {2}\n", level, "", node->token.radix);
            switch (node->token.number_type) {
                case number_types_t::none:
                    fmt::print(_file, "{1:{0}}      type: none\n", level, "");
                    break;
                case number_types_t::integer:
                    fmt::print(_file, "{1:{0}}      type: integer\n", level, "");
                    break;
                case number_types_t::floating_point:
                    fmt::print(_file, "{1:{0}}      type: floating_point\n", level, "");
                    break;
            }
        }
        fmt::print(_file, "{1:{0}}is_pointer: {2}\n", level, "", node->is_pointer());
        fmt::print(_file, "{1:{0}}  is_array: {2}\n", level, "", node->is_array());
        fmt::print(_file, "{1:{0}}             --\n", level, "");
        fmt::print(_file, "{1:{0}}       lhs: ", level, "");
        format_text_node(node->lhs, level + 7);
        fmt::print(_file, "\n");
        fmt::print(_file, "{1:{0}}             --\n", level, "");
        fmt::print(_file, "{1:{0}}       rhs: ", level, "");
        format_text_node(node->rhs, level + 7);
        fmt::print(_file, "\n");
        fmt::print(_file, "{1:{0}}             --\n", level, "");

        auto index = 0;
        for (auto child : node->children) {
            fmt::print(_file, "{1:{0}}      [{2:02}] ", level, "", index++);
            format_text_node(child, level + 6);
            fmt::print(_file, "\n");
        }
    }

    void ast_formatter::format_graph_viz() {
        fmt::print(_file, "digraph {{\n");
        //fmt::print("rankdir=LR\n");
        //fmt::print(_file, "\tsplines=\"line\";\n");
        format_graph_viz_node(_root);
        fmt::print(_file, "}}\n");
    }

    void ast_formatter::format_graph_viz_node(const ast_node_shared_ptr& node) {
        if (node == nullptr)
            return;

        auto node_vertex_name = get_vertex_name(node);

        std::string shape = "record", style, details;

        switch (node->type) {
            case ast_node_types_t::line_comment:
            case ast_node_types_t::block_comment:
                style = ", fillcolor=green, style=\"filled\"";
                details = fmt::format("|{{ token: '{}' }}", node->token.value);
                break;
            case ast_node_types_t::program:
                style = ", fillcolor=cadetblue, style=\"filled\"";
                break;
            case ast_node_types_t::binary_operator:
                style = ", fillcolor=goldenrod1, style=\"filled\"";
                break;
            case ast_node_types_t::symbol_reference:
                style = ", fillcolor=aquamarine3, style=\"filled\"";
                break;
            case ast_node_types_t::type_identifier:
                style = ", fillcolor=gainsboro, style=\"filled\"";
                break;
            case ast_node_types_t::attribute:
                style = ", fillcolor=darkseagreen, style=\"filled\"";
                break;
            case ast_node_types_t::statement:
                style = ", fillcolor=cornflowerblue, style=\"filled\"";
                break;
            case ast_node_types_t::fn_expression:
                style = ", fillcolor=cyan, style=\"filled\"";
                break;
            case ast_node_types_t::fn_call:
                style = ", fillcolor=darkorchid1, style=\"filled\"";
                break;
            case ast_node_types_t::basic_block:
                style = ", fillcolor=lightblue, style=\"filled\"";
                break;
            case ast_node_types_t::assignment:
                style = ", fillcolor=pink, style=\"filled\"";
                break;
            case ast_node_types_t::if_expression:
            case ast_node_types_t::else_expression:
            case ast_node_types_t::elseif_expression:
                shape = "Mrecord";
                style = ", fillcolor=yellow, style=\"filled\"";
                break;
            default:
                break;
        }

        if (!node->token.value.empty() && details.empty()) {
            std::string value = node->token.value;
            if (value == "|")
                value = "&#124;";
            else if (value == "||")
                value = "&#124;&#124;";
            details = fmt::format(
                "|{{ token: '{}' | radix: {} | ptr: {} | array: {} | spread: {} }}",
                value,
                node->token.radix,
                node->is_pointer(),
                node->is_array(),
                node->is_spread());
        }

        fmt::print(
            _file,
            "\t{}[shape={},label=\"<f0> lhs|<f1> {}{}|<f2> rhs\"{}];\n",
            node_vertex_name,
            shape,
            node->name(),
            details,
            style);
        if (node->lhs != nullptr) {
            format_graph_viz_node(node->lhs);
            fmt::print(
                _file,
                "\t{}:f0 -> {}:f1;\n",
                node_vertex_name,
                get_vertex_name(node->lhs));
        }

        if (node->rhs != nullptr) {
            format_graph_viz_node(node->rhs);
            fmt::print(
                _file,
                "\t{}:f2 -> {}:f1;\n",
                node_vertex_name,
                get_vertex_name(node->rhs));
        }

        auto index = 0;
        std::set<std::string> edges {};

        for (auto child : node->children) {
            format_graph_viz_node(child);
            edges.insert(get_vertex_name(child));
            index++;
        }

        if (!edges.empty()) {
            index = 0;
            for (const auto& edge : edges)
                fmt::print(
                    _file,
                    "\t{}:f1 -> {}:f1 [label=\"[{:02}]\"];\n",
                    node_vertex_name,
                    edge,
                    index++);
            fmt::print(_file, "\n");
        }
    }

    std::string ast_formatter::get_vertex_name(const ast_node_shared_ptr& node) const {
        return fmt::format("{}{}", node->name(), node->id);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_builder::ast_builder() {
    }

    ast_builder::~ast_builder() {
    }

    void ast_builder::configure_node(
            const ast_node_shared_ptr& node,
            const token_t& token,
            ast_node_types_t type) {
        node->id = ++_id;
        node->type = type;
        node->token = token;
    }

    ast_node_shared_ptr ast_builder::if_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::if_expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::else_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::else_expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    ast_node_shared_ptr ast_builder::end_scope() {
        return pop_scope();
    }

    ast_node_shared_ptr ast_builder::return_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::return_statement;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top().get();
    }

    ast_node_shared_ptr ast_builder::else_if_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::else_expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::program_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::program;
        push_scope(node);
        return node;
    }

    ast_node_shared_ptr ast_builder::begin_scope() {
        if (_scope_stack.empty()) {
            return program_node();
        } else {
            return basic_block_node();
        }
    }

    ast_node_shared_ptr ast_builder::for_in_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::for_in_statement;
        return node;
    }

    ast_node_shared_ptr ast_builder::fn_call_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::fn_call;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::fn_decl_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::fn_expression;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::statement_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::statement;
        return node;
    }

    ast_node_shared_ptr ast_builder::subscript_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::subscript_expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::expression_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::assignment_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::assignment;
        return node;
    }

    ast_node_shared_ptr ast_builder::basic_block_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::basic_block;
        push_scope(node);
        return node;
    }

    ast_node_shared_ptr ast_builder::binary_operator_node(
            const ast_node_shared_ptr& lhs,
            const token_t& token,
            const ast_node_shared_ptr& rhs) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::binary_operator);
        node->lhs = lhs;
        node->rhs = rhs;
        return node;
    }

    ast_node_shared_ptr ast_builder::argument_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::argument_list;
        return node;
    }

    void ast_builder::push_scope(const ast_node_shared_ptr& node) {
        _scope_stack.push(node);
    }

    ast_node_shared_ptr ast_builder::enum_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::enum_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::break_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::break_statement);
        return node;
    }

    ast_node_shared_ptr ast_builder::qualified_symbol_reference_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->lhs = argument_list_node();
        node->type = ast_node_types_t::qualified_symbol_reference;
        return node;
    }

    ast_node_shared_ptr ast_builder::union_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::union_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::struct_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::struct_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::continue_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::continue_statement);
        return node;
    }

    ast_node_shared_ptr ast_builder::namespace_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::namespace_statement);
        node->lhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::directive_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::directive);
        return node;
    }

    ast_node_shared_ptr ast_builder::attribute_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::attribute);
        return node;
    }

    ast_node_shared_ptr ast_builder::null_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::null_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::none_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::none_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::line_comment_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::line_comment);
        return node;
    }

    ast_node_shared_ptr ast_builder::block_comment_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::block_comment);
        return node;
    }

    ast_node_shared_ptr ast_builder::empty_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::empty_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::unary_operator_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::unary_operator);
        return node;
    }

    ast_node_shared_ptr ast_builder::string_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::string_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::number_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::number_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::boolean_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::boolean_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::type_identifier_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::type_identifier);
        return node;
    }

    ast_node_shared_ptr ast_builder::character_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::character_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::symbol_reference_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::symbol_reference);
        return node;
    }

};