// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <set>
#include <fmt/format.h>
#include "ast_formatter.h"

namespace basecode::syntax {

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
                case basecode::syntax::number_types_t::none:
                    fmt::print(_file, "{1:{0}}      type: none\n", level, "");
                    break;
                case basecode::syntax::number_types_t::integer:
                    fmt::print(_file, "{1:{0}}      type: integer\n", level, "");
                    break;
                case basecode::syntax::number_types_t::floating_point:
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
            case basecode::syntax::ast_node_types_t::line_comment:
            case basecode::syntax::ast_node_types_t::block_comment:
                style = ", fillcolor=green, style=\"filled\"";
                details = fmt::format("|{{ token: '{}' }}", node->token.value);
                break;
            case basecode::syntax::ast_node_types_t::program:
                style = ", fillcolor=cadetblue, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::binary_operator:
                style = ", fillcolor=goldenrod1, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::symbol_reference:
                style = ", fillcolor=aquamarine3, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::type_identifier:
                style = ", fillcolor=gainsboro, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::attribute:
                style = ", fillcolor=darkseagreen, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::statement:
                style = ", fillcolor=cornflowerblue, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::proc_expression:
                style = ", fillcolor=cyan, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::proc_call:
                style = ", fillcolor=darkorchid1, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::basic_block:
                style = ", fillcolor=lightblue, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::assignment:
                style = ", fillcolor=pink, style=\"filled\"";
                break;
            case basecode::syntax::ast_node_types_t::if_expression:
            case basecode::syntax::ast_node_types_t::else_expression:
            case basecode::syntax::ast_node_types_t::elseif_expression:
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
                "|{{ token: '{}' ",
                value);

            if (node->token.is_numeric()) {
                details += fmt::format("| radix: {}", node->token.radix);
            }

            if (node->is_pointer()) {
                details += "| ptr:    true ";
            }

            if (node->is_array()) {
                details += "| array:  true ";
            }

            if (node->is_spread()) {
                details += "| spread: true ";
            }

            details += "}";
        }

        fmt::print(
            _file,
            "\t{}[shape={},label=\"{}<f1> {}{}{}\"{}];\n",
            node_vertex_name,
            shape,
            node->lhs != nullptr ? "<f0> lhs|" : "",
            node->name(),
            details,
            node->rhs != nullptr ? "|<f2> rhs" : "",
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
        std::set <std::string> edges{};

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

};