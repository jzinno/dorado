#include "Version.h"
#include "cli/cli.h"
#include "minimap.h"
#include "spdlog/cfg/env.h"
#include "utils/cli_utils.h"

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef __linux__
#include <gnu/libc-version.h>
#endif

using entry_ptr = std::function<int(int, char**)>;

namespace {

void usage(const std::vector<std::string> commands) {
    std::cerr << "Usage: dorado [options] subcommand\n\n"
              << "Positional arguments:" << std::endl;

    for (const auto command : commands) {
        std::cerr << command << std::endl;
    }

    std::cerr << "\nOptional arguments:\n"
              << "-h --help               shows help message and exits\n"
              << "-v --version            prints version information and exits\n"
              << "-vv                     prints verbose version information and exits"
              << std::endl;
}

}  // namespace

int main(int argc, char* argv[]) {
    // Load logging settings from environment/command-line.
    spdlog::cfg::load_env_levels();

    const std::map<std::string, entry_ptr> subcommands = {
            {"basecaller", &dorado::basecaller}, {"duplex", &dorado::duplex},
            {"download", &dorado::download},     {"aligner", &dorado::aligner},
            {"summary", &dorado::summary},
    };

    std::vector<std::string> arguments(argv + 1, argv + argc);
    std::vector<std::string> keys;

    for (const auto& [key, _] : subcommands) {
        keys.push_back(key);
    }

    if (arguments.size() == 0) {
        usage(keys);
        return 0;
    }

    const auto subcommand = arguments[0];

    if (subcommand == "-v" || subcommand == "--version") {
        std::cerr << DORADO_VERSION << std::endl;
    } else if (subcommand == "-vv") {
#ifdef __APPLE__
        std::cerr << "dorado:   " << DORADO_VERSION << std::endl;
#else
        std::cerr << "dorado:   " << DORADO_VERSION << "+cu" << CUDA_VERSION << std::endl;
#endif
        std::cerr << "libtorch: " << TORCH_BUILD_VERSION << std::endl;
        std::cerr << "minimap2: " << MM_VERSION << std::endl;

    } else if (subcommand == "-h" || subcommand == "--help") {
        usage(keys);
        return 0;
    } else if (subcommands.find(subcommand) != subcommands.end()) {
#ifdef __linux__
        // There's a bug in GLIBC < 2.25 (Bug 11941) which can cause the
        // dynamically loaded library to be dlclose-d twice, once by ld.so potentially
        // once by the plugin that opened the DSO (more details available
        // at https://sourceware.org/legacy-ml/libc-alpha/2016-12/msg00859.html). This triggers
        // an assert in GLIBC. However, this can sometimes also corrupt the _at_exit registered
        // subroutines, causing seg faults at program teardown. The workaround below
        // bypasses the _at_exit teardown process and exits immediately, preventing the
        // GLIBC assert and subsequent corruption.
        auto glibc_version = dorado::utils::parse_version_str(gnu_get_libc_version());
        if (std::get<0>(glibc_version) < 3 && std::get<1>(glibc_version) < 25) {
            _Exit(subcommands.at(subcommand)(--argc, ++argv));
        }
#endif
        return subcommands.at(subcommand)(--argc, ++argv);
    } else {
        usage(keys);
        return 1;
    }

    return 0;
}
