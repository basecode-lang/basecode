// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <stdarg.h>
#include <cstdio>

extern "C" {
    void fmt_print(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        printf(fmt, ap);
        va_end(ap);
    }
};
