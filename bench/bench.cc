#include "../logs/yimingcode.h"
#include "bench.h"
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>

using namespace yimingcode;

static BenchResult runCase(const std::string& label, bool asynchronous,
                           size_t threads, size_t messages, size_t message_size,
                           const std::string& path)
{
    const std::string name = "bench_" + label;
    std::unique_ptr<LoggerBuilder> builder(new GlobalLoggerBuilder());
    builder->buildLoggerName(name);
    builder->buildFormatter("%m%n");
    builder->buildSink<FileSink>(path);
    builder->buildLoggerType(asynchronous ? LoggerType::LOGGER_ASYNC : LoggerType::LOGGER_SYNC);
    if (asynchronous) builder->buildEnableUnSafeAsync();
    builder->build();

    std::cout << "\n" << label << ": " << threads << " threads, " << messages
              << " messages, " << message_size << " bytes/message, "
              << (asynchronous ? "async" : "sync") << "\n";
    BenchResult result = bench(name, asynchronous, threads, message_size, messages, path);
    printResult(label, result, messages, asynchronous);
    return result;
}

int main()
{
    mkdir("bench-output", 0755);
    std::cout << "C++11 logger benchmark; file output, formatter=%m%n."
              << " Async end-to-end timing includes worker drain and logger destruction;"
              << " it does not measure fsync durability.\n";
    runCase("async-1t-10000", true, 1, 10000, 100, "bench-output/async-1t.log");
    runCase("async-4t-50000", true, 4, 50000, 100, "bench-output/async-4t.log");
    runCase("async-2t-256b", true, 2, 10000, 256, "bench-output/async-256b.log");
    runCase("async-2t-512b", true, 2, 10000, 512, "bench-output/async-512b.log");
    runCase("sync-1t-5000", false, 1, 5000, 100, "bench-output/sync-1t.log");
    runCase("sync-4t-20000", false, 4, 20000, 100, "bench-output/sync-4t.log");
    return 0;
}
