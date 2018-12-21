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

#include <map>
#include <stack>
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <configure.h>
#include <unordered_map>
#include <boost/any.hpp>
#include <vm/vm_types.h>
#include <common/result.h>
#include <common/id_pool.h>
#include <boost/variant.hpp>
#include <boost/filesystem.hpp>
#include <compiler/compiler_types.h>

#define CTRL(c) ((c) & 0x1f)

namespace basecode::debugger {

    enum class breakpoint_type_t : uint8_t {
        simple,
        flag_set,
        flag_clear,
        register_equals
    };

    struct breakpoint_t {
        bool enabled = false;
        uint64_t address = 0;
        breakpoint_type_t type = breakpoint_type_t::simple;
    };

};