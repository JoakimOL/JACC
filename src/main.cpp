#include <spdlog/spdlog.h>

#include <optional>

#include "argparse/argparse.hpp"
#include "driver.h"
#include "fmt/ranges.h"
#include "grammar.h"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program(argv[0]);
    program.add_argument("-f");
    program.add_argument("-v").default_value(false).implicit_value(true);
    program.add_argument("--first").default_value(false).implicit_value(true);
    program.parse_args(argc, argv);
    if (program.is_used("-v")) {
        spdlog::set_level(spdlog::level::debug);
    }

    bool generate_first_sets_only = false;
    if (program.is_used("--first")) {
        generate_first_sets_only = true;
    }
    auto filename = program.present("-f");
    spdlog::info("filename: {}", filename.has_value() ? *filename : "nullopt");

    Driver driver;
    driver.parse(filename.value());
    spdlog::info("grammar: {}", driver.grammar);

    Grammar::set_map<ProductionSymbol> first_sets = driver.grammar.generate_first_sets();
    spdlog::info("first sets: {}", first_sets);
    if(generate_first_sets_only) return 0;
}
