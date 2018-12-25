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
#include <common/string_support.h>
#include "environment.h"
#include "command_window.h"

namespace basecode::debugger {

    command_window::command_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 {}) {
    }

    void command_window::reset() {
        _input.clear();
        _cursor_x = 0;
        leaveok(ptr(), true);
        mark_dirty();
    }

    void command_window::cursor_left() {
        if (_cursor_x > 0)
            --_cursor_x;
    }

    void command_window::cursor_right() {
        if (_cursor_x < _input.size())
            ++_cursor_x;
    }

    void command_window::on_draw(environment& env) {
        std::string prompt("(command) > ");
        mvwhline(ptr(), 0, 0, ' ', max_width() - 1);
        mvwprintw(ptr(), 0, 0, "%s", (prompt + _input).c_str());
        mvwchgat(
            ptr(),
            0,
            static_cast<int>(prompt.size() + _cursor_x),
            1,
            A_REVERSE,
            0,
            0);
    }

    bool command_window::on_update(environment& env) {
        switch (env.ch()) {
            case 27: {
                reset();
                env.cancel_command();
                return true;
            }
            case 10:
            case KEY_ENTER: {
                _history.push_back(_input);
                _history_index = _history.size() - 1;
                auto parts = common::string_to_list(_input, ' ');
                if (parts.empty()) {
                    // XXX: error!
                    return false;
                }
                _command.name = parts[0];
                parts.erase(parts.begin());
                _command.params = parts;
                auto success = env.execute_command(_command);
                if (!success) {
                    // XXX: ?
                }
                return true;
            }
            case KEY_UP: {
                if (_history.empty())
                    return false;

                _input = _history[_history_index];
                _cursor_x = _input.size();
                if (_history_index > 0)
                    _history_index--;
                return true;
            }
            case KEY_DOWN: {
                if (_history.empty())
                    return false;

                _input = _history[_history_index];
                _cursor_x = _input.size();
                if (_history_index < _history.size()) {
                    _history_index++;
                } else {
                    reset();
                }
                return true;
            }
            case KEY_HOME: {
                _cursor_x = 0;
                return true;
            }
            case KEY_END: {
                _cursor_x = _input.size();
                return true;
            }
            case KEY_LEFT: {
                cursor_left();
                return true;
            }
            case KEY_RIGHT: {
                cursor_right();
                return true;
            }
            case KEY_DC:
            case KEY_BACKSPACE: {
                cursor_left();
                if (_cursor_x >= _input.size()) {
                    _input = _input.substr(0, _cursor_x);
                } else {
                    _input = _input.substr(0, _cursor_x) +
                             _input.substr(_cursor_x + 1, _input.size() - (_cursor_x + 1));
                }
                return true;
            }
            default: {
                if (_cursor_x >= _input.size()) {
                    _input += (char) env.ch();
                } else {
                    _input = _input.substr(0, _cursor_x) +
                             (char) env.ch() +
                             _input.substr(_cursor_x, _input.size() - _cursor_x);
                }
                _cursor_x++;
                return true;
            }
        }
    }

};