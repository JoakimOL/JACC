#include <algorithm>
#include <fmt/base.h>
#include <spdlog/spdlog.h>

#include <optional>
#include <stdexcept>
#include <vector>

#include "argparse/argparse.hpp"
#include <jacc/bearpiglexer.h>
#include <jacc/driver.h>
#include <jacc/first_follow_set_generator.h>
#include <jacc/grammar.h>
#include <jacc/ll_table_generator.h>
#include <jacc/table_driven_ll_parser.h>
#include <libbearpig/lexer.h>
#include <libbearpig/token.h>

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
    program.add_argument("-t").required().help("path to tokens file").metavar("tokens");
    program.add_argument("--lex").default_value(false).implicit_value(true).help(
        "stop after lexing");
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

    auto tokenfilename = program.present("-t");
    spdlog::info("tokenfilename: {}", tokenfilename.has_value() ? *tokenfilename : "nullopt");

    std::vector<bp::TokenRule> rules;
    get_tokens_from_file(*tokenfilename, rules);
    add_skip_whitespace_rule(rules);
    for(auto rule: rules){
        spdlog::info("name: {} pattern: {}", rule.name, rule.pattern);
    }
    bp::Lexer lexer{rules};

    auto cmd_input = program.get<std::string>("input");
    spdlog::info("{}", cmd_input);
    auto result_expected = lexer.tokenize(cmd_input);
    if (!result_expected.has_value()) {
        const auto &error = result_expected.error();
        spdlog::error("lexer error: {} at line {} column {}", error.message, error.line,
                      error.column);
        return 1;
    }

    auto result = result_expected.value();
    auto input = std::vector<ProductionSymbol>{};
    std::for_each(result.begin(), result.end(), [&input](const bp::Token &token) {
        spdlog::info("token {}: \"{}\" (line {} column {})", token.name, token.lexeme, token.line, token.column);
        input.emplace_back(
            token.name, ProductionSymbol::Kind::Terminal,
            ProductionSymbolLoc{token.lexeme, token.offset, token.line, token.column});
    });

    if (program.is_used("--lex"))
        return 0;

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

    spdlog::info(input);
    spdlog::info("first thing of grammar: {}", grammar.get_rules().front());

    LLParser parser{table, grammar.get_rules().front().get_LHS()};

    auto parse_result = parser.parse(input);
    if (!parse_result.has_value()) {
        const auto &error = parse_result.error();
        spdlog::error("parser error: {} at line {} column {}", error.message, error.line,
                      error.column);
        return 1;
    }
    spdlog::info("Success!");
}
