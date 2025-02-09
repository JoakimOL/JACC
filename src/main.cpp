#include <spdlog/spdlog.h>

#include <optional>

#include "argparse/argparse.hpp"
#include "driver.h"
#include "first_follow_set_generator.h"
#include "grammar.h"
#include "ll_table_generator.h"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program(argv[0]);
    program.add_argument("-f");
    program.add_argument("-v").default_value(false).implicit_value(true);
    program.add_argument("--first").default_value(false).implicit_value(true);
    program.add_argument("--follow").default_value(false).implicit_value(true);
    program.parse_args(argc, argv);
    if (program.is_used("-v")) {
        spdlog::set_level(spdlog::level::debug);
    }

    bool generate_first_sets_only = false;
    bool generate_follow_sets_only = false;
    if (program.is_used("--first")) {
        generate_first_sets_only = true;
    } else if (program.is_used("--follow")) {
        generate_follow_sets_only = true;
    }
    auto filename = program.present("-f");
    spdlog::info("filename: {}", filename.has_value() ? *filename : "nullopt");

    Driver driver;
    driver.parse(filename.value());
    Grammar grammar = driver.grammar;
    spdlog::info("grammar: {}", grammar);

    FirstFollowSetGenerator sets_generator(grammar);
    FirstFollowSetGenerator::set_map<ProductionSymbol> first_sets =
        sets_generator.generate_first_sets();
    spdlog::info("first sets: {}", first_sets);
    if (generate_first_sets_only)
        return 0;

    FirstFollowSetGenerator::set_map<ProductionSymbol> follow_sets =
        sets_generator.generate_follow_sets();
    spdlog::info("follow sets: {}", follow_sets);
    if (generate_follow_sets_only)
        return 0;

    auto table = generate_ll_table(grammar, sets_generator);
    spdlog::info("LL-parsing table:");
    for (auto thing : table) {
        spdlog::info(thing);
    }
}
