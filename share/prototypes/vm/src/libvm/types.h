// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/types.h>

namespace basecode::vm {

    enum class register_t : u8 {
        pc,
        fp,
        sp,
        r0,
        r1,
        r2,
        r3,
        r4,
        r5,
        r6,
        r7,
        r8,
        r9,
        r10,
        r11,
        r12,
        r13,
        r14,
        r15,
        r16,
        r17,
        r18,
        r19,
        r20,
        r21,
        r22,
        r23,
        r24,
        r25,
        r26,
        r27,
        r28,
        r29,
        r30,
        r31,
    };

    static constexpr u32 register_file_size = 32 + 3;

    struct state_t final {
        u64 r[register_file_size];
    };

    // Operation size parameter encoding works like this:
    //
    // 00000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000
    //
    // .b      ==                                                     |      |
    //                                                                +------+
    // .w      ==                                            |               |
    //                                                       +---------------+
    // .dw     ==                          |                                 |
    //                                     +---------------------------------+
    // .qw|empty
    // |                                                                     |
    // +---------------------------------------------------------------------+
    //

    enum class operation_code_t : u16 {
        // NOP
        nop,

        // LOAD[.size][:flag reg] {dest reg}, {base address reg}[, {reg | imm 24-bit relative}]
        loadb,
        loadw,
        loaddw,
        loadqw,

        // STORE[.size] {base address reg}, {source reg}[, {reg | imm 24-bit relative}]
        storeb,
        storew,
        storedw,
        storeqw,

        // MOVE[.size][:flag reg] {dest}, {reg | imm }
        moveb,
        movew,
        movedw,
        moveqw,             // XXX: requires "continuation" flag for lower 32-bit OR mask
                            //      encoded as an additional 8-byte "instruction" immediately after

        // MOVEZ[.size][:flag reg] {dest}, {source | imm }
        movezb,
        movezw,
        movezdw,
        movezqw,

        // MOVES[.size][:flag reg] {dest}, {source | imm }
        movesb,
        movesw,
        movesdw,
        movesqw,

        // ADDI[.size][:flag reg] {dest}, {addend reg}, {augend reg}
        addib,
        addiw,
        addidw,
        addiqw,

        // ADDIS[.size][:flag reg] {dest}, {addend reg}, {augend reg}
        addisb,
        addisw,
        addisdw,
        addisqw,

        // ADDF[.size][:flag reg] {dest}, {addend reg}, {augend reg}
        // NOTE: size limited to .dw & .qw
        addfdw,
        addfqw,

        // SUBI[.size][:flag reg] {dest}, {subtrahend reg}, {sub-something reg}
        subib,
        subiw,
        subidw,
        subiqw,

        // SUBIS[.size][:flag reg] {dest}, {subtrahend reg}, {sub-something reg}
        subisb,
        subisw,
        subisdw,
        subisqw,

        // SUBF[.size][:flag reg] {dest}, {subtrahend reg}, {sub-something reg}
        // NOTE: size limited to .dw & .qw
        subfdw,
        subfqw,

        // MULI[.size][:flag reg] {dest}, {multiplicand reg}, {multiplier reg}
        mulib,
        muliw,
        mulidw,
        muliqw,

        // MULIS[.size][:flag reg] {dest}, {multiplicand reg}, {multiplier reg}
        mulisb,
        mulisw,
        mulisdw,
        mulisqw,

        // MULF[.size][:flag reg] {dest}, {multiplicand reg}, {multiplier reg}
        // NOTE: size limited to .dw & .qw
        mulfdw,
        mulfqw,

        // DIVI[.size][:flag reg] {dest}, {dividend reg}, {divisor reg}
        divib,
        diviw,
        dividw,
        diviqw,

        // DIVIS[.size][:flag reg] {dest}, {dividend reg}, {divisor reg}
        divisb,
        divisw,
        divisdw,
        divisqw,

        // DIVF[.size][:flag reg] {dest}, {dividend reg}, {divisor reg}
        // NOTE: size limited to .dw & .qw
        divfdw,
        divfqw,

        // MODI[.size][:flag reg] {dest}, {lhs}, {rhs}
        modib,
        modiw,
        modidw,
        modiqw,

        // MODIS[.size][:flag reg] {dest}, {lhs}, {rhs}
        modisb,
        modisw,
        modisdw,
        modisqw,

        // MADDI[.size][:flag reg] {dest}, {multiplicand reg}, {multiplier reg}, {addend reg}
        maddib,
        maddiw,
        maddidw,
        maddiqw,

        // MADDIS[.size][:flag reg] {dest}, {multiplicand reg}, {multiplier reg}, {addend reg}
        maddisb,
        maddisw,
        maddisdw,
        maddisqw,

        // MADDF[.size][:flag reg] {dest}, {multiplicand reg}, {multiplier reg}, {addend reg}
        // NOTE: size limited to .dw & .qw
        maddfdw,
        maddfqw,

        // NEGIS[.size][:flag reg] {dest}, {source}
        negisb,
        negisw,
        negisdw,
        negisqw,

        // NEGF[.size][:flag reg] {dest}, {source}
        // NOTE: size limited to .dw & .qw
        negfdw,
        negfqw,

        // SHR[.size][:flag reg] {dest}, {source}, {count}
        shrb,
        shrw,
        shrdw,
        shrqw,

        // SHL[.size][:flag reg] {dest}, {source}, {count}
        shlb,
        shlw,
        shldw,
        shlqw,

        // ROR[.size][:flag reg] {dest}, {source}, {count}
        rorb,
        rorw,
        rordw,
        rorqw,

        // ROL[.size][:flag reg] {dest}, {source}, {count}
        rolb,
        rolw,
        roldw,
        rolqw,

        // AND[.size][:flag reg] {dest}, {lhs}, {rhs}
        andb,
        andw,
        anddw,
        andqw,

        // OR[.size][:flag reg] {dest}, {lhs}, {rhs}
        orb,
        orw,
        ordw,
        orqw,

        // XOR[.size][:flag reg] {dest}, {lhs}, {rhs}
        xorb,
        xorw,
        xordw,
        xorqw,

        // NOT[.size][:flag reg] {dest}, {source}
        notb,
        notw,
        notdw,
        notqw,

        // PUSH {reg | imm 32-bit | start reg-end reg}
        pushi,
        pushs,
        pushm,

        // POP  {reg | start reg-end reg}
        pops,
        popm,

        // CALL {reg | imm 32-bit relative offset}
        call,

        // RET
        ret,

        // TRAP {reg | imm 32-bit}
        trap,

        // JUMP {reg | imm 32-bit relative offset}
        jump,

        // BEQ  {reg}, {reg | imm 32-bit relative offset}
        beq,

        // BNE  {reg}, {reg | imm 32-bit relative offset}
        bne,

        // BG   {reg}, {reg | imm 32-bit relative offset}
        bg,

        // BGE  {reg}, {reg | imm 32-bit relative offset}
        bge,

        // BL   {reg}, {reg | imm 32-bit relative offset}
        bl,

        // BLE  {reg}, {reg | imm 32-bit relative offset}
        ble,

        // BOS  {reg}, {reg | imm 32-bit relative offset}
        bos,

        // BOC  {reg}, {reg | imm 32-bit relative offset}
        boc,

        // BCS  {reg}, {reg | imm 32-bit relative offset}
        bcs,

        // BCC  {reg}, {reg | imm 32-bit relative offset}
        bcc,

        // SEQ  {reg}, {reg}
        seq,

        // SNE  {reg}, {reg}
        sne,

        // SG   {reg}, {reg}
        sg,

        // SGE  {reg}, {reg}
        sge,

        // SL   {reg}, {reg}
        sl,

        // SLE  {reg}, {reg}
        sle,

        // SOS  {reg}, {reg}
        sos,

        // SOC  {reg}, {reg}
        soc,

        // SCS  {reg}, {reg}
        scs,

        // SCC  {reg}, {reg}
        scc,

        // CMP[.size]  {result}, {lhs}, {rhs}
        // NOTE: {result} LSB mask: 00000001=equal, 00000000=not equal/greater, 10000000=less
        //
        // TEST: this might or might not be a good idea but get some data
        // if foo == 2 && bar != true
        //
        // move.b r0, 2
        // move.b r1, 1
        //
        // not short-circuit
        //
        // move.b r2, 0
        // cmp.b r2, r0, 2
        // shl.b r2, r2, 1
        // cmp.b r2, r1, 1
        // cmp.b r2, r2, %0000_0011
        // bne _if_fail
        // ...do something inside if...
        // ret
        // _if_fail:
        // ret
        //
        // short-circuit
        //
        // cmp.b r2, r0, 2
        // bne _if_fail
        // cmp.b r2, r1, 1
        // bne _if_fail
        // ...do something inside if...
        // ret
        // _if_fail:
        // ret
        //
        // before all of this: r2 = ...1111_0011
        // move.b r3, 128
        // cmp.b  r2, r3, 128
        // r2 = 0111_0011

        cmpb,
        cmpw,
        cmpdw,
        cmpqw,

        // CMPS[.size]  {result}, {lhs}, {rhs}
        // NOTE: {result} LSB mask: 00000001=equal, 00000000=not equal/greater, 10000000=less
        cmpsb,
        cmpsw,
        cmpsdw,
        cmpsqw,

        // CMPF[.size]  {result}, {lhs}, {rhs}
        // NOTE: {result} LSB mask: 00000001=equal, 00000000=not equal/greater, 10000000=less
        cmpfdw,
        cmpfqw,

        // BIS {result reg}, {reg}, {reg | 8-bit imm}
        bis,

        // BIC {result reg}, {reg}, {reg | 8-bit imm}
        bic,

        // EXIT
        exit
    };

    struct instruction_t final {
        u64 op:8;
        u64 operands:56;
        //
        //           1         2         3         4         5
        // 9         1         1         1         1         1
        // 0123 456789 012345 678901 234567890123456789012345678901234
        //
        // flag|000100|000101|001110| 00000000000000000000000000000000
    };

}

