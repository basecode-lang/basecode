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

#include "window.h"

namespace basecode::debugger {

    class output_window : public window {
    public:
        output_window(
            window* parent,
            int x,
            int y,
            int width,
            int height);

        void clear();

        void stop_redirect();

        void start_redirect();

        void process_buffers();

    protected:
        void on_draw(environment& env) override;

    private:
        void add_stdout_line(const std::string& line);

    private:
        std::string _stdout_line;
        char _stdout_buffer[4096];
        char _stderr_buffer[4096];
        FILE* _stdout_fp = nullptr;
        FILE* _stderr_fp = nullptr;
        FILE* _old_stdout_fp = nullptr;
        FILE* _old_stderr_fp = nullptr;
        std::vector<std::string> _stderr_lines {};
        std::vector<std::string> _stdout_lines {};
    };

};

