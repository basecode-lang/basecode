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

#include "vm_types.h"

namespace basecode::vm {

    class ffi {
    public:
        explicit ffi(size_t heap_size);

        virtual ~ffi();

        void clear();

        void reset();

        bool register_function(
            common::result& r,
            function_signature_t& signature);

        void dump_shared_libraries();

        bool initialize(common::result& r);

        shared_library_t* load_shared_library(
            common::result& r,
            const boost::filesystem::path& path);

        uint64_t call(function_signature_t* func);

        void push(
            const function_value_t& param,
            uint64_t value);

        void calling_convention(ffi_calling_mode_t mode);

        function_signature_t* find_function(uint64_t address);

        shared_library_t* shared_library(const boost::filesystem::path& path);

    private:
        void add_struct_fields(
            DCstruct* s,
            const std::vector<function_value_t>& fields);

        DCstruct* make_struct(const function_value_t& value);

    private:
        size_t _heap_size;
        DCCallVM* _vm = nullptr;
        std::unordered_map<void*, function_signature_t> _foreign_functions {};
        std::unordered_map<std::string, shared_library_t> _shared_libraries {};
    };

};

