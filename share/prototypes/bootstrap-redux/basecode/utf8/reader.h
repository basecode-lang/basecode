// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <stack>
#include <string_view>
#include <basecode/result.h>
#include <basecode/adt/stack.h>
#include "rune.h"

namespace basecode::utf8 {

    class reader_t final {
    public:
        reader_t(
            memory::allocator_t* allocator,
            std::string_view slice);

        void push_mark();

        size_t pop_mark();

        size_t current_mark();

        bool seek(size_t index);

        void restore_top_mark();

        rune_t curr(result_t& r);

        rune_t next(result_t& r);

        rune_t prev(result_t& r);

        bool move_prev(result_t& r);

        bool move_next(result_t& r);

        [[nodiscard]] bool eof() const;

        [[nodiscard]] size_t pos() const;

        [[nodiscard]] uint32_t width() const;

        [[nodiscard]] std::string_view make_slice(
            size_t offset,
            size_t length) const;

        [[nodiscard]] std::string_view slice() const;

    private:
        rune_t read(result_t& r, uint32_t& width) const;

    private:
        size_t _index{};
        std::string_view _slice;
        memory::allocator_t* _allocator;
        adt::stack_t<size_t> _mark_stack;
        adt::stack_t<uint32_t> _width_stack;
    };

}