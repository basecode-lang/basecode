cmake_minimum_required(VERSION 3.15)
project(basecode)

message(STATUS "--------------- ${PROJECT_NAME} build configuration ---------------")

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (PRODUCT_NAME  "Basecode Compiler Library")

configure_file (
        "${PROJECT_SOURCE_DIR}/configure.in.h"
        "${PROJECT_BINARY_DIR}/configure.h"
)

add_library(
        ${PROJECT_NAME}

        ../ext/tracy-master/TracyClientDLL.cpp
        ../ext/mustach-master/mustach.c ../ext/mustach-master/mustach.h

        context/context.cpp context/context.h
        
        defer.h
        result.h result_message.h

        adt/set.h
        adt/array.h
        adt/stack.h
        adt/queue.h
        adt/trie_map.h
        adt/hash_table.h
        adt/binary_tree.h
        adt/red_black_tree.h
        adt/directed_graph.h
        adt/string.cpp adt/string.h
        adt/hashable.cpp adt/hashable.h

        types.cpp types.h

        memory/allocator.h
        memory/object_pool.h
        memory/std_allocator.h
        memory/temp_allocator.h
        memory/frame_allocator.h
        memory/system.cpp memory/system.h
        memory/dlmalloc.c memory/dlmalloc.h
        memory/dl_allocator.cpp memory/dl_allocator.h
        memory/slab_allocator.cpp memory/slab_allocator.h
        memory/stack_allocator.cpp memory/stack_allocator.h
        memory/proxy_allocator.cpp memory/proxy_allocator.h
        memory/trace_allocator.cpp memory/trace_allocator.h
        memory/system_allocator.cpp memory/system_allocator.h
        memory/scratch_allocator.cpp memory/scratch_allocator.h

        id/pool.cpp id/pool.h

        io/text.cpp io/text.h
        io/binary.cpp io/binary.h

        format/formatters.h
        format/format.cpp format/format.h

        graphviz/model.h
        graphviz/node.cpp graphviz/node.h
        graphviz/edge.cpp graphviz/edge.h
        graphviz/graph.cpp graphviz/graph.h
        graphviz/dot_model.cpp graphviz/dot_model.h
        graphviz/attribute.cpp graphviz/attribute.h
        graphviz/attribute_container.cpp graphviz/attribute_container.h

        errors/errors.cpp errors/errors.h

        signals/hook.cpp signals/hook.h

        numbers/bytes.h
        numbers/parse.cpp numbers/parse.h

        profiler/timer.cpp profiler/timer.h

        hashing/murmur.cpp hashing/murmur.h

        utf8/rune.cpp utf8/rune.h
        utf8/reader.cpp utf8/reader.h
        utf8/source_buffer.cpp utf8/source_buffer.h

        strings/pool.cpp strings/pool.h
        strings/transforms.cpp strings/transforms.h
        strings/escape_parser.cpp strings/escape_parser.h

        terminal/stream_factory.cpp terminal/stream_factory.h

        language/common.cpp language/common.h

        language/assembly/lexer/token.cpp language/assembly/lexer/token.h
        language/assembly/lexer/lexer.cpp language/assembly/lexer/lexer.h

        language/core/ast/ast.cpp language/core/ast/ast.h
        language/core/lexer/token.cpp language/core/lexer/token.h
        language/core/lexer/lexer.cpp language/core/lexer/lexer.h
        language/core/parser/parser.cpp language/core/parser/parser.h

        logging/logger.cpp logging/logger.h
        logging/spd_logger.cpp logging/spd_logger.h
        logging/fake_logger.cpp logging/fake_logger.h

        threading/auto_reset_event.cpp threading/auto_reset_event.h
        threading/manual_reset_event.cpp threading/manual_reset_event.h

        workspace/session.cpp workspace/session.h
)
target_link_libraries(
        ${PROJECT_NAME}
        utf8proc
        fmt-header-only
        boost-system
        boost-filesystem
        tcc
)

add_subdirectory(adt/tests)
add_subdirectory(memory/tests)
add_subdirectory(graphviz/tests)
add_subdirectory(profiler/tests)
add_subdirectory(language/core/lexer/tests)
add_subdirectory(language/core/parser/tests)