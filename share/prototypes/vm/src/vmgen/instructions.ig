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

registers(size=64, base=u8) {
    PC( mask=%00000001, name='pc',  desc='program counter');
    SP( mask=%00000010, name='sp',  desc='stack pointer');
    FP( mask=%00000011, name='fp',  desc='frame pointer');
    R0( mask=%00000100, name='r0',  desc='general/flag register r0',  flag_target=true);
    R1( mask=%00000101, name='r1',  desc='general/flag register r1',  flag_target=true);
    R2( mask=%00000110, name='r2',  desc='general/flag register r2',  flag_target=true);
    R3( mask=%00000111, name='r3',  desc='general/flag register r3',  flag_target=true);
    R4( mask=%00001000, name='r4',  desc='general/flag register r4',  flag_target=true);
    R5( mask=%00001001, name='r5',  desc='general/flag register r5',  flag_target=true);
    R6( mask=%00001010, name='r6',  desc='general/flag register r6',  flag_target=true);
    R7( mask=%00001011, name='r7',  desc='general/flag register r7',  flag_target=true);
    R8( mask=%00001100, name='r8',  desc='general/flag register r8',  flag_target=true);
    R9( mask=%00001101, name='r9',  desc='general/flag register r9',  flag_target=true);
    R10(mask=%00001110, name='r10', desc='general/flag register r10', flag_target=true);
    R11(mask=%00001111, name='r11', desc='general/flag register r11', flag_target=true);
    R12(mask=%00010000, name='r12', desc='general register r12');
    R13(mask=%00010001, name='r13', desc='general register r13');
    R14(mask=%00010010, name='r14', desc='general register r14');
    R15(mask=%00010011, name='r15', desc='general register r15');
    R16(mask=%00010100, name='r16', desc='general register r16');
    R17(mask=%00010101, name='r17', desc='general register r17');
    R18(mask=%00010110, name='r18', desc='general register r18');
    R19(mask=%00010111, name='r19', desc='general register r19');
    R20(mask=%00011000, name='r20', desc='general register r20');
    R21(mask=%00011001, name='r21', desc='general register r21');
    R22(mask=%00011010, name='r22', desc='general register r22');
    R23(mask=%00011011, name='r23', desc='general register r23');
    R24(mask=%00011100, name='r24', desc='general register r24');
    R25(mask=%00011101, name='r25', desc='general register r25');
    R26(mask=%00011110, name='r26', desc='general register r26');
    R27(mask=%00011111, name='r27', desc='general register r27');
    R28(mask=%00100000, name='r28', desc='general register r28');
    R29(mask=%00100001, name='r29', desc='general register r29');
    R30(mask=%00100010, name='r30', desc='general register r30');
    R31(mask=%00100011, name='r31', desc='general register r31');
}

flags(pos=9..13) {
    ZF(mask=%00000000_00000000_00000000_00000000_00000000_00000000_00000000_00000001, name='ZF', desc='Zero Flag');
    PF(mask=%00010000_00000000_00000000_00000000_00000000_00000000_00000000_00000000, name='PF', desc='Parity Flag');
    SF(mask=%00100000_00000000_00000000_00000000_00000000_00000000_00000000_00000000, name='SF', desc='Sign Flag');
    CF(mask=%01000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000, name='CF', desc='Carry Flag');
    OF(mask=%10000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000, name='OF', desc='Overflow Flag');
}

immediates(pos=24..56) {
    imm8( size=8 );
    imm12(size=12);
    imm16(size=16);
    imm24(size=24);
    imm32(size=32);
    imm64(size=64);
}

encoding(instruction_t, base=u64) {
    field(op,       type=u64, pos=0..7);
    field(operands, type=u64, pos=8..55);
}

sizes(register_zone_t) {
    b(
        int  = u8,
        mask = %00000001,
        msb  = %00000000_00000000_00000000_00000000_00000000_00000000_00000000_10000000,
        name = '.b',
        desc = 'byte'
    );
    w(
        int  = u16,
        mask = %00000010,
        msb  = %00000000_00000000_00000000_00000000_00000000_00000000_10000000_00000000,
        name = '.w',
        desc = 'word'
    );
    dw(
        int   = u32,
        float = f32,
        mask  = %00000100,
        msb   = %00000000_00000000_00000000_00000000_10000000_00000000_00000000_00000000,
        name  = '.dw',
        desc  = 'double word'
    );
    qw(
        int     = u64,
        float   = f64,
        mask    = %00001000,
        msb     = %10000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000,
        name    = '.qw',
        desc    = 'quad word',
        default = true
    );
}

op(load, pos=0..7) {
    flag(R0..R11);
    size(b|w|dw|qw);
    operand(val) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(addr) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(offs, optional=true) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        val <- zero_extend([addr + offs]);
        ZF  <- !val;
        SF  <- val & msb();
        CF  <- 0;
        OF  <- 0;
        PF  <- !(popcount(val) & 1);
    }
    jit() {
        target(x86_64) {
            movzx(val, [addr + offs]);
        }
        target(AArch64) {
            ldr(val, [addr + offs]);
        }
    }
}

op(loads, pos=0..7) {
    flag(R0..R11);
    size(b|w|dw|qw);
    operand(val) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(addr) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(offs, optional=true) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        val <- sign_extend([addr + offs]);
        ZF  <- !val;
        SF  <- val & msb();
        CF  <- 0;
        OF  <- 0;
        PF  <- !(popcount(dst) & 1);
    }
    jit() {
        target(x86_64) {
            movsx(val, [addr + offs]);
        }
        target(AArch64) {
            ldrs(val, [addr + offs]);
        }
    }
}

op(store, pos=0..7) {
    size(b|w|dw|qw);
    operand(val) {
        reg(R0..R31|SP|FP|PC, pos=14..20);
    }
    operand(addr) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(offs, optional=true) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        [addr + offs] <- zero_extend(val);
    }
    jit() {
        target(x86_64) {
            movzx([addr + offs], val);
        }
        target(AArch64) {
            str(val, [addr + offs]);
        }
    }
}

op(stores, pos=0..7) {
    size(b|w|dw|qw);
    operand(val) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(addr) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(offs, optional=true) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        [addr + offs] <- sign_extend(val);
    }
    jit() {
        target(x86_64) {
            movzx([addr + offs], val);
        }
        target(AArch64) {
            strs(val, [addr + offs]);
        }
    }
}

op(copy, pos=0..7) {
    operand(daddr) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(saddr) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(len) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        copy(daddr, saddr, len);
    }
    jit() {
        target(x86_64) {
        }
        target(AArch64) {
        }
    }
}

op(fill, pos=0..7) {
    operand(addr) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(val) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(len) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        fill(addr, val, len);
    }
    jit() {
        target(x86_64) {
        }
        target(AArch64) {
        }
    }
}

op(lfence, pos=0..7) {
    terp() {
        lfence();
    }
    jit() {
        target(x86_64) {
            lfence();
        }
    }
}

op(sfence, pos=0..7) {
    terp() {
        sfence();
    }
    jit() {
        target(x86_64) {
            sfence();
        }
    }
}

op(mfence, pos=0..7) {
    terp() {
        mfence();
    }
    jit() {
        target(x86_64) {
            mfence();
        }
    }
}

op(_moveqwx_, extension=true, pos=0..7) {
    operand(src) {
        imm32();
    }
}

op(move, pos=0..7) {
    flag(R0..R11);
    size(b|w|dw|qw);
    operand(dst) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(src) {
        reg(R0..R31|SP|FP|PC, pos=21..27)
            | imm8()
            | imm12()
            | imm16()
            | imm24()
            | imm32()
            | imm64(extension=_moveqwx_) {
                src <- src << 32 | _moveqwx_.src;
            };
    }
    terp() {
        dst <- zero_extend(src);
        ZF  <- !dst;
        SF  <- dst & msb();
        CF  <- 0;
        OF  <- 0;
        PF  <- !(popcount(dst) & 1);
    }
    jit() {
        target(x86_64) {
            movzx(dst, src);
        }
        target(AArch64) {
            mov(src, dst);
        }
    }
}

op(moves, pos=0..7) {
    flag(R0..R11);
    size(b|w|dw|qw);
    operand(dst) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(src) {
        reg(R0..R31|SP|FP|PC, pos=21..27)
            | imm8()
            | imm12()
            | imm16()
            | imm24()
            | imm32()
            | imm64(extension=_moveqwx_) {
                src <- src << 32 | _moveqwx_.src;
            };
    }
    terp() {
        dst <- sign_extend(src);
        ZF  <- !dst;
        SF  <- dst & msb();
        CF  <- 0;
        OF  <- 0;
        PF  <- !(popcount(dst) & 1);
    }
    jit() {
        target(x86_64) {
            movsx(dst, src);
        }
        target(AArch64) {
            movs(src, dst);
        }
    }
}

op(addf, type=float, pos=0..7) {
    flag(R0..R11);
    size(dw|qw);
    operand(sum) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(augend) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(addend) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        sum <- augend + addend;
        //ZF  <- !sum;
        //SF  <- sum & msb();
        //PF  <- !(popcount(dst) & 1);
        //CF  <- (augend == zone_mask()) && addend;
        //OF  <- 0;
    }
    jit() {
        target(x86_64) {
        }
        target(AArch64) {
        }
    }
}

op(addi, pos=0..7) {
    flag(R0..R11);
    size(b|w|dw|qw);
    operand(sum) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(augend) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(addend) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        sum <- zero_extend(augend) + zero_extend(addend);
        ZF  <- !sum;
        SF  <- sum & msb();
        PF  <- !(popcount(dst) & 1);
        CF  <- (augend == zone_mask()) && addend;
        OF  <- 0;
    }
    jit() {
        target(x86_64) {
            movzx(sum, augend);
            add  (sum, addend);
        }
        target(AArch64) {
            add  (sum, augend, addend);
        }
    }
}

op(addis, pos=0..7) {
    flag(R0..R11);
    size(b|w|dw|qw);
    operand(sum) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    operand(augend) {
        reg(R0..R31|SP|FP|PC, pos=21..27);
    }
    operand(addend) {
        reg(R0..R31|SP|FP|PC, pos=28..34) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        sum <- sign_extend(augend) + sign_extend(addend);
        ZF  <- !sum;
        SF  <- sum & msb();
        PF  <- !(popcount(dst) & 1);
        OF  <- ~(augend ^ addend) & (augend ^ sum) & zone_mask();
        CF  <- 0;
    }
    jit() {
        target(x86_64) {
            movsx(sum, augend);
            add  (sum, addend);
        }
        target(AArch64) {
        }
    }
}

op(push, pos=0..7) {
    operand(val) {
        reg(R0..R31|SP|FP|PC, pos=14..20) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        [--SP] <- val;
    }
    jit() {
        target(x86_64) {
            push val;
        }
        target(AArch64) {
            sub(sp, sp, size_in_bytes(size()));
            str(val, [sp]);
        }
    }
}

op(pop, pos=0..7) {
    operand(val) {
        reg(R0..R31|SP|FP, pos=14..20);
    }
    terp() {
        val <- [SP++];
    }
    jit() {
        target(x86_64) {
            pop(val);
        }
        target(AArch64) {
            ldr(val, [sp]);
            add(sp, sp, size_in_bytes(size()));
        }
    }
}

op(call, pos=0..7) {
    operand(addr) {
        reg(R0..31|SP|FP, pos=14..20) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        [--SP]  <- PC;
        PC      <- PC + addr;
    }
    jit() {
        target(x86_64) {
            call (addr, rip=true);
        }
        target(AArch64) {
            local(addr=reg_alloc());
            ldr  (addr, PC + addr);
            br   (addr);
        }
    }
}

op(jump, pos=0..7) {
    operand(addr) {
        reg(R0..31|SP|FP, pos=14..20) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        PC <- PC + addr;
    }
    jit() {
        target(x86_64) {
            jmp  (addr, rip=true);
        }
        target(AArch64) {
            local(addr=reg_alloc());
            ldr  (addr, PC + addr);
            b    (PC + addr);
        }
    }
}

op(trap, pos=0..7) {
    operand(mode) {
        reg(R0..R31|SP|FP, pos=14..20) | imm8() | imm12() | imm16() | imm24() | imm32();
    }
    terp() {
        trap(mode);
    }
    // XXX: how to translate to native?
}

op(ret, pos=0..7) {
    terp() {
        PC <- [SP++];
    }
    jit() {
        target(x86_64) {
            ret();
        }
        target(AArch64) {
            bx(lr);
        }
    }
}

op(nop, pos=0..7) {
    terp() {
        nop();
    }
    jit() {
        target(x84_64) {
            nop();
        }
        target(AArch64) {
            nop();
        }
    }
}

op(exit, pos=0..7) {
    terp() {
        halt();
    }
}