#include "terp.h"

namespace basecode {

    terp::terp(uint32_t heap_size) : _heap_size(heap_size) {
    }

    terp::~terp() {
        delete _heap;
        _heap = nullptr;
    }

    size_t terp::heap_size() const {
        return _heap_size;
    }

    bool terp::initialize(result& r) {
        _heap = new uint64_t[heap_size_in_qwords()];

        _registers.pc = 0;
        _registers.fr = 0;
        _registers.sr = 0;
        _registers.sp = heap_size_in_qwords();

        for (size_t i = 0; i < 64; i++) {
            _registers.i[i] = 0;
            _registers.f[i] = 0.0;
        }

        return !r.is_failed();
    }

    uint64_t terp::pop() {
        uint64_t value = _heap[_registers.sp];
        _registers.sp += sizeof(uint64_t);
        return value;
    }

    void terp::push(uint64_t value) {
        _registers.sp -= sizeof(uint64_t);
        _heap[_registers.sp] = value;
        return;
    }

    const register_file_t& terp::register_file() const {
        return _registers;
    }

    size_t terp::heap_size_in_qwords() const {
        return _heap_size / sizeof(uint64_t);
    }

};