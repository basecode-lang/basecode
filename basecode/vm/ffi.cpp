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
#include <dyncall/dyncall.h>
#include <dynload/dynload.h>
#include <dyncall/dyncall_struct.h>
#include <dyncall/dyncall_signature.h>
#include "ffi.h"

namespace basecode::vm {

    ffi::ffi(size_t heap_size) : _heap_size(heap_size) {
    }

    ffi::~ffi() {
        dcFree(_vm);
        _vm = nullptr;
    }

    void ffi::clear() {
        _foreign_functions.clear();
        _shared_libraries.clear();
    }

    void ffi::reset() {
        dcReset(_vm);
    }

    bool ffi::register_function(
            common::result& r,
            function_signature_t& signature) {
        if (signature.func_ptr != nullptr) {
            auto it = _foreign_functions.find(signature.func_ptr);
            if (it != _foreign_functions.end())
                return true;
        }

        auto func_ptr = signature.library->symbol_address(signature.symbol);
        if (func_ptr == nullptr) {
            // XXX: add an error message
            return false;
        }

        signature.func_ptr = func_ptr;
        _foreign_functions.insert(std::make_pair(func_ptr, signature));

        return true;
    }

    void ffi::dump_shared_libraries() {
        fmt::print("\n{:32}{:64}{:17}\n", "Image Name", "Symbol Name", "Address");
        fmt::print("{}\n", std::string(180, '-'));
        for (const auto& kvp : _shared_libraries) {
            auto index = 0;
            for (const auto& entry : kvp.second.symbols()) {
                fmt::print(
                    "{:32}{:64}${:016X}\n",
                    index == 0 ? kvp.first.substr(0, std::min<size_t>(32, kvp.first.length())) : "",
                    entry.first.substr(0, std::min<size_t>(64, entry.first.length())),
                    reinterpret_cast<uint64_t>(entry.second));
                ++index;
            }
        }
        fmt::print("\n");
    }

    bool ffi::initialize(common::result& r) {
        _vm = dcNewCallVM(_heap_size);
        _shared_libraries.clear();
        return true;
    }

    shared_library_t* ffi::load_shared_library(
            common::result& r,
            const boost::filesystem::path& path) {
        auto it = _shared_libraries.find(path.string());
        if (it != _shared_libraries.end())
            return &it->second;

        shared_library_t shared_library {};
        if (!shared_library.initialize(r, path))
            return nullptr;

        auto pair = _shared_libraries.insert(std::make_pair(
            path.string(),
            shared_library));

        return &(*pair.first).second;
    }

    uint64_t ffi::call(function_signature_t* func) {
        switch (func->return_value.type) {
            case ffi_types_t::void_type: {
                dcCallVoid(_vm, func->func_ptr);
                return 0;
            }
            case ffi_types_t::bool_type: {
                auto value = static_cast<uint64_t>(dcCallBool(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::char_type: {
                auto value = static_cast<uint64_t>(dcCallChar(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::short_type: {
                auto value = static_cast<uint64_t>(dcCallShort(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::int_type: {
                auto value = static_cast<uint64_t>(dcCallInt(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::long_type: {
                auto value = static_cast<uint64_t>(dcCallLong(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::long_long_type: {
                auto value = static_cast<uint64_t>(dcCallLongLong(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::float_type: {
                auto value = static_cast<uint64_t>(dcCallFloat(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::double_type: {
                auto value = static_cast<uint64_t>(dcCallDouble(
                    _vm,
                    func->func_ptr));
                return value;
            }
            case ffi_types_t::pointer_type: {
                auto value = reinterpret_cast<uint64_t>(dcCallPointer(
                    _vm,
                    func->func_ptr));
                return value;
            }
            default:
                return 0;
//            case ffi_types_t::struct_type: {
//                auto dc_struct = func->return_value.struct_meta_info();
//
//                DCpointer output_value;
//                dcCallStruct(
//                    _vm,
//                    func->func_ptr,
//                    dc_struct,
//                    &output_value);
//
//                return reinterpret_cast<uint64_t>(output_value);
//            }
        }
    }

    void ffi::push(ffi_types_t type, uint64_t value) {
        register_value_alias_t alias {};
        alias.qw = value;

        switch (type) {
            case ffi_types_t::any_type:
            case ffi_types_t::void_type:
                break;
            case ffi_types_t::bool_type:
                dcArgBool(_vm, static_cast<DCbool>(value));
                break;
            case ffi_types_t::char_type:
                dcArgChar(_vm, static_cast<DCchar>(value));
                break;
            case ffi_types_t::short_type:
                dcArgShort(_vm, static_cast<DCshort>(value));
                break;
            case ffi_types_t::long_type:
                dcArgLong(_vm, static_cast<DClong>(value));
                break;
            case ffi_types_t::long_long_type:
                dcArgLongLong(_vm, static_cast<DClonglong>(value));
                break;
            case ffi_types_t::float_type:
                dcArgFloat(_vm, alias.dwf);
                break;
            case ffi_types_t::double_type:
                dcArgDouble(_vm, alias.qwf);
                break;
            case ffi_types_t::pointer_type:
                dcArgPointer(_vm, reinterpret_cast<DCpointer>(value));
                break;
//            case ffi_types_t::struct_type: {
//                auto dc_struct = struct_meta_info();
//                dcArgStruct(
//                    vm,
//                    dc_struct,
//                    reinterpret_cast<DCpointer>(value));
//                break;
//            }
            default:
            case ffi_types_t::int_type: {
                dcArgInt(_vm, static_cast<DCint>(value));
                break;
            }
        }
    }

    void ffi::calling_convention(ffi_calling_mode_t mode) {
        switch (mode) {
            case ffi_calling_mode_t::c_default:
                dcMode(_vm, DC_CALL_C_DEFAULT);
                break;
            case ffi_calling_mode_t::c_ellipsis:
                dcMode(_vm, DC_CALL_C_ELLIPSIS);
                break;
            case ffi_calling_mode_t::c_ellipsis_varargs:
                dcMode(_vm, DC_CALL_C_ELLIPSIS_VARARGS);
                break;
        }
    }

    function_signature_t* ffi::find_function(uint64_t address) {
        auto it = _foreign_functions.find(reinterpret_cast<void*>(address));
        if (it == _foreign_functions.end())
            return nullptr;
        return &it->second;
    }

    shared_library_t* ffi::shared_library(const boost::filesystem::path& path) {
        auto it = _shared_libraries.find(path.string());
        if (it == _shared_libraries.end())
            return nullptr;
        return &it->second;
    }

//    DCstruct* function_value_t::struct_meta_info() {
//        if (_struct_meta_data != nullptr)
//            return _struct_meta_data;
//
//        _struct_meta_data = dcNewStruct(
//            fields.size(),
//            DEFAULT_ALIGNMENT);
//        add_struct_fields(_struct_meta_data);
//        dcCloseStruct(_struct_meta_data);
//
//        return _struct_meta_data;
//    }
//
//    void function_value_t::add_struct_fields(DCstruct* s) {
//        for (auto& value : fields) {
//            switch (value.type) {
//                case ffi_types_t::void_type:
//                    break;
//                case ffi_types_t::bool_type:
//                    dcStructField(s, DC_SIGCHAR_BOOL, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::char_type:
//                    dcStructField(s, DC_SIGCHAR_CHAR, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::short_type:
//                    dcStructField(s, DC_SIGCHAR_SHORT, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::int_type:
//                    dcStructField(s, DC_SIGCHAR_INT, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::long_type:
//                    dcStructField(s, DC_SIGCHAR_LONG, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::long_long_type:
//                    dcStructField(s, DC_SIGCHAR_LONGLONG, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::float_type:
//                    dcStructField(s, DC_SIGCHAR_FLOAT, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::double_type:
//                    dcStructField(s, DC_SIGCHAR_DOUBLE, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::pointer_type:
//                    dcStructField(s, DC_SIGCHAR_POINTER, DEFAULT_ALIGNMENT, 1);
//                    break;
//                case ffi_types_t::struct_type: {
//                    dcStructField(s, DC_SIGCHAR_STRUCT, DEFAULT_ALIGNMENT, 1);
//                    dcSubStruct(s, value.fields.size(), DEFAULT_ALIGNMENT, 1);
//                    value.add_struct_fields(s);
//                    dcCloseStruct(s);
//                    break;
//                }
//            }
//        }
//    }

};