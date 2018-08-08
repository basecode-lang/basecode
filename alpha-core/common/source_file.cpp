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

        const auto number_of_lines = static_cast<int32_t>(_lines_by_number.size());
        const auto target_line = static_cast<int32_t>(location.start().line);
        const auto message_indicator = common::colorizer::colorize(
            "^ " + message,
            common::term_colors_t::red);

        auto start_line = static_cast<int32_t>(location.start().line - 4);
        if (start_line < 0)
            start_line = 0;

        auto stop_line = static_cast<int32_t>(location.end().line + 4);
        if (stop_line >= number_of_lines)
            stop_line = number_of_lines - 1;

        for (int32_t i = start_line; i < stop_line; i++) {
            const auto source_line = line_by_number(static_cast<size_t>(i));
            if (source_line == nullptr)
                break;
            const auto source_text = substring(
                source_line->begin,
                source_line->end);
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

    void source_file::push_mark() {
        _mark_stack.push(_index);
    }

    bool source_file::eof() const {
        return _index > _buffer.size() - 1;
    }

    size_t source_file::pop_mark() {
        if (_mark_stack.empty())
            return _index;
        const auto mark = _mark_stack.top();
        _mark_stack.pop();
        return mark;
    }

    size_t source_file::pos() const {
        return _index;
    }

    bool source_file::empty() const {
        return _buffer.empty();
    }

    void source_file::build_lines(common::result& r) {
        uint32_t line = 0;
        uint32_t columns = 0;
        size_t line_start = 0;

        while (true) {
            auto rune = next(r);
            if (rune == rune_invalid) {
                break;
            } else if (rune == rune_bom) {
                rune = next(r);
            }

            const auto end_of_buffer = rune == rune_eof;
            const auto unix_new_line = rune == '\n';

            if (unix_new_line || end_of_buffer) {
                const auto end = end_of_buffer ? _buffer.size() : _index - 1;
                const auto it = _lines_by_index_range.insert(std::make_pair(
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
                line_start = _index;
                line++;
                columns = 0;
            } else {
                columns++;
            }

            if (rune == rune_eof)
                break;
        }

        seek(0);
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
            const auto file_size = file.tellg();
            file.seekg(0, std::ios::beg);
            _buffer.reserve(static_cast<size_t>(file_size));
            _buffer.insert(_buffer.begin(),
                           std::istream_iterator<uint8_t>(file),
                           std::istream_iterator<uint8_t>());
            build_lines(r);

//            for (size_t i = 0; i < number_of_lines(); i++) {
//                auto line = line_by_number(i);
//                fmt::print("{}\n", substring(line->begin, line->end));
//            }
        } else {
            r.add_message(
                "S001",
                fmt::format("unable to open source file: {}", _path.string()),
                true );
        }
        return !r.is_failed();
    }

    size_t source_file::number_of_lines() const {
        return _lines_by_number.size();
    }

    rune_t source_file::next(common::result& r) {
        if (_index > _buffer.size() - 1)
            return rune_eof;
        size_t width = 1;
        auto ch = _buffer[_index];
        rune_t rune = ch;
        if (ch == 0) {
            r.add_message(
                "S003",
                "illegal character NUL",
                true);
            return rune_invalid;
        } else if (ch >= 0x80) {
            auto cp = utf8_decode(
                (char*)(_buffer.data() + _index),
                _buffer.size() - _index);
            width = cp.width;
            rune = cp.value;
            if (rune == rune_invalid && width == 1) {
                r.add_message(
                    "S001",
                    "illegal utf-8 encoding",
                    true);
                return rune_invalid;
            } else if (rune == rune_bom && _index > 0) {
                r.add_message(
                    "S002",
                    "illegal byte order mark",
                    true);
                return rune_invalid;
            }
        }
        _index += width;
        return rune;
    }

    uint8_t source_file::operator[](size_t index) {
        return _buffer[index];
    }

    const boost::filesystem::path& source_file::path() const {
        return _path;
    }

    uint32_t source_file::column_by_index(size_t index) const {
        auto line = line_by_index(index);
        if (line == nullptr)
            return 0;
        return static_cast<const uint32_t>(index - line->begin);
    }

    std::string source_file::substring(size_t start, size_t end) {
        const auto length = end - start;
        std::string value;
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