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

#include <fstream>
#include <fmt/ostream.h>
#include <parser/lexer.h>
#include <parser/ast_formatter.h>
#include <compiler/elements/program.h>
#include "bytecode_emitter.h"
#include "code_dom_formatter.h"

namespace basecode::compiler {

    bytecode_emitter::bytecode_emitter(
        const bytecode_emitter_options_t& options): _terp(options.heap_size, options.stack_size),
                                                    _options(options) {
    }

    bytecode_emitter::~bytecode_emitter() {
    }

    bool bytecode_emitter::compile(
            common::result& r,
            compiler::session& session) {
        compiler::program program_element(&_terp);
        if (program_element.compile(r, session)) {
        }
        return !r.is_failed();
    }

    bool bytecode_emitter::initialize(common::result& r) {
        return _terp.initialize(r);
    }

}