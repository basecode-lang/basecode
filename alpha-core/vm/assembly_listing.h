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

#include <stack>
#include <string>
#include <vector>

namespace basecode::vm {

    struct listing_source_line_t {
        uint64_t address = 0;
        std::string source {};
    };

    struct listing_source_file_t {
        void add_source_line(
                uint64_t address,
                const std::string& source) {
            lines.push_back(listing_source_line_t {
                .address = address,
                .source = source
            });
        }

        void add_blank_lines(uint16_t count = 1) {
            for (uint16_t i = 0; i < count; i++) {
                lines.push_back(listing_source_line_t {
                    .address = 0,
                    .source = ""
                });
            }
        }

        std::string filename;
        std::vector<listing_source_line_t> lines {};
    };

    class assembly_listing {
    public:
        assembly_listing();

        void reset();

        void write(FILE* file);

        void pop_source_file();

        void push_source_file(size_t index);

        listing_source_file_t* current_source_file();

        size_t add_source_file(const std::string& filename);

    private:
        std::stack<size_t> _source_file_stack {};
        std::vector<listing_source_file_t> _source_files {};
    };

};

