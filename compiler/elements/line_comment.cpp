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

#include "line_comment.h"

namespace basecode::compiler {

    line_comment::line_comment(
            element* parent,
            const std::string& value) : element(parent),
                                        _value(value) {
    }

};