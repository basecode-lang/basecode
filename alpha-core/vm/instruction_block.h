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

#pragma once

#include <map>
#include <string>
#include <vector>
#include "terp.h"
#include "label.h"

namespace basecode::vm {

    class instruction_block {
    public:
        instruction_block();

        virtual ~instruction_block();

        void push(double value);

        void push(uint64_t value);

        void call(const std::string& proc_name);

        vm::label* make_label(const std::string& name);

    private:
        std::vector<instruction_block*> _blocks {};
        std::vector<instruction_t> _instructions {};
        std::map<std::string, vm::label*> _labels {};
        std::map<std::string, size_t> _label_to_instruction_map {};
    };

};

