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

#if _MSC_VER 
	#if !defined(API_EXPORT)
		#define API_EXPORT __declspec(dllexport)
	#endif
#else
	#define API_EXPORT
#endif

#include <cstdio>
#include <cstdint>
#include <stdarg.h>
#include <functional>
#include <boost/filesystem.hpp>
#include <compiler/elements/element_types.h>

namespace basecode::compiler {

    class session;

    using path_list_t = std::vector<boost::filesystem::path>;

    enum class session_compile_phase_t : uint8_t {
        start,
        success,
        failed
    };

	using session_compile_callback = std::function<void(
		session_compile_phase_t,
		const boost::filesystem::path&)>;

    struct API_EXPORT session_options_t {
        bool verbose = false;
        size_t heap_size = 0;
        size_t stack_size = 0;
        boost::filesystem::path ast_graph_file;
        boost::filesystem::path dom_graph_file;
        session_compile_callback compile_callback;
    };

}