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
        _source_files.clear();
    }

    void assembly_listing::write(FILE* file) {
        size_t count = 0;
        for (const auto& kvp : _source_files) {
            fmt::print(file, "FILE: {:<64} Basecode Alpha Compiler\n", kvp.first);
            fmt::print(file, "      {:>91}\n\n", "Assembly Listing (byte code)");
            fmt::print(file, "LINE    ADDRESS     SOURCE\n");
            size_t line_number = 1;
            for (const auto& line : kvp.second.lines)
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

    listing_source_file_t* assembly_listing::current_source_file() {
        return _current_source_file;
    }

    void assembly_listing::add_source_file(const boost::filesystem::path& path) {
        _source_files.insert(std::make_pair(
            path.string(),
            listing_source_file_t {path}));
    }

    void assembly_listing::select_source_file(const boost::filesystem::path& path) {
        auto it = _source_files.find(path.string());
        if (it == _source_files.end()) {
            _current_source_file = nullptr;
            return;
        }
        _current_source_file = &it->second;
    }

};