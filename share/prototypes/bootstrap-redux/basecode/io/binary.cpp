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

#include <fstream>
#include <basecode/errors/errors.h>
#include "binary.h"

namespace basecode::io::binary {

    bool read(
            result_t& r,
            const path_t& path,
            std::iostream& stream) {
        try {
            std::ifstream file(
                path.string(),
                std::ios::in|std::ios::binary);
            if (file.is_open()) {
                stream << file.rdbuf();
                file.close();
            }
        } catch (std::exception& e) {
            errors::add_error(r, errors::io::unable_to_read_file, e.what());
            return false;
        }
        return true;
    }

    bool write(
            result_t& r,
            const path_t& path,
            const std::iostream& stream) {
        try {
            std::ofstream file(
                path.string(),
                std::ios::out|std::ios::binary);
            if (file.is_open()) {
                file << stream.rdbuf();
                file.close();
            }
        } catch (std::exception& e) {
            errors::add_error(r, errors::io::unable_to_write_file, e.what());
            return false;
        }
        return true;
    }

}