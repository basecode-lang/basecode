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

#include <set>
#include <fmt/format.h>
#include "ast_formatter.h"

namespace basecode::syntax {

    ast_formatter::ast_formatter(
            const ast_node_shared_ptr& root,
            FILE* file) : _file(file),
                          _root(root) {
    }

    void ast_formatter::format(const std::string& title) {
        fmt::print(_file, "digraph {{\n");
        fmt::print(_file, "graph [ fontsize=22 ];\n");
        fmt::print(_file, "labelloc=\"t\";\n");
        fmt::print(_file, "label=\"{}\";\n", title);
        format_node(_root);
        fmt::print(_file, "}}\n");
    }

    void ast_formatter::format_node(const ast_node_shared_ptr& node) {
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
            case basecode::syntax::ast_node_types_t::module:
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
            format_node(node->lhs);
            fmt::print(
                _file,
                "\t{}:f0 -> {}:f1;\n",
                node_vertex_name,
                get_vertex_name(node->lhs));
        }

        if (node->rhs != nullptr) {
            format_node(node->rhs);
            fmt::print(
                _file,
                "\t{}:f2 -> {}:f1;\n",
                node_vertex_name,
                get_vertex_name(node->rhs));
        }

        auto index = 0;
        std::set <std::string> edges{};

        for (auto child : node->children) {
            format_node(child);
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