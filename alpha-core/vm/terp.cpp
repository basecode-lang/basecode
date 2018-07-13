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

#include <sstream>
#include <climits>
#include <iomanip>
#include <fmt/format.h>
#include <common/bytes.h>
#include <common/hex_formatter.h>
#include "terp.h"
#include "instruction_block.h"

namespace basecode::vm {

    static inline uint64_t rotl(uint64_t n, uint8_t c) {
        const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n << c) | (n >> ((-c) & mask));
    }

    static inline uint64_t rotr(uint64_t n, uint8_t c) {
        const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n >> c) | (n << ((-c) & mask));
    }

    ///////////////////////////////////////////////////////////////////////////

    function_value_t::~function_value_t() {
        if (_struct_meta_data != nullptr)
            delete _struct_meta_data;
    }

    DCstruct* function_value_t::struct_meta_info() {
        if (_struct_meta_data != nullptr)
            return _struct_meta_data;

        _struct_meta_data = dcNewStruct(
            fields.size(),
            DEFAULT_ALIGNMENT);
        add_struct_fields(_struct_meta_data);
        dcCloseStruct(_struct_meta_data);

        return _struct_meta_data;
    }

    void function_value_t::push(DCCallVM* vm, uint64_t value) {
        switch (type) {
            case ffi_types_t::void_type:
                break;
            case ffi_types_t::bool_type:
                dcArgBool(vm, static_cast<DCbool>(value));
                break;
            case ffi_types_t::char_type:
                dcArgChar(vm, static_cast<DCchar>(value));
                break;
            case ffi_types_t::short_type:
                dcArgShort(vm, static_cast<DCshort>(value));
                break;
            case ffi_types_t::int_type:
                dcArgInt(vm, static_cast<DCint>(value));
                break;
            case ffi_types_t::long_type:
                dcArgLong(vm, static_cast<DClong>(value));
                break;
            case ffi_types_t::long_long_type:
                dcArgLongLong(vm, static_cast<DClonglong>(value));
                break;
            case ffi_types_t::float_type:
                dcArgFloat(vm, static_cast<DCfloat>(value));
                break;
            case ffi_types_t::double_type:
                dcArgDouble(vm, static_cast<DCdouble>(value));
                break;
            case ffi_types_t::pointer_type:
                dcArgPointer(vm, reinterpret_cast<DCpointer>(value));
                break;
            case ffi_types_t::struct_type: {
                auto dc_struct = struct_meta_info();
                dcArgStruct(
                    vm,
                    dc_struct,
                    reinterpret_cast<DCpointer>(value));
                break;
            }
        }
    }

    void function_value_t::add_struct_fields(DCstruct* s) {
        for (auto& value : fields) {
            switch (value.type) {
                case ffi_types_t::void_type:
                    break;
                case ffi_types_t::bool_type:
                    dcStructField(s, DC_SIGCHAR_BOOL, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::char_type:
                    dcStructField(s, DC_SIGCHAR_CHAR, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::short_type:
                    dcStructField(s, DC_SIGCHAR_SHORT, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::int_type:
                    dcStructField(s, DC_SIGCHAR_INT, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::long_type:
                    dcStructField(s, DC_SIGCHAR_LONG, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::long_long_type:
                    dcStructField(s, DC_SIGCHAR_LONGLONG, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::float_type:
                    dcStructField(s, DC_SIGCHAR_FLOAT, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::double_type:
                    dcStructField(s, DC_SIGCHAR_DOUBLE, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::pointer_type:
                    dcStructField(s, DC_SIGCHAR_POINTER, DEFAULT_ALIGNMENT, 1);
                    break;
                case ffi_types_t::struct_type: {
                    dcStructField(s, DC_SIGCHAR_STRUCT, DEFAULT_ALIGNMENT, 1);
                    dcSubStruct(s, value.fields.size(), DEFAULT_ALIGNMENT, 1);
                    value.add_struct_fields(s);
                    dcCloseStruct(s);
                    break;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    void function_signature_t::apply_calling_convention(DCCallVM* vm) {
        switch (calling_mode) {
            case ffi_calling_mode_t::c_default:
                dcMode(vm, DC_CALL_C_DEFAULT);
                break;
            case ffi_calling_mode_t::c_ellipsis:
                dcMode(vm, DC_CALL_C_ELLIPSIS);
                break;
            case ffi_calling_mode_t::c_ellipsis_varargs:
                dcMode(vm, DC_CALL_C_ELLIPSIS_VARARGS);
                break;
        }
    }

    uint64_t function_signature_t::call(DCCallVM* vm, uint64_t address) {
        switch (return_value.type) {
            case ffi_types_t::void_type:
                dcCallVoid(vm, reinterpret_cast<DCpointer>(address));
                break;
            case ffi_types_t::bool_type: {
                auto value = static_cast<uint64_t>(dcCallBool(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::char_type: {
                auto value = static_cast<uint64_t>(dcCallChar(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::short_type: {
                auto value = static_cast<uint64_t>(dcCallShort(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::int_type: {
                auto value = static_cast<uint64_t>(dcCallInt(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::long_type: {
                auto value = static_cast<uint64_t>(dcCallLong(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::long_long_type: {
                auto value = static_cast<uint64_t>(dcCallLongLong(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::float_type: {
                auto value = static_cast<uint64_t>(dcCallFloat(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::double_type: {
                auto value = static_cast<uint64_t>(dcCallDouble(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::pointer_type: {
                auto value = reinterpret_cast<uint64_t>(dcCallPointer(
                    vm,
                    reinterpret_cast<DCpointer>(address)));
                return value;
            }
            case ffi_types_t::struct_type: {
                auto dc_struct = return_value.struct_meta_info();

                DCpointer output_value;
                dcCallStruct(
                    vm,
                    reinterpret_cast<DCpointer>(address),
                    dc_struct,
                    &output_value);

                return reinterpret_cast<uint64_t>(output_value);
            }
        }

        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool shared_library_t::initialize(
            common::result& r,
            const std::filesystem::path& path) {
        _library = dlLoadLibrary(path.c_str());
        if (_library == nullptr) {
            r.add_message(
                "B062",
                fmt::format("unable to load library image file: {}.", path.string()),
                true);
            return false;
        }
        get_library_path();
        return true;
    }

    bool shared_library_t::initialize(common::result& r) {
        _library = dlLoadLibrary(nullptr);
        if (_library == nullptr) {
            r.add_message(
                "B062",
                fmt::format("unable to load library image for self."),
                true);
            return false;
        }
        get_library_path();
        return true;
    }

    void shared_library_t::get_library_path() {
        if (_library == nullptr)
            return;

        char library_path[PATH_MAX];
        dlGetLibraryPath(_library, library_path, PATH_MAX);
        _path = library_path;
    }

    void shared_library_t::load_symbols(const char* path) {
        _symbols.clear();
        auto symbol_ptr = dlSymsInit(path);
        if (symbol_ptr != nullptr) {
            int count = dlSymsCount(symbol_ptr);
            for (int i = 0; i < count; i++) {
                const char* symbol_name = dlSymsName(symbol_ptr, i);
                if (symbol_name != nullptr)
                    _symbols.insert(std::make_pair(symbol_name, nullptr));
            }
            dlSymsCleanup(symbol_ptr);
        }
    }

    bool shared_library_t::exports_symbol(const std::string& symbol_name) {
        return _symbols.count(symbol_name) > 0;
    }

    void* shared_library_t::symbol_address(const std::string& symbol_name) {
        auto it = _symbols.find(symbol_name);
        if (it == _symbols.end()) {
            auto func_ptr = dlFindSymbol(_library, symbol_name.c_str());
            _symbols.insert(std::make_pair(symbol_name, func_ptr));
            return func_ptr;
        }

        if (it->second == nullptr) {
            it->second = dlFindSymbol(_library, symbol_name.c_str());
        }

        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    size_t instruction_t::encoding_size() const {
        size_t encoding_size = base_size;

        for (size_t i = 0; i < operands_count; i++) {
            encoding_size += 1;

            if ((operands[i].is_reg())) {
                encoding_size += sizeof(uint8_t);
            } else {
                switch (size) {
                    case op_sizes::none:
                        break;
                    case op_sizes::byte:
                        encoding_size += sizeof(uint8_t);
                        break;
                    case op_sizes::word:
                        encoding_size += sizeof(uint16_t);
                        break;
                    case op_sizes::dword:
                        if (operands[i].is_integer())
                            encoding_size += sizeof(uint32_t);
                        else
                            encoding_size += sizeof(float);
                        break;
                    case op_sizes::qword:
                        if (operands[i].is_integer())
                            encoding_size += sizeof(uint64_t);
                        else
                            encoding_size += sizeof(double);
                        break;
                }
            }
        }

        encoding_size = align(encoding_size, alignment);

        return encoding_size;
    }

    size_t instruction_t::align(uint64_t value, size_t size) const {
        auto offset = value % size;
        return offset ? value + (size - offset) : value;
    }

    size_t instruction_t::decode(common::result& r, uint8_t* heap, uint64_t address) {
        if (address % alignment != 0) {
            r.add_message(
                "B003",
                fmt::format("instruction alignment violation: alignment = {} bytes, address = ${:016X}",
                            alignment,
                            address),
                true);
            return 0;
        }

        uint8_t* encoding_ptr = heap + address;
        uint8_t encoding_size = *encoding_ptr;
        op = static_cast<op_codes>(*(encoding_ptr + 1));
        uint8_t op_size_and_operands_count = static_cast<uint8_t>(*(encoding_ptr + 2));
        size = static_cast<op_sizes>(common::get_upper_nybble(op_size_and_operands_count));
        operands_count = common::get_lower_nybble(op_size_and_operands_count);

        size_t offset = base_size;
        for (size_t i = 0; i < operands_count; i++) {
            operands[i].type = static_cast<operand_encoding_t::flags_t>(*(encoding_ptr + offset));
            ++offset;

            if ((operands[i].is_reg())) {
                operands[i].value.r8 = *(encoding_ptr + offset);
                ++offset;
            } else {
                switch (size) {
                    case op_sizes::byte: {
                        uint8_t* constant_value_ptr = encoding_ptr + offset;
                        operands[i].value.u64 = *constant_value_ptr;
                        offset += sizeof(uint8_t);
                        break;
                    }
                    case op_sizes::word: {
                        uint16_t* constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                        operands[i].value.u64 = *constant_value_ptr;
                        offset += sizeof(uint16_t);
                        break;
                    }
                    case op_sizes::dword: {
                        if (operands[i].is_integer()) {
                            uint32_t* constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            operands[i].value.u64 = *constant_value_ptr;
                            offset += sizeof(uint32_t);
                        } else {
                            float* constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            operands[i].value.d64 = *constant_value_ptr;
                            offset += sizeof(float);
                        }
                        break;
                    }
                    case op_sizes::qword: {
                        if (operands[i].is_integer()) {
                            uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            operands[i].value.u64 = *constant_value_ptr;
                            offset += sizeof(uint64_t);
                        } else {
                            double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            operands[i].value.d64 = *constant_value_ptr;
                            offset += sizeof(double);
                        }
                        break;
                    }
                    case op_sizes::none: {
                        if (operands[i].is_integer()) {
                            r.add_message(
                                "B010",
                                "constant integers cannot have a size of 'none'.",
                                true);
                        } else {
                            r.add_message(
                                "B010",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.",
                                true);
                        }
                        break;
                    }
                }
            }
        }

        return encoding_size;
    }

    size_t instruction_t::encode(common::result& r, uint8_t* heap, uint64_t address) {
        if (address % alignment != 0) {
            r.add_message(
                "B003",
                fmt::format("instruction alignment violation: alignment = {} bytes, address = ${:016X}",
                            alignment,
                            address),
                true);
            return 0;
        }

        uint8_t encoding_size = base_size;
        size_t offset = base_size;

        auto encoding_ptr = heap + address;
        *(encoding_ptr + 1) = static_cast<uint8_t>(op);

        uint8_t size_type_and_operand_count = 0;
        size_type_and_operand_count = common::set_upper_nybble(
            size_type_and_operand_count,
            static_cast<uint8_t>(size));
        size_type_and_operand_count = common::set_lower_nybble(
            size_type_and_operand_count,
            operands_count);
        *(encoding_ptr + 2) = size_type_and_operand_count;

        for (size_t i = 0; i < operands_count; i++) {
            *(encoding_ptr + offset) = static_cast<uint8_t>(operands[i].type);
            ++offset;
            ++encoding_size;

            if (operands[i].is_reg()) {
                *(encoding_ptr + offset) = operands[i].value.r8;
                ++offset;
                ++encoding_size;
            } else {
                switch (size) {
                    case op_sizes::byte: {
                        uint8_t* constant_value_ptr = encoding_ptr + offset;
                        *constant_value_ptr = static_cast<uint8_t>(operands[i].value.u64);
                        offset += sizeof(uint8_t);
                        encoding_size += sizeof(uint8_t);
                        break;
                    }
                    case op_sizes::word: {
                        uint16_t* constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                        *constant_value_ptr = static_cast<uint16_t>(operands[i].value.u64);
                        offset += sizeof(uint16_t);
                        encoding_size += sizeof(uint16_t);
                        break;
                    }
                    case op_sizes::dword: {
                        if (operands[i].is_integer()) {
                            uint32_t* constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            *constant_value_ptr = static_cast<uint32_t>(operands[i].value.u64);
                            offset += sizeof(uint32_t);
                            encoding_size += sizeof(uint32_t);
                        } else {
                            float* constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            *constant_value_ptr = static_cast<float>(operands[i].value.d64);
                            offset += sizeof(float);
                            encoding_size += sizeof(float);
                        }
                        break;
                    }
                    case op_sizes::qword: {
                        if (operands[i].is_integer()) {
                            uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            *constant_value_ptr = operands[i].value.u64;
                            offset += sizeof(uint64_t);
                            encoding_size += sizeof(uint64_t);
                        } else {
                            double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            *constant_value_ptr = operands[i].value.d64;
                            offset += sizeof(double);
                            encoding_size += sizeof(double);
                        }
                        break;
                    }
                    case op_sizes::none:
                        if (operands[i].is_integer()) {
                            r.add_message(
                                "B009",
                                "constant integers cannot have a size of 'none'.",
                                true);
                        } else {
                            r.add_message(
                                "B009",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.",
                                true);
                        }
                        break;
                }
            }
        }

        encoding_size = static_cast<uint8_t>(align(encoding_size, alignment));
        *encoding_ptr = encoding_size;

        return encoding_size;
    }

    void instruction_t::patch_branch_address(uint64_t address, uint8_t index) {
        operands[index].value.u64 = align(address, alignment);
    }

    std::string instruction_t::disassemble(const id_resolve_callable& id_resolver) const {
        std::stringstream stream;

        auto op_name = op_code_name(op);
        if (!op_name.empty()) {
            std::stringstream mnemonic;
            std::string format_spec;

            mnemonic << op_name;
            switch (size) {
                case op_sizes::byte:
                    mnemonic << ".B";
                    format_spec = "#${:02X}";
                    break;
                case op_sizes::word:
                    mnemonic << ".W";
                    format_spec = "#${:04X}";
                    break;
                case op_sizes::dword:
                    mnemonic << ".DW";
                    format_spec = "#${:08X}";
                    break;
                case op_sizes::qword:
                    mnemonic << ".QW";
                    format_spec = "#${:016X}";
                    break;
                default: {
                    break;
                }
            }

            stream << std::left << std::setw(10) << mnemonic.str();

            std::stringstream operands_stream;
            for (size_t i = 0; i < operands_count; i++) {
                if (i > 0 && i < operands_count) {
                    operands_stream << ", ";
                }

                const auto& operand = operands[i];
                std::string prefix, postfix;

                if (operand.is_negative()) {
                    if (operand.is_prefix())
                        prefix = "--";
                    else
                        prefix = "-";

                    if (operand.is_postfix())
                        postfix = "--";
                } else {
                    if (operand.is_prefix())
                        prefix = "++";

                    if (operand.is_postfix())
                        postfix = "++";
                }

                if (operand.is_reg()) {
                    if (operand.is_integer()) {
                        switch (operand.value.r8) {
                            case i_registers_t::sp: {
                                operands_stream << prefix << "SP" << postfix;
                                break;
                            }
                            case i_registers_t::fp: {
                                operands_stream << prefix << "FP" << postfix;
                                break;
                            }
                            case i_registers_t::pc: {
                                operands_stream << prefix << "PC" << postfix;
                                break;
                            }
                            case i_registers_t::fr: {
                                operands_stream << "FR";
                                break;
                            }
                            case i_registers_t::sr: {
                                operands_stream << "SR";
                                break;
                            }
                            default: {
                                operands_stream << prefix
                                                << "I"
                                                << std::to_string(operand.value.r8)
                                                << postfix;
                                break;
                            }
                        }
                    } else {
                        operands_stream << "F" << std::to_string(operand.value.r8);
                    }
                } else {
                    if (operand.is_integer()) {
                        if (operand.is_unresolved()) {
                            if (id_resolver == nullptr)
                                operands_stream << fmt::format("id({})", operand.value.u64);
                            else
                                operands_stream << id_resolver(operand.value.u64);
                        } else {
                            if (prefix == "-") {
                                operands_stream << fmt::format("{}", static_cast<int64_t>(operand.value.u64));
                            } else {
                                operands_stream << prefix
                                                << fmt::format(format_spec, operand.value.u64)
                                                << postfix;
                            }
                        }
                    } else {
                        operands_stream << prefix
                                        << fmt::format(format_spec, operand.value.d64)
                                        << postfix;
                    }
                }
            }

            stream << std::left << std::setw(24) << operands_stream.str();
        } else {
            stream << "UNKNOWN";
        }

        return stream.str();
    }

    ///////////////////////////////////////////////////////////////////////////

    instruction_cache::instruction_cache(terp* terp) : _terp(terp) {
    }

    void instruction_cache::reset() {
        _cache.clear();
    }

    size_t instruction_cache::fetch_at(
            common::result& r,
            uint64_t address,
            instruction_t& inst) {
        auto it = _cache.find(address);
        if (it == _cache.end()) {
            auto size = inst.decode(r, _terp->heap(), address);
            if (size == 0)
                return 0;
            _cache.insert(std::make_pair(
                address,
                icache_entry_t{.size = size, .inst = inst}));
            return size;
        } else {
            inst = it->second.inst;
            return it->second.size;
        }
    }

    size_t instruction_cache::fetch(common::result& r, instruction_t& inst) {
        return fetch_at(r, _terp->register_file().pc, inst);
    }

    ///////////////////////////////////////////////////////////////////////////

    terp::terp(size_t heap_size, size_t stack_size) : _heap_size(heap_size),
                                                      _stack_size(stack_size),
                                                      _icache(this) {
    }

    terp::~terp() {
        if (_call_vm != nullptr) {
            dcFree(_call_vm);
            _call_vm = nullptr;
        }

        free_heap_block_list();

        delete _heap;
        _heap = nullptr;
    }

    void terp::reset() {
        _registers.pc = heap_vector(heap_vectors_t::program_start);
        _registers.sp = heap_vector(heap_vectors_t::top_of_stack);
        _registers.fr = 0;
        _registers.sr = 0;

        for (size_t i = 0; i < 64; i++) {
            _registers.i[i] = 0;
            _registers.f[i] = 0.0;
        }

        _icache.reset();
        dcReset(_call_vm);
        free_heap_block_list();

        _exited = false;
    }

    uint64_t terp::pop() {
        uint64_t value = *qword_ptr(_registers.sp);
        _registers.sp += sizeof(uint64_t);
        return value;
    }

    uint64_t terp::peek() const {
        uint64_t value = *qword_ptr(_registers.sp);
        return value;
    }

    bool terp::register_foreign_function(
            common::result& r,
            function_signature_t& signature) {
        if (signature.func_ptr != nullptr) {
            auto it = _foreign_functions.find(signature.func_ptr);
            if (it != _foreign_functions.end())
                return true;
        }

        if (signature.library == nullptr) {
            // XXX: add an error message
            return false;
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

    void terp::dump_shared_libraries() {
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

    void terp::dump_state(uint8_t count) {
        fmt::print("\n-------------------------------------------------------------\n");
        fmt::print(
            "PC =${:08x} | SP =${:08x} | FR =${:08x} | SR =${:08x}\n",
            _registers.pc,
            _registers.sp,
            _registers.fr,
            _registers.sr);

        fmt::print("-------------------------------------------------------------\n");

        uint8_t index = 0;
        for (size_t y = 0; y < count; y++) {
            fmt::print(
                "I{:02}=${:08x} | I{:02}=${:08x} | I{:02}=${:08x} | I{:02}=${:08x}\n",
                index,
                _registers.i[index],
                index + 1,
                _registers.i[index + 1],
                index + 2,
                _registers.i[index + 2],
                index + 3,
                _registers.i[index + 3]);
            index += 4;
        }

        fmt::print("-------------------------------------------------------------\n");

        index = 0;
        for (size_t y = 0; y < count; y++) {
            fmt::print(
                "F{:02}=${:08x} | F{:02}=${:08x} | F{:02}=${:08x} | F{:02}=${:08x}\n",
                index,
                static_cast<uint64_t>(_registers.f[index]),
                index + 1,
                static_cast<uint64_t>(_registers.f[index + 1]),
                index + 2,
                static_cast<uint64_t>(_registers.f[index + 2]),
                index + 3,
                static_cast<uint64_t>(_registers.f[index + 3]));
            index += 4;
        }

        fmt::print("\n");
    }

    bool terp::step(common::result& r) {
        instruction_t inst;
        auto inst_size = _icache.fetch(r, inst);
        if (inst_size == 0)
            return false;

        _registers.pc += inst_size;

        switch (inst.op) {
            case op_codes::nop: {
                break;
            }
            case op_codes::alloc: {
                uint64_t size;
                if (!get_operand_value(r, inst, 1, size))
                    return false;

                size *= op_size_in_bytes(inst.size);
                uint64_t address = alloc(size);
                if (address == 0) {
                    execute_trap(trap_out_of_memory);
                    return false;
                }

                if (!set_target_operand_value(r, inst, 0, address))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, address == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(address, inst.size));

                break;
            }
            case op_codes::free: {
                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;

                auto freed_size = free(address);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::negative, false);
                _registers.flags(register_file_t::flags_t::zero, freed_size != 0);

                break;
            }
            case op_codes::size: {
                uint64_t address;
                if (!get_operand_value(r, inst, 1, address))
                    return false;

                uint64_t block_size = size(address);
                if (!set_target_operand_value(r, inst, 0, block_size))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, block_size == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(block_size, inst.size));

                break;
            }
            case op_codes::load: {
                uint64_t address;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                if (inst.operands_count > 2) {
                    uint64_t offset;
                    if (!get_operand_value(r, inst, 2, offset))
                        return false;
                    address += offset;
                }

                uint64_t value = *qword_ptr(address);
                if (!set_target_operand_value(r, inst, 0, value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));
                break;
            }
            case op_codes::store: {
                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;

                uint64_t value;
                if (!get_operand_value(r, inst, 1, value))
                    return false;

                if (inst.operands_count > 2) {
                    uint64_t offset;
                    if (!get_operand_value(r, inst, 2, offset))
                        return false;
                    address += offset;
                }

                *qword_ptr(address) = value;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));
                break;
            }
            case op_codes::copy: {
                uint64_t source_address, target_address;

                if (!get_operand_value(r, inst, 0, source_address))
                    return false;

                if (!get_operand_value(r, inst, 1, target_address))
                    return false;

                uint64_t length;
                if (!get_operand_value(r, inst, 2, length))
                    return false;

                memcpy(
                    _heap + target_address,
                    _heap + source_address,
                    length * op_size_in_bytes(inst.size));

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, false);

                break;
            }
            case op_codes::fill: {
                uint64_t value;
                if (!get_operand_value(r, inst, 0, value))
                    return false;

                uint64_t address;
                if (!get_operand_value(r, inst, 1, address))
                    return false;

                uint64_t length;
                if (!get_operand_value(r, inst, 2, length))
                    return false;
                length *= op_size_in_bytes(inst.size);

                switch (inst.size) {
                    case op_sizes::byte:
                        memset(_heap + address, static_cast<uint8_t>(value), length);
                        break;
                    case op_sizes::word:
                        memset(_heap + address, static_cast<uint16_t>(value), length);
                        break;
                    case op_sizes::dword:
                        memset(_heap + address, static_cast<uint32_t>(value), length);
                        break;
                    case op_sizes::qword:
                    default:
                        // XXX: this is an error
                        break;
                }

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, false);

                break;
            }
            case op_codes::move: {
                uint64_t source_value;

                if (!get_operand_value(r, inst, 1, source_value))
                    return false;

                if (!set_target_operand_value(r, inst, 0, source_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(source_value, inst.size));

                break;
            }
            case op_codes::push: {
                uint64_t source_value;

                if (!get_operand_value(r, inst, 0, source_value))
                    return false;

                push(source_value);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(source_value, inst.size));

                break;
            }
            case op_codes::pop: {
                uint64_t value = pop();

                if (!set_target_operand_value(r, inst, 0, value))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::dup: {
                uint64_t value = peek();
                push(value);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::inc: {
                uint8_t reg = inst.operands[0].value.r8;

                uint64_t lhs_value = _registers.i[reg];
                uint64_t rhs_value = 1;
                uint64_t value = lhs_value + rhs_value;
                if (set_target_operand_value(r, inst, reg, value))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, value, inst.size));
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::dec: {
                uint8_t reg = inst.operands[0].value.r8;

                uint64_t lhs_value = _registers.i[reg];
                uint64_t rhs_value = 1;
                uint64_t value = lhs_value - rhs_value;
                if (set_target_operand_value(r, inst, reg, value))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, value, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));
                break;
            }
            case op_codes::add: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t sum_result = lhs_value + rhs_value;
                if (!set_target_operand_value(r, inst, 0, sum_result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, sum_result, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, sum_result == 0);
                _registers.flags(register_file_t::flags_t::carry, has_carry(sum_result, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(sum_result, inst.size));

                break;
            }
            case op_codes::sub: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t subtraction_result = lhs_value - rhs_value;
                if (!set_target_operand_value(r, inst, 0, subtraction_result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, subtraction_result, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::carry, rhs_value > lhs_value);
                _registers.flags(register_file_t::flags_t::zero, subtraction_result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(subtraction_result, inst.size));
                break;
            }
            case op_codes::mul: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t product_result = lhs_value * rhs_value;
                if (!set_target_operand_value(r, inst, 0, product_result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, product_result, inst.size));
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, product_result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(product_result, inst.size));
                break;
            }
            case op_codes::div: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = 0;
                if (rhs_value != 0)
                    result = lhs_value / rhs_value;

                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, result, inst.size));
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::mod: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value % rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, result, inst.size));
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::neg: {
                uint64_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                int64_t negated_result = -static_cast<int64_t>(value);
                uint64_t result = static_cast<uint64_t>(negated_result);
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::shr: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value >> rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));
                break;
            }
            case op_codes::shl: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value << rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::ror: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t right_rotated_value = rotr(lhs_value, static_cast<uint8_t>(rhs_value));
                if (!set_target_operand_value(r, inst, 0, right_rotated_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, right_rotated_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(right_rotated_value, inst.size));

                break;
            }
            case op_codes::rol: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t left_rotated_value = rotl(lhs_value, static_cast<uint8_t>(rhs_value));
                if (!set_target_operand_value(r, inst, 0, left_rotated_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, left_rotated_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(left_rotated_value, inst.size));

                break;
            }
            case op_codes::and_op: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value & rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::or_op: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value | rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::xor_op: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value ^ rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::not_op: {
                uint64_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                uint64_t not_result = ~value;
                if (!set_target_operand_value(r, inst, 0, not_result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, not_result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(not_result, inst.size));

                break;
            }
            case op_codes::bis: {
                uint64_t value, bit_number;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;

                uint64_t masked_value = static_cast<uint64_t>(1 << bit_number);
                uint64_t result = value | masked_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::bic: {
                uint64_t value, bit_number;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;

                uint64_t masked_value = static_cast<uint64_t>(~(1 << bit_number));
                uint64_t result = value & masked_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, true);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::test: {
                uint64_t value, mask;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                uint64_t result = value & mask;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::cmp: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 0, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 1, rhs_value))
                    return false;

                uint64_t result = lhs_value - rhs_value;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, result, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::carry, has_carry(result, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::bz: {
                uint64_t value, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                if (value == 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::bnz: {
                uint64_t value, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                if (value != 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::tbz: {
                uint64_t value, mask, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                if (!get_operand_value(r, inst, 2, address))
                    return false;

                uint64_t result = value & mask;
                if (result == 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::tbnz: {
                uint64_t value, mask, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                if (!get_operand_value(r, inst, 2, address))
                    return false;

                uint64_t result = value & mask;
                if (result != 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::bne: {
                uint64_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (!_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }

                break;
            }
            case op_codes::beq: {
                uint64_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }

                break;
            }
            case op_codes::bg: {
                uint64_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (!_registers.flags(register_file_t::flags_t::carry)
                &&  !_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }

                break;
            }
            case op_codes::bge: {
                uint64_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (!_registers.flags(register_file_t::flags_t::carry)) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::bl: {
                uint64_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::carry)
                ||  _registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::ble: {
                uint64_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::carry)) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::setz: {
                uint64_t result = _registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;
                break;
            }
            case op_codes::setnz: {
                uint64_t result = !_registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;
                break;
            }
            case op_codes::jsr: {
                push(_registers.pc);

                uint64_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                _registers.pc = address;
                break;
            }
            case op_codes::rts: {
                uint64_t address = pop();
                _registers.pc = address;
                break;
            }
            case op_codes::jmp: {
                uint64_t address;

                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                _registers.pc = address;
                break;
            }
            case op_codes::swi: {
                uint64_t index;

                if (!get_operand_value(r, inst, 0, index))
                    return false;

                size_t swi_offset = sizeof(uint64_t) * index;
                uint64_t swi_address = *qword_ptr(swi_offset);
                if (swi_address != 0) {
                    // XXX: what state should we save and restore here?
                    push(_registers.pc);
                    _registers.pc = swi_address;
                }

                break;
            }
            case op_codes::swap: {
                uint64_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                uint64_t result = 0;
                switch (inst.size) {
                    case op_sizes::byte: {
                        uint8_t byte_value = static_cast<uint8_t>(value);
                        uint8_t upper_nybble = common::get_upper_nybble(byte_value);
                        uint8_t lower_nybble = common::get_lower_nybble(byte_value);
                        byte_value = common::set_upper_nybble(byte_value, lower_nybble);
                        result = common::set_lower_nybble(byte_value, upper_nybble);
                        break;
                    }
                    case op_sizes::word:
                        result = common::endian_swap_word(static_cast<uint16_t>(value));
                        break;
                    case op_sizes::dword:
                        result = common::endian_swap_dword(static_cast<uint32_t>(value));
                        break;
                    case op_sizes::qword:
                    default:
                        result = common::endian_swap_qword(value);
                        break;
                }

                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::trap: {
                uint64_t index;

                if (!get_operand_value(r, inst, 0, index))
                    return false;

                execute_trap(static_cast<uint8_t>(index));

                break;
            }
            case op_codes::ffi: {
                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;

                auto it = _foreign_functions.find(reinterpret_cast<void*>(address));
                if (it == _foreign_functions.end()) {
                    execute_trap(trap_invalid_ffi_call);
                    break;
                }

                auto func_signature = &it->second;
                func_signature->apply_calling_convention(_call_vm);
                dcReset(_call_vm);

                for (auto& argument : func_signature->arguments) {
                    argument.push(_call_vm, pop());
                }

                uint64_t result_value = func_signature->call(_call_vm, address);
                if (func_signature->return_value.type != ffi_types_t::void_type)
                    push(result_value);

                break;
            }
            case op_codes::meta: {
                uint64_t meta_data_size;

                if (!get_operand_value(r, inst, 0, meta_data_size))
                    return false;

                break;
            }
            case op_codes::exit: {
                _exited = true;
                break;
            }
        }

        return !r.is_failed();
    }

    bool terp::has_exited() const {
        return _exited;
    }

    size_t terp::heap_size() const {
        return _heap_size;
    }

    size_t terp::stack_size() const {
        return _stack_size;
    }

    void terp::push(uint64_t value) {
        _registers.sp -= sizeof(uint64_t);
        *qword_ptr(_registers.sp) = value;
        return;
    }

    void terp::free_heap_block_list() {
        _address_blocks.clear();

        if (_head_heap_block == nullptr)
            return;

        auto current_block = _head_heap_block;
        while (current_block != nullptr) {
            auto next_block = current_block->next;
            delete current_block;
            current_block = next_block;
        }

        _head_heap_block = nullptr;
    }

    uint64_t terp::alloc(uint64_t size) {
        uint64_t size_delta = size;
        heap_block_t* best_sized_block = nullptr;
        auto current_block = _head_heap_block;

        while (current_block != nullptr) {
            if (current_block->is_free()) {
                if (current_block->size == size) {
                    current_block->mark_allocated();
                    return current_block->address;
                } else if (current_block->size > size) {
                    auto local_size_delta = current_block->size - size;
                    if (best_sized_block == nullptr
                    ||  local_size_delta < size_delta) {
                        size_delta = local_size_delta;
                        best_sized_block = current_block;
                    }
                }
            }
            current_block = current_block->next;
        }

        if (best_sized_block != nullptr) {
            // if the block is over-sized by 64 bytes or less, just use it as-is
            if (size_delta <= 64) {
                best_sized_block->mark_allocated();
                return best_sized_block->address;
            } else {
                // otherwise, we need to split the block in two
                auto new_block = new heap_block_t;
                new_block->size = size;
                new_block->mark_allocated();
                new_block->prev = best_sized_block->prev;
                if (new_block->prev != nullptr)
                    new_block->prev->next = new_block;
                new_block->next = best_sized_block;
                new_block->address = best_sized_block->address;

                best_sized_block->prev = new_block;
                best_sized_block->address += size;
                best_sized_block->size -= size;

                if (new_block->prev == nullptr)
                    _head_heap_block = new_block;

                _address_blocks[new_block->address] = new_block;
                _address_blocks[best_sized_block->address] = best_sized_block;

                return best_sized_block->prev->address;
            }
        }

        return 0;
    }

    uint64_t terp::free(uint64_t address) {
        auto it = _address_blocks.find(address);
        if (it == _address_blocks.end())
            return 0;

        heap_block_t* freed_block = it->second;
        auto freed_size = freed_block->size;
        freed_block->clear_allocated();

        // coalesce free blocks
        // first, we walk down the prev chain until we find a non-free block
        // then, we walk down the next chain until we find a non-free block
        // because blocks are known to be adjacent to each other in the heap,
        //          we then coalesce these blocks into one

        std::vector<heap_block_t*> delete_list {};
        uint64_t new_size = 0;

        auto first_free_block = freed_block;
        while (true) {
            auto prev = first_free_block->prev;
            if (prev == nullptr || !prev->is_free())
                break;
            first_free_block = prev;
        }

        auto last_free_block = freed_block;
        while (true) {
            auto next = last_free_block->next;
            if (next == nullptr || !next->is_free())
                break;
            last_free_block = next;
        }

        auto current_node = first_free_block;
        while (true) {
            delete_list.emplace_back(current_node);
            new_size += current_node->size;

            if (current_node == last_free_block)
                break;

            current_node = current_node->next;
        }

        if (first_free_block != last_free_block) {
            auto new_block = new heap_block_t;
            new_block->size = new_size;

            new_block->next = last_free_block->next;
            if (new_block->next != nullptr)
                new_block->next->prev = new_block;

            new_block->prev = first_free_block->prev;
            if (new_block->prev != nullptr)
                new_block->prev->next = new_block;

            new_block->address = first_free_block->address;

            for (auto block : delete_list) {
                _address_blocks.erase(block->address);
                delete block;
            }

            if (new_block->prev == nullptr)
                _head_heap_block = new_block;

            _address_blocks[new_block->address] = new_block;
        }

        return freed_size;
    }

    uint64_t terp::size(uint64_t address) {
        auto it = _address_blocks.find(address);
        if (it == _address_blocks.end())
            return 0;
        return it->second->size;
    }

    bool terp::initialize(common::result& r) {
        if (_heap != nullptr)
            return true;

        _call_vm = dcNewCallVM(4096);

        _shared_libraries.clear();
//        shared_library_t self_image;
//        if (!self_image.initialize(r))
//            return false;
//        _shared_libraries.insert(std::make_pair(self_image.path(), self_image));

        _heap = new uint8_t[_heap_size];

        heap_vector(heap_vectors_t::top_of_stack, _heap_size);
        heap_vector(heap_vectors_t::bottom_of_stack, _heap_size - _stack_size);
        heap_vector(heap_vectors_t::program_start, program_start);

        reset();

        return !r.is_failed();
    }

    void terp::remove_trap(uint8_t index) {
        _traps.erase(index);
    }

    void terp::execute_trap(uint8_t index) {
        auto it = _traps.find(index);
        if (it == _traps.end())
            return;
        it->second(this);
    }

    shared_library_t* terp::load_shared_library(
            common::result& r,
            const std::filesystem::path& path) {
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

    std::vector<uint64_t> terp::jump_to_subroutine(
            common::result& r,
            uint64_t address) {
        std::vector<uint64_t> return_values;

        auto return_address = _registers.pc;
        push(return_address);
        _registers.pc = address;

        while (!has_exited()) {
            // XXX: need to introduce a terp_step_result_t
            auto result = step(r);
            // XXX: did an RTS just execute?
            //      does _registers.pc == return_address?  if so, we're done
            if (!result) {
                break;
            }
        }

        // XXX: how do we handle multiple return values?
        // XXX: pull return values from the stack
        return return_values;
    }

    void terp::swi(uint8_t index, uint64_t address) {
        size_t swi_address = interrupt_vector_table_start + (sizeof(uint64_t) * index);
        *qword_ptr(swi_address) = address;
    }

    const register_file_t& terp::register_file() const {
        return _registers;
    }

    void terp::dump_heap(uint64_t offset, size_t size) {
        auto program_memory = common::hex_formatter::dump_to_string(
            reinterpret_cast<const void*>(_heap + offset),
            size);
        fmt::print("{}\n", program_memory);
    }

    void terp::heap_free_space_begin(uint64_t address) {
        heap_vector(heap_vectors_t::free_space_start, address);
        _head_heap_block = new heap_block_t;
        _head_heap_block->address = address;
        _head_heap_block->size = heap_vector(heap_vectors_t::bottom_of_stack) - address;
        _address_blocks.insert(std::make_pair(_head_heap_block->address, _head_heap_block));
    }

    uint64_t terp::heap_vector(heap_vectors_t vector) const {
        size_t heap_vector_address = heap_vector_table_start
            + (sizeof(uint64_t) * static_cast<uint8_t>(vector));
        return *qword_ptr(heap_vector_address);
    }

    const meta_information_t& terp::meta_information() const {
        return _meta_information;
    }

    void terp::heap_vector(heap_vectors_t vector, uint64_t address) {
        size_t heap_vector_address = heap_vector_table_start
            + (sizeof(uint64_t) * static_cast<uint8_t>(vector));
        *qword_ptr(heap_vector_address) = address;
    }

    std::string terp::disassemble(common::result& r, uint64_t address) {
        std::stringstream stream;
        while (true) {
            instruction_t inst;
            auto inst_size = _icache.fetch_at(r, address, inst);
            if (inst_size == 0)
                break;

            stream << fmt::format("${:016X}: ", address)
                   << inst.disassemble()
                   << fmt::format(" (${:02X} bytes)\n", inst_size);

            if (inst.op == op_codes::exit)
                break;

            address += inst_size;
        }
        return stream.str();
    }

    bool terp::get_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            double& value) const {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                value = _registers.i[operand.value.r8];
            } else {
                value = _registers.f[operand.value.r8];
            }
        } else {
            if (operand.is_integer()) {
                value = operand.value.u64;
            } else {
                value = operand.value.d64;
            }
        }

        return true;
    }

    bool terp::get_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t& value) const {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                auto reg = static_cast<i_registers_t>(operand.value.r8);
                switch (reg) {
                    case i_registers_t::pc: {
                        value = _registers.pc;
                        break;
                    }
                    case i_registers_t::sp: {
                        value = _registers.sp;
                        break;
                    }
                    case i_registers_t::fp: {
                        value = _registers.fp;
                        break;
                    }
                    case i_registers_t::fr: {
                        value = _registers.fr;
                        break;
                    }
                    case i_registers_t::sr: {
                        value = _registers.sr;
                        break;
                    }
                    default: {
                        value = _registers.i[reg];
                        break;
                    }
                }
            } else {
                value = static_cast<uint64_t>(_registers.f[operand.value.r8]);
            }
        } else {
            if (operand.is_integer()) {
                value = operand.value.u64;
            } else {
                value = static_cast<uint64_t>(operand.value.d64);
            }
        }

        return true;
    }

    bool terp::set_target_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t value) {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                auto reg = static_cast<i_registers_t>(operand.value.r8);
                switch (reg) {
                    case i_registers_t::pc: {
                        _registers.pc = set_zoned_value(_registers.pc, value, inst.size);
                        break;
                    }
                    case i_registers_t::sp: {
                        _registers.sp = set_zoned_value(_registers.sp, value, inst.size);
                        break;
                    }
                    case i_registers_t::fp: {
                        _registers.fp = set_zoned_value(_registers.fp, value, inst.size);
                        break;
                    }
                    case i_registers_t::fr: {
                        _registers.fr = set_zoned_value(_registers.fr, value, inst.size);
                        break;
                    }
                    case i_registers_t::sr: {
                        _registers.sr = set_zoned_value(_registers.sr, value, inst.size);
                        break;
                    }
                    default: {
                        _registers.i[reg] = set_zoned_value(_registers.i[reg], value, inst.size);
                        break;
                    }
                }
            } else {
                _registers.f[operand.value.r8] = value;
            }

        } else {
            r.add_message(
                "B006",
                "constant cannot be a target operand type.",
                true);
            return false;
        }

        return true;
    }

    bool terp::set_target_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            double value) {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                auto integer_value = static_cast<uint64_t>(value);
                auto reg = static_cast<i_registers_t>(operand.value.r8);
                switch (reg) {
                    case i_registers_t::pc: {
                        _registers.pc = set_zoned_value(_registers.pc, integer_value, inst.size);
                        break;
                    }
                    case i_registers_t::sp: {
                        _registers.sp = set_zoned_value(_registers.sp, integer_value, inst.size);
                        break;
                    }
                    case i_registers_t::fp: {
                        _registers.fp = set_zoned_value(_registers.fp, integer_value, inst.size);
                        break;
                    }
                    case i_registers_t::fr: {
                        _registers.fr = set_zoned_value(_registers.fr, integer_value, inst.size);
                        break;
                    }
                    case i_registers_t::sr: {
                        _registers.sr = set_zoned_value(_registers.sr, integer_value, inst.size);
                        break;
                    }
                    default: {
                        _registers.i[reg] = set_zoned_value(_registers.i[reg], integer_value, inst.size);;
                        break;
                    }
                }
            } else {
                _registers.f[operand.value.r8] = value;
            }
        } else {
            r.add_message(
                "B006",
                "constant cannot be a target operand type.",
                true);
            return false;
        }

        return true;
    }

    bool terp::get_constant_address_or_pc_with_offset(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t inst_size,
            uint64_t& address) {
        if (!get_operand_value(r, inst, operand_index, address))
            return false;

        if (inst.operands_count >= 2) {
            uint64_t offset;

            uint8_t offset_index = static_cast<uint8_t>(operand_index + 1);
            if (!get_operand_value(r, inst, offset_index, offset))
                return false;

            if (inst.operands[offset_index].is_negative()) {
                address -= offset + inst_size;
            } else {
                address += offset - inst_size;
            }
        }

        return true;
    }

    bool terp::has_carry(uint64_t value, op_sizes size) {
        switch (size) {
            case op_sizes::byte:
                return value > UINT8_MAX;
            case op_sizes::word:
                return value > UINT16_MAX;
            case op_sizes::dword:
                return value > UINT32_MAX;
            case op_sizes::qword:
            default:
                return value > UINT64_MAX;
        }
    }

    bool terp::is_negative(uint64_t value, op_sizes size) {
        switch (size) {
            case op_sizes::byte: {
                return (value & mask_byte_negative) != 0;
            }
            case op_sizes::word: {
                return (value & mask_word_negative) != 0;
            }
            case op_sizes::dword: {
                return (value & mask_dword_negative) != 0;
            }
            case op_sizes::qword:
            default:
                return (value & mask_qword_negative) != 0;
        }
    }

    shared_library_t* terp::shared_library(const std::filesystem::path& path) {
        auto it = _shared_libraries.find(path.string());
        if (it == _shared_libraries.end())
            return nullptr;
        return &it->second;
    }

    void terp::register_trap(uint8_t index, const terp::trap_callable& callable) {
        _traps.insert(std::make_pair(index, callable));
    }

    uint64_t terp::set_zoned_value(uint64_t source, uint64_t value, op_sizes size) {
        uint64_t result = source;
        switch (size) {
            case op_sizes::byte: {
                result &= mask_byte_clear;
                result |= (value & mask_byte);
                break;
            }
            case op_sizes::word: {
                result &= mask_word_clear;
                result |= (value & mask_word);
                break;
            }
            case op_sizes::dword: {
                result &= mask_dword_clear;
                result |= (value & mask_dword);
                break;
            }
            case op_sizes::qword:
            default:
                result = value;
                break;
        }
        return result;
    }

    bool terp::has_overflow(uint64_t lhs, uint64_t rhs, uint64_t result, op_sizes size) {
        switch (size) {
            case op_sizes::byte:
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_byte_negative) != 0;
            case op_sizes::word:
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_word_negative) != 0;
            case op_sizes::dword:
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_dword_negative) != 0;
            case op_sizes::qword:
            default: {
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_qword_negative) != 0;
            }
        }
    }

};