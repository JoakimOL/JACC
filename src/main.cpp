#include <fmt/base.h>
#include <spdlog/spdlog.h>

#include <optional>
#include <stdexcept>

#include "argparse/argparse.hpp"
#include "driver.h"
#include "first_follow_set_generator.h"
#include "grammar.h"
#include "ll_table_generator.h"
#include "table_driven_ll_parser.h"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program(argv[0]);
    program.add_argument("-f").required().help("path to grammar file").metavar("filename");
    program.add_argument("-v").default_value(false).implicit_value(true).help("enable verbose logging");
    program.add_argument("--grammar").default_value(false).implicit_value(true).help("stop after parsing the input grammar");
    program.add_argument("--first").default_value(false).implicit_value(true).help("stop after generating first sets");
    program.add_argument("--follow").default_value(false).implicit_value(true).help("stop after generating follow sets");
    program.add_argument("--ll").default_value(false).implicit_value(true).help("stop after generating the LL(1) parse table");
    try{
        program.parse_args(argc, argv);
    }
    catch(const std::runtime_error& e){
     // why did you have to use exceptions, mr argparse-developer
        spdlog::error(e.what());
        exit(1);
    }


    if (program.is_used("-v")) {
        spdlog::set_level(spdlog::level::debug);
    }

    auto filename = program.present("-f");
    spdlog::info("filename: {}", filename.has_value() ? *filename : "nullopt");

    Driver driver;
    driver.parse(filename.value());
    Grammar grammar = driver.grammar;
    spdlog::info("grammar: {}", grammar);
    if (program.is_used("--grammar"))
        return 0;

    FirstFollowSetGenerator sets_generator(grammar);
    FirstFollowSetGenerator::set_map<ProductionSymbol> first_sets =
        sets_generator.generate_first_sets();
    spdlog::info("first sets: {}", first_sets);
    if (program.is_used("--first"))
        return 0;

    FirstFollowSetGenerator::set_map<ProductionSymbol> follow_sets =
        sets_generator.generate_follow_sets();
    spdlog::info("follow sets: {}", follow_sets);
    if (program.is_used("--follow"))
        return 0;

    auto table = generate_ll_table(grammar, sets_generator);
    spdlog::info("LL-parsing table:");
    for (auto thing : table) {
        spdlog::info(thing);
    }
    if (program.is_used("--ll"))
        return 0;

    auto input = std::vector<ProductionSymbol>{
        ProductionSymbol("(", ProductionSymbol::Kind::Terminal),
        ProductionSymbol("id", ProductionSymbol::Kind::Terminal),
        ProductionSymbol(")", ProductionSymbol::Kind::Terminal),
        ProductionSymbol("+", ProductionSymbol::Kind::Terminal),
        ProductionSymbol("id", ProductionSymbol::Kind::Terminal),
    };
    spdlog::info(input);

    LLParser parser{table, ProductionSymbol("E", ProductionSymbol::Kind::NonTerminal)};

    if (parser.parse(input)) {
        spdlog::info("Success!");
    } else {
        spdlog::info("Task succeeded with failure");
    }
}
