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

#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
#include <boost/stacktrace.hpp>
#include "trace_allocator.h"

using namespace std::literals;

namespace basecode::memory {

    trace_allocator_t::trace_allocator_t(
        allocator_t* backing,
        size_t debug_heap_size) : _debug_heap(create_mspace(debug_heap_size, 0)),
                                  _backing(backing),
                                  _debug_allocator(_debug_heap) {
        format::allocator_t alloc(&_debug_allocator);
        _buffer = memory::construct_with_allocator<format::memory_buffer_t>(&_debug_allocator, alloc);
        _stream_factory.enabled(true);
        _stream = _stream_factory.use_memory_buffer(*_buffer);
    }

    trace_allocator_t::~trace_allocator_t() {
        {
            auto debug_context = *context::current();
            debug_context.allocator = &_debug_allocator;
            context::push(&debug_context);

            format::print("{}\n", _stream->format());
        }
        memory::destroy(&_debug_allocator, _buffer);
        destroy_mspace(_debug_heap);
    }

    void* trace_allocator_t::allocate(
            uint32_t size,
            uint32_t align) {
        boost::stacktrace::stacktrace trace(2, 2);

        {
            auto debug_context = *context::current();
            debug_context.allocator = &_debug_allocator;
            context::push(&debug_context);

            _stream->color(
                terminal::colors_t::default_color,
                terminal::colors_t::yellow);
            for (int32_t i = trace.size() - 1; i >= 0; --i) {
                if (i != trace.size() - 1) _stream->append(" -> "sv);
                const auto& frame = trace[i];
                if (frame.empty()) continue;
                _stream->append(frame.name());
            }
            _stream->append("\n"sv)->color_reset();
        }

        auto p = _backing->allocate(size, align);
        return p;
    }

    void trace_allocator_t::deallocate(void* p) {
        boost::stacktrace::stacktrace trace(2, 2);

        {
            auto debug_context = *context::current();
            debug_context.allocator = &_debug_allocator;
            context::push(&debug_context);

            _stream->color(
                terminal::colors_t::default_color,
                terminal::colors_t::green);
            for (int32_t i = trace.size() - 1; i >= 0; --i) {
                if (i != trace.size() - 1) _stream->append(" -> "sv);
                const auto& frame = trace[i];
                if (frame.empty()) continue;
                _stream->append(frame.name());
            }
            _stream->append("\n"sv)->color_reset();
        }

        _backing->deallocate(p);
    }

    std::optional<uint32_t> trace_allocator_t::total_allocated() {
        return _backing->total_allocated();
    }

    std::optional<uint32_t> trace_allocator_t::allocated_size(void* p) {
        return _backing->allocated_size(p);
    }

}