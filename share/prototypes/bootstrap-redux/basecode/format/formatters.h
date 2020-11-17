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

#include <fmt/format.h>
#include <basecode/types.h>
#include <basecode/result.h>
#include <basecode/utf8/rune.h>
#include <basecode/adt/string.h>
#include <basecode/language/common.h>
#include <basecode/language/core/lexer/token.h>
#include <basecode/language/assembly/lexer/token.h>

namespace fmt {

    using namespace basecode;

    template<>
    struct formatter<result_t> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(
                const result_t& r,
                FormatContext& ctx) {
            auto has_messages = !r.messages().empty();

            if (has_messages)
                format_to(ctx.out(), "\n");

            auto messages = r.messages();
            for (size_t i = 0; i < messages.size(); i++) {
                const auto& msg = messages[i];
                format_to(
                        ctx.out(),
                        "[{}] {}{}\n",
                        msg.code(),
                        msg.is_error() ? "ERROR: " : "WARNING: ",
                        msg.message());
                if (!msg.details().empty()) {
                    format_to(ctx.out(), "{}\n", msg.details());
                }
                if (i < messages.size() - 1)
                    format_to(ctx.out(), "\n");
            }

            return format_to(ctx.out(), "");
        }
    };

    template<>
    struct formatter<utf8::rune_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(
                const utf8::rune_t& rune,
                FormatContext& ctx) {
            std::string temp{};
            auto encode_result = utf8::encode(rune);
            for (size_t j = 0; j < encode_result.width; j++)
                temp += static_cast<char>(encode_result.data[j]);
            return format_to(ctx.out(), "{}", temp);
        }
    };

    template<>
    struct formatter<adt::string_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(
                const adt::string_t& str,
                FormatContext& ctx) {
            return format_to(ctx.out(), "{}", str.slice());
        }
    };

    template<>
    struct formatter<entt::entity> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(
                const entt::entity& entity,
                FormatContext& ctx) {
            return format_to(ctx.out(), "entity_{}", (uint32_t) entity);
        }
    };

    template<>
    struct formatter<source_location_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(
                const source_location_t& loc,
                FormatContext& ctx) {
            return format_to(
                ctx.out(),
                "{}@{}-{}@{}",
                loc.start.line + 1,
                loc.start.column + 1,
                loc.end.line + 1,
                loc.end.column);
        }
    };

    template<>
    struct formatter<language::core::lexer::token_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(
                const language::core::lexer::token_t& token,
                FormatContext& ctx) {
            format_to(
                ctx.out(),
                "<type = {}",
                language::core::lexer::token_type_to_name(token.type));
            if (!token.value.empty()) {
                format_to(ctx.out(), ", value = {}", token.value);
            }
            return format_to(ctx.out(), ">");
        }
    };

    template<>
    struct formatter<language::assembly::lexer::token_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(
            const language::assembly::lexer::token_t& token,
            FormatContext& ctx) {
            format_to(
                ctx.out(),
                "<type = {}",
                language::assembly::lexer::token_type_to_name(token.type));
            if (!token.value.empty()) {
                format_to(ctx.out(), ", value = {}", token.value);
            }
            return format_to(ctx.out(), ">");
        }
    };

    template<>
    struct formatter<language::number_token_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(
                const language::number_token_t& token,
                FormatContext& ctx) {
            format_to(
                ctx.out(),
                "<is_signed = {}, radix = {}, type = {}",
                token.is_signed,
                token.radix,
                language::number_type_to_name(token.type));
            if (token.type == language::number_type_t::integer) {
                switch (token.size) {
                    case language::number_size_t::byte:
                        return format_to(ctx.out(), ", value = u8({})>", token.value.u8);
                    case language::number_size_t::word:
                        return format_to(ctx.out(), ", value = u16({})>", token.value.u16);
                    case language::number_size_t::dword:
                        return format_to(ctx.out(), ", value = u32({})>", token.value.u32);
                    case language::number_size_t::qword:
                        return format_to(ctx.out(), ", value = u64({})>", token.value.u64);
                }
            } else if (token.type == language::number_type_t::floating_point) {
                switch (token.size) {
                    case language::number_size_t::dword:
                        return format_to(ctx.out(), ", value = f32({})>", token.value.f32);
                    case language::number_size_t::qword:
                        return format_to(ctx.out(), ", value = f64({})>", token.value.f64);
                    default:
                        return format_to(ctx.out(), ", value = invalid({})>", token.value.f64);
                }
            } else {
                // XXX: revisit to potentially support other number types, e.g. arbitrary precision numbers
                return format_to(ctx.out(), ">");
            }
        }
    };

}
