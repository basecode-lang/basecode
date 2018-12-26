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

#include <ncurses.h>
#include "debugger_types.h"

namespace basecode::debugger {

    class window;

    class window {
    public:
        window(window* parent, WINDOW* ptr);

        window(
            window* parent,
            int x,
            int y,
            int width,
            int height,
            const std::string& title);

        int x() const;

        int y() const;

        int width() const;

        void mark_dirty();

        void initialize();

        int height() const;

        WINDOW* ptr() const;

        bool visible() const;

        int max_width() const;

        int max_height() const;

        window* parent() const;

        bool scrollable() const;

        void visible(bool value);

        std::string title() const;

        void scrollable(bool value);

        virtual ~window() = default;

        void draw(environment& env);

        bool update(environment& env);

        void title(const std::string& value);

    protected:
        int row() const;

        void make_win();

        void draw_title();

        int column() const;

        void row(int value);

        void column(int value);

        void print_centered_row(
            int row,
            const std::string& value);

        void print_centered_window(
            const std::string& value);

        virtual void on_draw(environment& env);

        virtual bool on_update(environment& env);

    private:
        int _x = 0;
        int _y = 0;
        int _row = 0;
        int _width = 0;
        int _height = 0;
        int _column = 0;
        int _max_width = 0;
        bool _dirty = true;
        int _max_height = 0;
        bool _visible = true;
        std::string _title {};
        short _color_pair = 1;
        WINDOW* _ptr = nullptr;
        bool _scrollable = false;
        window* _parent = nullptr;
    };

};

