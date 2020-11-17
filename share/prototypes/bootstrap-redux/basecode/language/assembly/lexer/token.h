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

#include <basecode/types.h>
#include <basecode/language/common.h>

namespace basecode::language::assembly::lexer {

    using namespace std::literals;

    enum class token_type_t {
        plus,
        comma,
        minus,
        colon,
        period,
        symbol,
        dollar,
        percent,
        exponent,
        asterisk,
        directive,
        i_register,
        f_register,
        left_paren,
        back_slash,
        left_brace,
        right_brace,
        right_paren,
        pc_register,
        sp_register,
        fp_register,
        line_comment,
        left_bracket,
        right_bracket,
        forward_slash,
        number_literal,
        string_literal,
        size_specifier,
        or_instruction,
        bz_instruction,
        bs_instruction,
        bo_instruction,
        ba_instruction,
        bb_instruction,
        bg_instruction,
        bl_instruction,
        nop_instruction,
        clr_instruction,
        pop_instruction,
        dup_instruction,
        inc_instruction,
        dec_instruction,
        add_instruction,
        sub_instruction,
        mul_instruction,
        div_instruction,
        mod_instruction,
        neg_instruction,
        shr_instruction,
        shl_instruction,
        ror_instruction,
        rol_instruction,
        pow_instruction,
        and_instruction,
        xor_instruction,
        not_instruction,
        bis_instruction,
        bic_instruction,
        cmp_instruction,
        bnz_instruction,
        tbz_instruction,
        bne_instruction,
        beq_instruction,
        bcc_instruction,
        bcs_instruction,
        bae_instruction,
        bbe_instruction,
        bge_instruction,
        ble_instruction,
        jsr_instruction,
        rts_instruction,
        jmp_instruction,
        swi_instruction,
        ffi_instruction,
        seta_instruction,
        popm_instruction,
        free_instruction,
        size_instruction,
        load_instruction,
        copy_instruction,
        fill_instruction,
        move_instruction,
        push_instruction,
        madd_instruction,
        test_instruction,
        tnbz_instruction,
        setb_instruction,
        setc_instruction,
        setg_instruction,
        setl_instruction,
        sets_instruction,
        seto_instruction,
        setz_instruction,
        swap_instruction,
        trap_instruction,
        meta_instruction,
        exit_instruction,
        setnl_instruction,
        setle_instruction,
        setna_instruction,
        setae_instruction,
        setnb_instruction,
        setbe_instruction,
        setnc_instruction,
        setng_instruction,
        setge_instruction,
        setns_instruction,
        setno_instruction,
        setnz_instruction,
        character_literal,
        alloc_instruction,
        store_instruction,
        moves_instruction,
        movez_instruction,
        pushm_instruction,
        setnae_instruction,
        setnbe_instruction,
        setnge_instruction,
        setnle_instruction,
        convert_instruction,
    };

    std::string_view token_type_to_name(token_type_t type);

    using token_t = basic_token_t<token_type_t>;

}