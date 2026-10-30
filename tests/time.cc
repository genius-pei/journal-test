#include "logs/yimingcode.h"
#include "extend/roll_by_time_sink.hpp"
#include <dirent.h>
#include <iostream>

// Deterministic clock substitution in this test executable only.
static time_t test_now = 1700000040; // divisible by 60
extern "C" time_t time(time_t* out) noexcept {
    if (out) *out = test_now;
    return test_now;
}
static int file_count() {
    int count = 0;
    DIR* dir = opendir(".");
    if (!dir) return -1;
    while (dirent* entry = readdir(dir)) {
        if (std::string(entry->d_name).find("roll-") == 0) ++count;
    }
    closedir(dir);
    return count;
}
int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "second";
    const bool second = mode == "second";
    const int expected = mode == "minute-same" ? 1 : 2;
    {
        RollByTimeSink sink("roll-", second ? TimeGap::GAP_SECOND : TimeGap::GAP_MINUTE);
        sink.log("first\n", 6);
        test_now += mode == "minute-boundary" ? 60 : 1;
        sink.log("second\n", 7);
    }
    const int actual = file_count();
    std::cout << mode << " expected_files=" << expected << " actual_files=" << actual << '\n';
    return actual == expected ? 0 : 1;
}
