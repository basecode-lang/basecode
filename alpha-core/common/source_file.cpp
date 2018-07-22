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

#include <fstream>
#include <iterator>
#include <fmt/format.h>
#include "rune.h"
#include "source_file.h"

namespace basecode::common {

    source_file::source_file(const std::filesystem::path& path) : _path(path) {
    }

    source_file::~source_file() {
    }

    rune_t source_file::next() {
        if (_index >= _buffer.size())
            return rune_eof;
        rune_t c = _buffer[_index];
        _index++;
        return c;
    }

    bool source_file::eof() const {
        return _index >= _buffer.size();
    }

    bool source_file::empty() const {
        return _buffer.empty();
    }

    void source_file::build_lines() {
        uint32_t line = 0;
        uint32_t columns = 0;
        size_t line_start = 0;

        for (size_t i = 0; i < _buffer.size(); i++) {
            auto end_of_buffer = i == _buffer.size() - 1;
            if (_buffer[i] == '\n' || end_of_buffer) {
                auto end = end_of_buffer ? _buffer.size() : i;
                auto it = _lines_by_index_range.insert(std::make_pair(
                    std::make_pair(line_start, end),
                    source_file_line_t {
                        .end = end,
                        .begin = line_start,
                        .line = line,
                        .columns = columns
                    }));
                _lines_by_number.insert(std::make_pair(
                    line,
                    &it.first->second));
                line_start = i + 1;
                line++;
                columns = 0;
            } else {
                columns++;
            }
        }
    }

    size_t source_file::length() const {
        return _buffer.size();
    }

    bool source_file::load(common::result& r) {
        _buffer.clear();
        _lines_by_number.clear();
        _lines_by_index_range.clear();

        std::ifstream file(_path, std::ios::in | std::ios::binary);
        if (file.is_open()) {
            file.unsetf(std::ios::skipws);
            file.seekg(0, std::ios::end);
            auto file_size = file.tellg();
            file.seekg(0, std::ios::beg);
            _buffer.reserve(static_cast<size_t>(file_size));
            _buffer.insert(_buffer.begin(),
                           std::istream_iterator<uint8_t>(file),
                           std::istream_iterator<uint8_t>());
            build_lines();
        } else {
            r.add_message(
                "S001",
                fmt::format("unable to open source file: {}", _path.string()),
                true );
        }
        return true;
    }

    size_t source_file::number_of_lines() const {
        return _lines_by_number.size();
    }

    uint8_t source_file::operator[](size_t index) {
        return _buffer[index];
    }

    const std::filesystem::path& source_file::path() const {
        return _path;
    }

    std::string source_file::substring(size_t start, size_t end) {
        std::string value;
        auto length = end - start;
        value.reserve(length);
        value.assign((const char*)_buffer.data(), start, length);
        return value;
    }

    const source_file_line_t* source_file::line_by_number(size_t line) const {
        auto it = _lines_by_number.find(line);
        if (it == _lines_by_number.end())
            return nullptr;
        return it->second;
    }

    const source_file_line_t* source_file::line_by_index(size_t index) const {
        auto it = _lines_by_index_range.find(std::make_pair(index, index));
        if (it == _lines_by_index_range.end())
            return nullptr;
        return &it->second;
    }

};