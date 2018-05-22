#pragma once

#include <memory.h>
#include "symbol_table.h"

namespace basecode::compiler {

    class scope;

    using scope_list = std::vector<std::unique_ptr<scope>>;

    class scope {
    public:
        scope(
            compiler::scope* parent,
            const syntax::ast_node_shared_ptr& node);

        void clear();

        uint64_t address() const;

        compiler::scope* parent();

        void address(uint64_t value);

        const scope_list& children() const;

        syntax::ast_node_shared_ptr ast_node();

        compiler::scope* add_child_scope(const syntax::ast_node_shared_ptr& node);

    private:
        uint64_t _address;
        scope_list _children {};
        compiler::scope* _parent = nullptr;
        compiler::symbol_table _symbol_table {};
        syntax::ast_node_shared_ptr _node = nullptr;
    };

};

