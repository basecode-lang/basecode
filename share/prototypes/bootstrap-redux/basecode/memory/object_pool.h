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
#include <type_traits>
#include <boost/any.hpp>
#include <basecode/adt/hash_table.h>
#include "slab_allocator.h"

namespace basecode::memory {

    class object_pool_t final {
        template<typename...>
        class family_t {
            inline static uint32_t identifier;

            template<typename...>
            inline static const uint32_t inner = identifier++;

        public:
            using family_type = uint32_t;

            template<typename... Type>
            inline static const family_type type = inner<std::decay_t<Type>...>;
        };

        struct destroyer_t final {
            const void* p;
            void (*destroy)(const void*);
        };

    public:
        explicit object_pool_t(
            allocator_t* backing = context::current()->allocator) : _backing(backing),
                                                                    _objs(backing),
                                                                    _pools(backing) {
            assert(_backing);
        }

        ~object_pool_t() {
            if (!_released) release();
        }

        void release() {
            const auto& objs = _objs.values();
            for (auto obj : objs)
                obj->destroy(obj->p);

            const auto& slabs = _pools.values();
            for (auto slab : slabs) {
                slab->~slab_allocator_t();
                _backing->deallocate(slab);
            }

            _released = true;
        }

        template <typename T>
        void destroy(T* obj) {
            auto type_id = family_t<>::template type<T>;
            auto slab = _pools.find(type_id);
            assert(slab);

            auto dtor = _objs.find(obj);
            dtor->destroy(dtor->p);

            _objs.remove(obj);

            slab->deallocate(obj);
        }

        template <typename T, typename... Args>
        T* construct(Args&&... args) {
            auto type_id = family_t<>::template type<T>;
            auto slab = _pools.find(type_id);
            if (slab == nullptr) {
                auto mem = _backing->allocate(
                    sizeof(slab_allocator_t),
                    alignof(slab_allocator_t));
                auto type_name = typeid(T).name();
                slab = new (mem) slab_allocator_t(
                    adt::string_t(type_name, strlen(type_name)),
                    sizeof(T),
                    alignof(T),
                    _backing);
                _pools.insert(type_id, slab);
            }
            auto mem = slab->allocate();
            _objs.insert(
                mem,
                destroyer_t{
                    .p = mem,
                    .destroy =[](const void* x) {
                        static_cast<const T*>(x)->~T();
                    }
                });
            return new (mem) T(std::forward<Args>(args)...);
        }

    private:
        bool _released{};
        allocator_t* _backing;
        adt::hash_table_t<void*, destroyer_t> _objs;
        adt::hash_table_t<uint32_t, slab_allocator_t*> _pools;
    };

}