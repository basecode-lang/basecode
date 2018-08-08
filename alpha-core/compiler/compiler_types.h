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

#include <functional>
#include <boost/filesystem.hpp>
#include "elements/element_types.h"

namespace basecode::compiler {

    class session;

    enum class type_access_model_t {
        none,
        value,
        pointer
    };

    enum class type_number_class_t {
        none,
        integer,
        floating_point,
    };

    enum class identifier_usage_t : uint8_t {
        heap = 1,
        stack
    };

    using path_list_t = std::vector<boost::filesystem::path>;

    enum class session_compile_phase_t : uint8_t {
        start,
        success,
        failed
    };

    using session_compile_callback = std::function<void (
        session_compile_phase_t,
        const boost::filesystem::path&)>;

    struct session_options_t {
        bool verbose = false;
        size_t heap_size = 0;
        size_t stack_size = 0;
        boost::filesystem::path compiler_path;
        boost::filesystem::path ast_graph_file;
        boost::filesystem::path dom_graph_file;
        session_compile_callback compile_callback;
    };

}