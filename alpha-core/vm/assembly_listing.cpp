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

#include <fmt/format.h>
#include "assembly_listing.h"

namespace basecode::vm {

    assembly_listing::assembly_listing() {
    }

    void assembly_listing::reset() {
        while (!_source_file_stack.empty())
            _source_file_stack.pop();
        _source_files.clear();
    }

    void assembly_listing::pop_source_file() {
        if (_source_file_stack.empty())
            return;
        _source_file_stack.pop();
    }

    void assembly_listing::write(FILE* file) {
        size_t count = 0;
        for (const auto& source_file : _source_files) {
            fmt::print(file, "FILE: {:<64} Basecode Alpha Compiler\n", source_file.filename);
            fmt::print(file, "      {:>91}\n\n", "Assembly Listing (byte code)");
            fmt::print(file, "LINE    ADDRESS     SOURCE\n");
            size_t line_number = 1;
            for (const auto& line : source_file.lines)
                fmt::print(
                    file,
                    "{:06d}: ${:08x}   {}\n",
                    line_number++,
                    line.address,
                    line.source);
            count++;
            if (count < _source_files.size())
                fmt::print(file, "\n\n");
        }
    }

    void assembly_listing::push_source_file(size_t index) {
        _source_file_stack.push(index);
    }

    listing_source_file_t* assembly_listing::current_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        return &_source_files[_source_file_stack.top()];
    }

    size_t assembly_listing::add_source_file(const std::string& filename) {
        if (_source_files.capacity() < 256)
            _source_files.reserve(256);
        _source_files.push_back(listing_source_file_t {.filename = filename});
        return _source_files.size() - 1;
    }

};