" ----------------------------------------------------------------------------
" ____                               _
" |  _\                             | |
" | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
" |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
" | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
" |____/\__,_|___/\___|\___\___/ \__,_|\___|
"
"       C O M P I L E R  P R O J E C T
"
" Copyright (C) 2019 Jeff Panici
" All rights reserved.
"
" This software source file is licensed under the terms of MIT license.
" For details, please read the LICENSE file.
"
" ----------------------------------------------------------------------------

function! Project_UpdateTags()
    UpdateTags! -R bc/ compiler/ tests/
endfunction

function! Project_Cscope()
    call asyncrun#quickfix_toggle(15)
    AsyncRun cscope -Rb
endfunction

function! Project_CMakeGenerate(reset)
    call asyncrun#quickfix_toggle(15, 1)

    if a:reset != ''
        silent execute "!cd build/debug && rm -rf clang && mkdir clang"
    endif

    AsyncRun cd build/debug/clang && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ -G "Ninja" -Wno-dev /Users/jeff/src/basecode-lang/bootstrap-redux
endfunction

function! Project_Build(...)
    call asyncrun#quickfix_toggle(15, 1)

    let l:cmd = "cd build/debug/clang && cmake --build ."
    
    for arg in a:000
        let l:cmd = l:cmd . " --target " . arg
    endfor

    let l:cmd = l:cmd . " -- -j12"

    call asyncrun#run(0, '', l:cmd) 
endfunction

function! Project_Run(...)
    call asyncrun#quickfix_toggle(15, 1)

    let l:cmd = "cd build/debug/clang/bin" 

    for arg in a:000
        let l:cmd = l:cmd . " && ./" . arg
    endfor

    call asyncrun#run(0, '', l:cmd)
endfunction

command! ProjectCscope :call Project_Cscope()
command! ProjectUpdateTags :call Project_UpdateTags()
command! -nargs=+ ProjectRun :call Project_Run(<f-args>)
command! -nargs=+ ProjectBuild :call Project_Build(<f-args>)
command! -bang ProjectCmakeGenerate :call Project_CMakeGenerate('<bang>')

"autocmd FileType cpp :TagbarOpen
"autocmd BufUnload * :TagbarClose
"
nnoremap <F9> :ProjectBuild test-lexer<cr>
nnoremap <S-F9> :ProjectRun test-lexer<cr>
nnoremap <S-F4> :cclose<cr>
