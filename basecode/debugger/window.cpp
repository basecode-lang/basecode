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

#include "window.h"

namespace basecode::debugger {

    window::window(window* parent, WINDOW* ptr) : _ptr(ptr), _parent(parent) {
        make_win();
    }

    window::window(
            window* parent,
            int x,
            int y,
            int width,
            int height,
            const std::string& title) : _x(x),
                                        _y(y),
                                        _width(width),
                                        _height(height),
                                        _title(title),
                                        _parent(parent) {
        make_win();
    }

    int window::x() const {
        return _x;
    }

    int window::y() const {
        return _y;
    }

    int window::row() const {
        return _row;
    }

    void window::make_win() {
        if (_ptr == nullptr)
            _ptr = newwin(_height, _width, _y, _x);

        if (_height > 1)
            box(_ptr, 0, 0);

        scrollok(_ptr, _scrollable);

        getmaxyx(_ptr, _max_height, _max_width);

        if (!_title.empty())
            draw_title();

        wrefresh(_ptr);
    }

    void window::mark_dirty() {
        _dirty = true;
    }

    void window::draw_title() {
        mvwhline(_ptr, 0, 1, ' ', _max_width - 2);
        mvwprintw(_ptr, 0, 2, "%s", _title.c_str());
        mvwchgat(_ptr, 0, 1, _max_width - 2, A_REVERSE, _color_pair, 0);
    }

    int window::width() const {
        return _width;
    }

    int window::height() const {
        return _height;
    }

    int window::column() const {
        return _column;
    }

    void window::row(int value) {
        if (_row != value) {
            _row = value;
            _dirty = true;
        }
    }

    WINDOW* window::ptr() const {
        return _ptr;
    }

    int window::max_width() const {
        return _max_width;
    }

    int window::max_height() const {
        return _max_height;
    }

    window* window::parent() const {
        return _parent;
    }

    void window::print_centered_row(
            int row,
            const std::string& value) {
        mvwprintw(
            _ptr,
            row,
            static_cast<int>((_max_width - value.length()) / 2),
            "%s",
            value.c_str());
    }

    void window::column(int value) {
        if (_column != value) {
            _column = value;
            _dirty = true;
        }
    }

    bool window::scrollable() const {
        return _scrollable;
    }

    std::string window::title() const {
        return _title;
    }

    void window::draw(environment& env) {
        if (!_dirty)
            return;
        on_draw(env);
        wrefresh(_ptr);
        _dirty = false;
    }

    void window::scrollable(bool value) {
        if (_scrollable != value) {
            _scrollable = value;
            scrollok(_ptr, _scrollable);
        }
    }

    bool window::update(environment& env) {
        return on_update(env);
    }

    void window::on_draw(environment& env) {
    }

    bool window::on_update(environment& env) {
        return false;
    }

    void window::title(const std::string& value) {
        if (_title != value) {
            _title = value;
            _dirty = true;
        }
    }

    void window::print_centered_window(const std::string& value) {
        print_centered_row(_max_height / 2, value);
    }

};