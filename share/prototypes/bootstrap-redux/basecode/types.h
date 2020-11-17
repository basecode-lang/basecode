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

#include <cstdint>
#include <entt/entt.hpp>
#include <boost/uuid/uuid.hpp>
#include <basecode/adt/array.h>
#include <basecode/adt/string.h>
#include <basecode/adt/hash_table.h>
#include <boost/filesystem/path.hpp>

#if defined(__GNUC__)
#  define factor_force_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#  define factor_force_inline __forceinline
#else
#  define factor_force_inline inline
#endif

#if defined(__GNUC__)
#  define factor_no_inline __attribute__((noinline))
#elif defined(_MSC_VER)
#  define factor_no_inline __declspec(noinline)
#else
#  define factor_no_inline
#endif

namespace basecode {

    using namespace std::literals;

    ///////////////////////////////////////////////////////////////////////////

    // filesystem type aliases
    using path_t = boost::filesystem::path;
    using path_list_t = adt::array_t<path_t>;

    // string type aliases
    using string_t = adt::string_t;
    using slice_list_t = adt::array_t<std::string_view>;
    using string_list_t = adt::array_t<adt::string_t>;
    using string_map_t = adt::hash_table_t<string_t, string_t>;

    // time type aliases
    using time_point_t = std::chrono::system_clock::time_point;
    using monotonic_time_point_t = std::chrono::steady_clock::time_point;

    // bytes type aliases
    using slice_t = std::string_view;
    using byte_buffer_t = adt::array_t<uint8_t>;

    // uuid type aliases
    using uuid_t = boost::uuids::uuid;

    // entity aliases
    using entity_t = entt::entity;
    using entity_list_t = adt::array_t<entt::entity>;
    [[maybe_unused]] constexpr entity_t null_entity = entt::null;

}