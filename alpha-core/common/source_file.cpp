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
#include <sstream>
#include <iterator>
#include <fmt/format.h>
#include "rune.h"
#include "colorizer.h"
#include "source_file.h"

namespace basecode::common {

    source_file::source_file(const boost::filesystem::path& path) : _path(path) {
    }

    source_file::~source_file() {
    }

    void source_file::error(
            common::result& r,
            const std::string& code,
            const std::string& message,
            const common::source_location& location) {
        std::stringstream stream;

        int32_t number_of_lines = static_cast<int32_t>(_lines_by_number.size());

        auto start_line = static_cast<int32_t>(location.start().line - 4);
        if (start_line < 0)
            start_line = 0;

        auto stop_line = static_cast<int32_t>(location.end().line + 4);
        if (stop_line >= number_of_lines)
            stop_line = number_of_lines - 1;

        auto message_indicator = common::colorizer::colorize(
            "^ " + message,
            common::term_colors_t::red);
        int32_t target_line = static_cast<int32_t>(location.start().line);
        for (int32_t i = start_line; i < stop_line; i++) {
            auto source_line = line_by_number(static_cast<size_t>(i));
            if (source_line == nullptr)
                break;
            auto source_text = substring(source_line->begin, source_line->end);
            if (i == target_line) {
                stream << fmt::format("{:04d}: ", i + 1)
                       << common::colorizer::colorize_range(
                           source_text,
                           location.start().column,
                           location.end().column,
                           common::term_colors_t::yellow,
                           common::term_colors_t::blue) << "\n"
                       << fmt::format("{}{}",
                              std::string(6 + location.start().column, ' '),
                              message_indicator);
            } else {
                stream << fmt::format("{:04d}: ", i + 1)
                       << source_text;
            }

            if (i < static_cast<int32_t>(stop_line - 1))
                stream << "\n";
        }

        r.add_message(
            code,
            fmt::format(
                "({}@{}:{}) {}",
                _path.filename().string(),
                location.start().line + 1,
                location.start().column + 1,
                message),
            stream.str(),
            true);
    }

    rune_t source_file::next() {
        if (_index >= _buffer.size())
            return rune_eof;
        rune_t c = _buffer[_index];
        _index++;
        return c;
    }

    void source_file::push_mark() {
        _mark_stack.push(_index);
    }

    bool source_file::eof() const {
        return _index >= _buffer.size();
    }

    size_t source_file::pop_mark() {
        if (_mark_stack.empty())
            return _index;
        auto mark = _mark_stack.top();
        _mark_stack.pop();
        return mark;
    }

    size_t source_file::pos() const {
        return _index;
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

    size_t source_file::current_mark() {
        if (_mark_stack.empty())
            return _index;
        return _mark_stack.top();
    }

    size_t source_file::length() const {
        return _buffer.size();
    }

    void source_file::restore_top_mark() {
        if (_mark_stack.empty())
            return;
        _index = _mark_stack.top();
    }

    void source_file::seek(size_t index) {
        _index = index;
    }

    bool source_file::load(common::result& r) {
        _buffer.clear();
        _lines_by_number.clear();
        _lines_by_index_range.clear();

        std::ifstream file(
            _path.string(),
            std::ios::in | std::ios::binary);
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

    const boost::filesystem::path& source_file::path() const {
        return _path;
    }

    std::string source_file::substring(size_t start, size_t end) {
        std::string value;
        auto length = end - start;
        value.reserve(length);
        value.assign((const char*)_buffer.data(), start, length);
        return value;
    }

    const uint32_t source_file::column_by_index(size_t index) const {
        auto line = line_by_index(index);
        if (line == nullptr)
            return 0;
        return static_cast<const uint32_t>(index - line->begin);
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