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

#include "compiler_types.h"

namespace basecode::compiler {

    enum class variable_type_t : uint8_t {
        temporary,
        parameter,
        module,
        local,
        return_value
    };

    enum class variable_state_t : uint8_t {
        unknown,
        filled,
        spilled
    };

    struct variable_t {
        ~variable_t();

        int64_t offset = 0;
        std::string path {};
        std::string label {};
        variable_type_t type {};
        std::string base_label {};
        variable_state_t state {};
        variable_map* map = nullptr;
        number_class_t number_class {};
        compiler::identifier* identifier = nullptr;
    };

    class variable_map {
    public:
        explicit variable_map(compiler::session& session);

        void reset();

        bool build(compiler::block* block);

        identifier_by_section_t& module_variables();

        bool emit_prologue(vm::basic_block** basic_block);

        bool emit_epilogue(vm::basic_block** basic_block);

    private:
        bool find_referenced_module_variables(compiler::block* block);

    private:
        compiler::session& _session;
        std::stack<uint16_t> _temps {};
        identifier_by_section_t _module_variables {};
        std::map<std::string, variable_t> _variables {};
    };

}

