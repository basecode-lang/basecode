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
#include "footer_window.h"

namespace basecode::debugger {

    footer_window::footer_window(
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

    void footer_window::on_draw(environment& env) {
        auto footer = fmt::format(" F1=Command | F2=Reset | F3=Exit | F8=Step | F9=Run {} ", "");

        size_t pad_length = 0;
        size_t page_width = static_cast<size_t>(max_width());
        if (footer.length() < page_width)
            pad_length = max_width() - footer.length();

        footer = fmt::format(
            "{}{}",
            footer,
            std::string(pad_length, ' '));

        leaveok(ptr(), true);
        wattron(ptr(), A_BOLD | A_REVERSE);
        mvwprintw(ptr(), 0, 0, "%s", footer.c_str());
        wattroff(ptr(), A_BOLD | A_REVERSE);
    }

};