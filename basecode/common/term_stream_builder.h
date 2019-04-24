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

#include <memory>
#include <string>
#include <memory>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <fmt/format.h>

namespace basecode::common {

    using namespace std::literals;

    enum class term_colors_t : uint8_t {
        black = 30,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        light_gray,
        default_color = 39,

        dark_gray = 90,
        light_red,
        light_green,
        light_yellow,
        light_blue,
        light_magenta,
        light_cyan,
        white
    };

    constexpr std::string_view color_code_reset() {
        return "\033[0m"sv;
    }

    std::string color_code(term_colors_t fg_color, term_colors_t bg_color);

    class term_stream {
    public:
        virtual ~term_stream();

        virtual term_stream* color(
            term_colors_t bg,
            term_colors_t fg) = 0;

        virtual term_stream* color_reset() = 0;

        virtual term_stream* dim(bool enabled) = 0;

        virtual term_stream* bold(bool enabled) = 0;

        virtual std::stringstream& underlying() = 0;

        virtual term_stream* blink(bool enabled) = 0;

        virtual term_stream* reverse(bool enabled) = 0;

        virtual term_stream* underline(bool enabled) = 0;

        [[nodiscard]] virtual std::string format() const = 0;

        virtual term_stream* append(const std::string& value) = 0;

        virtual term_stream* append(const std::string_view& value) = 0;

        virtual term_stream* append(const std::string& value, ssize_t width) = 0;

        virtual term_stream* append(const std::string_view& value, ssize_t width) = 0;
    };

    class ascii_stream : public term_stream {
    public:
        explicit ascii_stream(std::stringstream& stream) : _stream(stream) {
        }

        term_stream* color(
                term_colors_t bg,
                term_colors_t fg) override {
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* color_reset() override {
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* dim(bool enabled) override {
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* bold(bool enabled) override {
            return dynamic_cast<term_stream*>(this);
        }

        std::stringstream& underlying() override {
            return _stream;
        }

        term_stream* blink(bool enabled) override {
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* reverse(bool enabled) override {
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* underline(bool enabled) override {
            return dynamic_cast<term_stream*>(this);
        }

        [[nodiscard]] std::string format() const override {
            return _stream.str();
        }

        term_stream* append(const std::string& value) override {
            _stream << value;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* append(const std::string_view& value) override {
            _stream << value;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* append(const std::string& value, ssize_t width) override {
            _stream << std::left << std::setw(width) << value;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* append(const std::string_view& value, ssize_t width) override {
            _stream << std::left << std::setw(width) << value;
            return dynamic_cast<term_stream*>(this);
        }

    private:
        std::stringstream& _stream;
    };

    class ansi_stream : public term_stream {
    public:
        explicit ansi_stream(std::stringstream& stream) : _stream(stream) {
        }

        term_stream* color(
                term_colors_t bg,
                term_colors_t fg) override {
            _stream << color_code(fg, bg);
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* color_reset() override {
            _stream << color_code_reset();
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* dim(bool enabled) override {
            if (enabled)
                _stream << "\033[2m"sv;
            else
                _stream << "\033[22m"sv;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* bold(bool enabled) override {
            if (enabled)
                _stream << "\033[1m"sv;
            else
                _stream << "\033[21m"sv;
            return dynamic_cast<term_stream*>(this);
        }

        std::stringstream& underlying() override {
            return _stream;
        }

        term_stream* blink(bool enabled) override {
            if (enabled)
                _stream << "\033[5m"sv;
            else
                _stream << "\033[25m"sv;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* reverse(bool enabled) override {
            if (enabled)
                _stream << "\033[7m"sv;
            else
                _stream << "\033[27m"sv;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* underline(bool enabled) override {
            if (enabled)
                _stream << "\033[4m"sv;
            else
                _stream << "\033[24m"sv;
            return dynamic_cast<term_stream*>(this);
        }

        [[nodiscard]] std::string format() const override {
            return _stream.str();
        }

        term_stream* append(const std::string& value) override {
            _stream << value;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* append(const std::string_view& value) override {
            _stream << value;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* append(const std::string& value, ssize_t width) override {
            _stream << std::left << std::setw(width) << value;
            return dynamic_cast<term_stream*>(this);
        }

        term_stream* append(const std::string_view& value, ssize_t width) override {
            _stream << std::left << std::setw(width) << value;
            return dynamic_cast<term_stream*>(this);
        }

    private:
        std::stringstream& _stream;
    };

    using term_stream_unique_ptr = std::unique_ptr<term_stream>;

    class term_stream_builder {
    public:
        explicit term_stream_builder(bool enabled);

        term_stream_builder(const term_stream_builder&) = delete;

        [[nodiscard]] bool enabled() const;

        [[nodiscard]] std::string colorize(
            const std::string& text,
            term_colors_t fg_color,
            term_colors_t bg_color = term_colors_t::default_color);

        [[nodiscard]] std::string colorize_range(
            const std::string& text,
            size_t begin,
            size_t end,
            term_colors_t fg_color,
            term_colors_t bg_color = term_colors_t::default_color);

        [[nodiscard]] term_stream_unique_ptr use_stream(std::stringstream& stream) const;

    private:
        bool _enabled;
    };

}

