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

#include <set>
#include <map>
#include <cstdio>
#include "session.h"
#include "compiler_types.h"

namespace basecode::compiler {

    class cfg_formatter {
    public:
        cfg_formatter(
            compiler::session& session,
            FILE* output_file);

        void format(const std::string& title);

    private:
        static std::string html_escape(const std::string& html);

        void add_vertex(
            vm::assembler& assembler,
            vm::basic_block* block,
            uint64_t address);

        void add_successor(
            vm::basic_block* from,
            vm::basic_block* to,
            const std::string& label = "");

        void add_predecessor(
            vm::basic_block* from,
            vm::basic_block* to,
            const std::string& label = "");

        std::string get_vertex_name(vm::basic_block* block) const;

    private:
        FILE* _file = nullptr;
        compiler::session& _session;
        std::set<std::string> _edges {};
        std::set<std::string> _nodes {};
    };

}

