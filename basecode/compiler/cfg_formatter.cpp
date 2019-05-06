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

#include <vm/assembler.h>
#include <compiler/byte_code_emitter.h>
#include <common/graphviz_formatter.h>
#include "cfg_formatter.h"

namespace basecode::compiler {

    cfg_formatter::cfg_formatter(
            compiler::session& session,
            FILE* output_file) : _file(output_file),
                                 _session(session) {
    }

    void cfg_formatter::add_successor(
            vm::basic_block* from,
            vm::basic_block* to,
            const std::string& label) {
        std::string attributes;
        if (!label.empty())
            attributes = fmt::format("[ label=\"{}\"; ]", label);

        _edges.insert(fmt::format(
            "{} -> {} {};",
            get_vertex_name(from),
            get_vertex_name(to),
            attributes));
    }

    void cfg_formatter::add_predecessor(
            vm::basic_block* from,
            vm::basic_block* to,
            const std::string& label) {
        std::string attributes = "[ style=dashed; ";
        if (!label.empty())
            attributes = fmt::format("label=\"{}\"; ", label);
        attributes += "]";

        _edges.insert(fmt::format(
            "{} -> {} {};",
            get_vertex_name(from),
            get_vertex_name(to),
            attributes));
    }

    void cfg_formatter::format(const std::string& title) {
        fmt::print(_file, "digraph {{\n");
        //fmt::print(_file, "rankdir=LR\n");
        fmt::print(_file, "graph [ fontsize=22 ];\n");
        fmt::print(_file, "labelloc=\"t\";\n");
        fmt::print(_file, "label=\"{}\";\n", title);

        _nodes.clear();
        _edges.clear();

        uint64_t address = 0;
        auto& assembler = _session.assembler();
        const auto& blocks = _session.byte_code_emitter().blocks();
        for (auto block : blocks.as_list())
            add_vertex(assembler, block, address);

        for (const auto& node : _nodes)
            fmt::print(_file, "\t{}\n", node);
        for (const auto& edge : _edges)
            fmt::print(_file, "\t{}\n", edge);

        fmt::print(_file, "}}\n");
    }

    void cfg_formatter::add_vertex(
            vm::assembler& assembler,
            vm::basic_block* block,
            uint64_t address) {
        vm::listing_source_file_t listing_file{};
        assembler.disassemble(&listing_file, block, address);

        std::string byte_code;
        for (const auto& line : listing_file.lines)
            byte_code += fmt::format(R"({}<BR ALIGN="LEFT"/>)", html_escape(line.source));

        std::string html;
        html = R"(<TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0">)";
        html += fmt::format("<TR><TD><B>basic_block</B></TD><TD><B>{}</B></TD></TR>", block->id());
        html += fmt::format(R"(<TR><TD ALIGN="LEFT" VALIGN="TOP" COLSPAN="2"><FONT FACE="Menlo" POINT-SIZE="12">{}</FONT></TD></TR>)", byte_code);
        html += "</TABLE>";

        _nodes.insert(fmt::format(
            "{}[shape=plaintext,label=<{}>];",
            get_vertex_name(block),
            html));
        for (auto pred : block->predecessors()) {
            if (pred != nullptr)
                add_predecessor(block, pred);
        }
        for (auto succ : block->successors()) {
            if (succ != nullptr)
                add_successor(block, succ);
        }
    }

    std::string cfg_formatter::html_escape(const std::string& html) {
        std::string escaped{};
        for (const auto& c : html) {
            if (c == '>') {
                escaped += "&gt;";
            } else if (c == '<') {
                escaped += "&lt;";
            } else {
                escaped += c;
            }
        }
        return escaped;
    }

    std::string cfg_formatter::get_vertex_name(vm::basic_block* block) const {
        return fmt::format("bb_{}", block->id());
    }

}