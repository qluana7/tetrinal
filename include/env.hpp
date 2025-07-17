#pragma once

#include <vector>
#include <string>

#include <filesystem>

#include <lib/intdef>

/* static */ class env {
    /* Raw values */
    static inline char** _S_raw_arguments;
    static inline i32 _S_raw_argument_count;
    /* ---------- */

    static inline std::filesystem::path _S_executable_path;
    static inline std::vector<std::string> _S_arguments;

public:
    static void initialize(i32 argc, char** argv) {
        _S_raw_argument_count = argc;
        _S_raw_arguments = argv;

        _S_executable_path = argv[0];
        _S_arguments.reserve(argc - 1);

        for (i32 i = 1; i < argc; ++i) {
            _S_arguments.push_back(std::string(argv[i]));
        }
    }

    static inline const std::filesystem::path& exec_path() { return _S_executable_path; }
    static inline std::filesystem::path exec_dir() { return exec_path().parent_path(); }
    static inline const std::vector<std::string>& arguments() { return _S_arguments; }

    static inline std::pair<i32, char**> raw()
    { return { _S_raw_argument_count, _S_raw_arguments }; };
};