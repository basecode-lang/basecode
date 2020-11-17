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

#include <sstream>
#include <basecode/format/format.h>
#include "transforms.h"

namespace basecode::strings {

    void word_wrap(
            string_t& text,
            size_t width,
            size_t right_pad,
            const char& fill) {
        size_t line_begin = 0;

        while (line_begin < text.size()) {
            const auto ideal_end = line_begin + width;
            size_t line_end = ideal_end <= text.size() ?
                              ideal_end :
                              text.size() - 1;

            if (line_end == text.size() - 1)
                ++line_end;
            else if (std::isspace(text[line_end])) {
                text[line_end++] = '\n';
                text.insert(line_end++, right_pad, fill);
            } else {
                auto end = line_end;
                while (end > line_begin && !std::isspace(text[end]))
                    --end;

                if (end != line_begin) {
                    line_end = end;
                    text[line_end++] = '\n';
                    text.insert(line_end++, right_pad, fill);
                } else {
                    text.insert(line_end++, 1, '\n');
                    text.insert(line_end++, right_pad, fill);
                }
            }

            line_begin = line_end;
        }
    }

    bool for_each_line(
            const string_t& buffer,
            const line_callback_t& callback) {
        int32_t i = 0;
        int32_t start_pos = 0;

        while (i < buffer.size()) {
            if (buffer[i] == '\n') {
                if (!callback(buffer.slice(start_pos, i - start_pos)))
                    return false;
                start_pos = i + 1;
            }
            ++i;
        }

        if (i > start_pos)
            if (!callback(buffer.slice(start_pos, i - start_pos)))
                return false;

        return true;
    }

    bool string_to_hash_table(
            const string_t& value,
            string_map_t& result_table,
            const char& sep) {
        return for_each_line(
            value,
            [&](std::string_view slice) {
                const auto parts = string_to_list(slice, sep);
                if (parts.size() == 2)
                    result_table.insert(parts[0], parts[1]);
                return true;
            });
    }

    unitized_byte_size_t size_to_units(size_t size) {
        static std::string_view units[] = {
            "bytes"sv,
            "KB"sv,
            "MB"sv,
            "GB"sv,
            "TB"sv,
            "PB"sv,
            "EB"sv,
            "ZB"sv,
            "YB"sv
        };

        auto i = 0;
        while (size > 1024) {
            size /= 1024;
            i++;
        }

        return unitized_byte_size_t{
            i > 1 ?
            format::format("{}.{}", i, size) :
            format::format("{}", size),
            units[i]
        };
    }

    string_t remove_underscores(const std::string_view& value) {
        format::memory_buffer_t buffer{};
        for (const auto& c : value)
            if (c != '_') format::format_to(buffer, "{}", c);
        return format::to_string(buffer);
    }

    string_t list_to_string(const string_list_t& list, const char sep) {
        format::memory_buffer_t buffer{};
        for (size_t i = 0; i < list.size(); i++) {
            if (i > 0)
                format::format_to(buffer, "{}", sep);
            format::format_to(buffer, "{}", list[i]);
        }
        return format::to_string(buffer);
    }

    string_list_t string_to_list(const string_t& value, const char sep) {
        string_list_t list{};

        uint32_t curr_pos = 0;
        uint32_t start_pos = 0;
        for (const auto& c : value) {
            if (c == sep) {
                const auto slice = value.slice(
                    start_pos,
                    curr_pos - start_pos);
                list.add(slice);
                start_pos = curr_pos + 1;
            }
            ++curr_pos;
        }

        if (curr_pos > start_pos) {
            const auto slice = value.slice(
                start_pos,
                curr_pos - start_pos);
            list.add(slice);
        }

        return list;
    }

    string_t pad_to(const string_t& str, const size_t num, const char padding) {
        if (num > str.size()) {
            auto padded = str;
            padded.insert(0, num - padded.size(), padding);
            return padded;
        } else {
            return str;
        }
    }

}