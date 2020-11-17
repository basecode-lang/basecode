// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/io/text.h>
#include <basecode/errors/errors.h>
#include "source_buffer.h"

namespace basecode::utf8 {

    source_buffer_t::source_buffer_t(
            memory::allocator_t* allocator) : _allocator(allocator),
                                              _lines(allocator),
                                              _lines_by_index_range(allocator) {
        assert(_allocator);
    }

    source_buffer_t::~source_buffer_t() {
        if (_buffer)
            _allocator->deallocate(_buffer);
        if (_reader) {
            _reader->~reader_t();
            _allocator->deallocate(_reader);
        }
    }

    bool source_buffer_t::load(
            result_t& r,
            const adt::string_t& buffer) {
        if (_buffer) {
            _allocator->deallocate(_buffer);
            _buffer_size = 0;

            _lines.clear();
            _lines_by_index_range.clear();
        }

        _buffer_size = buffer.size() + 1;
        _buffer = (char*)_allocator->allocate(_buffer_size);
        std::memcpy(_buffer, buffer.begin(), buffer.size());
        _buffer[_buffer_size - 1] = '\n';

        auto mem = _allocator->allocate(
            sizeof(reader_t),
            alignof(reader_t));
        _reader = new (mem) reader_t(
            _allocator,
            std::string_view(_buffer, _buffer_size));

        index_lines(r);

        return true;
    }

    bool source_buffer_t::load(
            result_t& r,
            const path_t& path) {
        _path = path;

        std::stringstream stream{};
        if (!io::text::read(r, _path.string(), stream)) {
            errors::add_error(
                r,
                errors::utf8_module::unable_to_open_file,
                _path.string());
            return false;
        }

        const auto& str = stream.str();
        return load(r, string_t(str.c_str(), str.size()));
    }

    void source_buffer_t::push_mark() {
        _reader->push_mark();
    }

    bool source_buffer_t::eof() const {
        return _reader->eof();
    }

    void source_buffer_t::dump_lines() {
        for (size_t i = 0; i < number_of_lines(); i++) {
            auto line = line_by_number(i);
            format::print("{}\n", substring(line->begin, line->end));
        }
    }

    size_t source_buffer_t::pop_mark() {
        return _reader->pop_mark();
    }

    size_t source_buffer_t::pos() const {
        return _reader->pos();
    }

    bool source_buffer_t::empty() const {
        return _buffer_size == 0;
    }

    size_t source_buffer_t::current_mark() {
        return _reader->current_mark();
    }

    size_t source_buffer_t::length() const {
        return _buffer_size;
    }

    uint32_t source_buffer_t::width() const {
        return _reader->width();
    }

    void source_buffer_t::restore_top_mark() {
        _reader->restore_top_mark();
    }

    bool source_buffer_t::seek(size_t index) {
        return _reader->seek(index);
    }

    rune_t source_buffer_t::curr(result_t& r) {
        return _reader->curr(r);
    }

    rune_t source_buffer_t::prev(result_t& r) {
        return _reader->prev(r);
    }

    rune_t source_buffer_t::next(result_t& r) {
        return _reader->next(r);
    }

    const path_t& source_buffer_t::path() const {
        return _path;
    }

    bool source_buffer_t::move_next(result_t& r) {
        return _reader->move_next(r);
    }

    bool source_buffer_t::move_prev(result_t& r) {
        return _reader->move_prev(r);
    }

    void source_buffer_t::index_lines(result_t& r) {
        int32_t line = 0;
        int32_t columns = 0;
        int32_t line_start = 0;

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
                const int32_t end = end_of_buffer ? _buffer_size : _reader->pos() - 1;
                _lines.add(source_buffer_line_t {
                    .end = end,
                    .begin = line_start,
                    .line = line,
                    .columns = columns
                });
                _lines_by_index_range.insert(source_buffer_range_t{end, line, line_start});
                line_start = _reader->pos();
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

    size_t source_buffer_t::number_of_lines() const {
        return _lines.size();
    }

    uint8_t source_buffer_t::operator[](size_t index) {
        return _buffer[index];
    }

    memory::allocator_t* source_buffer_t::allocator() {
        return _allocator;
    }

    std::string_view source_buffer_t::substring(size_t start, size_t end) const {
        const auto length = end - start;
        return _reader->make_slice(start, length);
    }

    const source_buffer_line_t* source_buffer_t::line_by_number(size_t line) const {
        return &_lines[line];
    }

    const source_buffer_line_t* source_buffer_t::line_by_index(int32_t index) const {
        const auto self = const_cast<source_buffer_t*>(this);
        auto node = self->_lines_by_index_range.search(source_buffer_range_t{index, 0, index});
        return node ? &_lines[node->key.line] : nullptr;
    }

    std::string_view source_buffer_t::make_slice(size_t offset, size_t length) const {
        return _reader->make_slice(offset, length);
    }

}