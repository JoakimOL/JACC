#include "driver.h"
#include "grammar.h"
#include "first_follow_set_generator.h"
#include <fmt/core.h>
#include <gtest/gtest.h>

TEST(Grammars, ProductionSymbolsBasicPropertiesAreAllright)
{
    auto symbol_f_terminal = ProductionSymbol("f", ProductionSymbol::Kind::Terminal);
    EXPECT_FALSE(symbol_f_terminal.is_nonTerminal());
    EXPECT_FALSE(symbol_f_terminal.is_epsilon());
    EXPECT_TRUE(symbol_f_terminal.is_terminal());
    EXPECT_TRUE(symbol_f_terminal.is_initialized());

    auto symbol_epsilon = ProductionSymbol(std::nullopt, ProductionSymbol::Kind::Terminal);
    EXPECT_FALSE(symbol_epsilon.is_nonTerminal());
    EXPECT_TRUE(symbol_epsilon.is_epsilon());
    EXPECT_TRUE(symbol_epsilon.is_terminal());
    EXPECT_TRUE(symbol_epsilon.is_initialized());

    auto symbol_F_non_terminal = ProductionSymbol("F", ProductionSymbol::Kind::NonTerminal);
    EXPECT_TRUE(symbol_F_non_terminal.is_nonTerminal());
    EXPECT_FALSE(symbol_F_non_terminal.is_epsilon());
    EXPECT_FALSE(symbol_F_non_terminal.is_terminal());
    EXPECT_TRUE(symbol_F_non_terminal.is_initialized());
}

TEST(Grammars, ProductionSymbolsComparisonsMakeSense)
{
    auto symbol_f_terminal = ProductionSymbol("f", ProductionSymbol::Kind::Terminal);
    auto symbol_f_terminal2 = ProductionSymbol("f", ProductionSymbol::Kind::Terminal);
    auto symbol_g_terminal = ProductionSymbol("g", ProductionSymbol::Kind::Terminal);

    // We expect it to be alphabetically ordered
    EXPECT_TRUE(symbol_f_terminal < symbol_g_terminal);

    // Symbols are not unique, so two symbols with the same
    // raw_symbol and kind are considered equal
    EXPECT_TRUE(symbol_f_terminal == symbol_f_terminal);
}

TEST(Grammars, ProductionsCanFindEpsilon)
{
    auto terminal_production_without_epsilon = Production(
        {{"f", ProductionSymbol::Kind::Terminal}, {"g", ProductionSymbol::Kind::Terminal}});

    auto terminal_production_with_epsilon =
        Production({{"f", ProductionSymbol::Kind::Terminal}, ProductionSymbol::create_epsilon()});

    auto production_with_epsilon_only =
        Production({std::nullopt, ProductionSymbol::Kind::Terminal});

    auto non_terminal_production_without_epsilon = Production(
        {{"G", ProductionSymbol::Kind::NonTerminal}, {"g", ProductionSymbol::Kind::Terminal}});

    auto non_terminal_production_with_epsilon = Production(
        {{"F", ProductionSymbol::Kind::NonTerminal}, ProductionSymbol::create_epsilon()});

    EXPECT_FALSE(terminal_production_without_epsilon.is_epsilon());
    EXPECT_FALSE(terminal_production_with_epsilon.is_epsilon());
    EXPECT_TRUE(production_with_epsilon_only.is_epsilon());
    EXPECT_FALSE(non_terminal_production_without_epsilon.is_epsilon());
    EXPECT_FALSE(non_terminal_production_with_epsilon.is_epsilon());
}

TEST(Grammars, RulesCanFindEpsilonDerivations)
{
    auto LHS = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};

    auto terminal_production_without_epsilon = Production{
        {{"f", ProductionSymbol::Kind::Terminal}, {"g", ProductionSymbol::Kind::Terminal}}};
    auto terminal_rule_without_epsilon = GrammarRule{LHS, terminal_production_without_epsilon};

    auto terminal_production_with_epsilon = std::vector<Production>{
        Production{std::vector<ProductionSymbol>{{"f", ProductionSymbol::Kind::Terminal},
                                                 {"g", ProductionSymbol::Kind::Terminal}}},

        Production{std::vector<ProductionSymbol>{ProductionSymbol::create_epsilon()}}};
    auto terminal_rule_with_epsilon = GrammarRule{LHS, terminal_production_with_epsilon};

    EXPECT_FALSE(terminal_rule_without_epsilon.rule_contains_epsilon_production());
    EXPECT_TRUE(terminal_rule_with_epsilon.rule_contains_epsilon_production());
}

// TEST(FirstSetGeneration, FirstSetOfTerminalIsSelf)
TEST(FirstSetGeneration, FirstSetOfProductionWithOnlyTerminalIsSelf)
{
    auto non_terminal = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};
    auto terminal = ProductionSymbol{"f", ProductionSymbol::Kind::Terminal};
    auto grammar = Grammar{GrammarRule{non_terminal, Production{terminal}}};

    auto set_generator = FirstFollowSetGenerator(grammar);
    auto first_set = set_generator.generate_first_sets()[non_terminal];
    auto search = first_set.find(terminal);

    EXPECT_TRUE(search != first_set.end())
        << fmt::format("grammar:{}\nfirst_set:{}", grammar, first_set);
    EXPECT_EQ(*search, terminal);
}

TEST(FirstSetGeneration, CanGenerateFirstSetFromFile)
{
    Driver driver;
    driver.parse(std::string{EXAMPLE_GRAMMAR_DIR}.append("test.bnf"));
    auto set_generator = FirstFollowSetGenerator(driver.grammar);

    auto first_set_actual = set_generator.generate_first_sets();
    auto first_set_correct = std::map<ProductionSymbol, std::set<ProductionSymbol>>{
        {ProductionSymbol("S", ProductionSymbol::Kind::NonTerminal),
         std::set{ProductionSymbol("1", ProductionSymbol::Kind::Terminal),
                  ProductionSymbol("2", ProductionSymbol::Kind::Terminal)}}};
    EXPECT_EQ(first_set_actual, first_set_correct);
}

TEST(FirstSetGeneration, CanGenerateFirstSetFromExpressionExample)
{
    // non terminals
    auto e = ProductionSymbol{"E", ProductionSymbol::Kind::NonTerminal};
    auto ep = ProductionSymbol{"E'", ProductionSymbol::Kind::NonTerminal};
    auto t = ProductionSymbol{"T", ProductionSymbol::Kind::NonTerminal};
    auto tp = ProductionSymbol{"T'", ProductionSymbol::Kind::NonTerminal};
    auto f = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};

    // terminals
    auto epsilon = ProductionSymbol::create_epsilon();
    auto plus = ProductionSymbol{"+", ProductionSymbol::Kind::Terminal};
    auto times = ProductionSymbol{"*", ProductionSymbol::Kind::Terminal};
    auto lparen = ProductionSymbol{"(", ProductionSymbol::Kind::Terminal};
    auto rparen = ProductionSymbol{")", ProductionSymbol::Kind::Terminal};
    auto id = ProductionSymbol{"id", ProductionSymbol::Kind::Terminal};

    // rules
    auto e_rule = GrammarRule{e, Production{{t, ep}}};
    auto ep_rule = GrammarRule{ep, {Production{{plus, t, ep}}, Production{epsilon}}};
    auto t_rule = GrammarRule{t, Production{{f, tp}}};
    auto tp_rule = GrammarRule{tp, {Production{{times, f, tp}}, Production{epsilon}}};
    auto f_rule = GrammarRule{f, {Production{{lparen, e, rparen}}, Production{id}}};

    // grammar
    auto grammar = Grammar{{e_rule, ep_rule, t_rule, tp_rule, f_rule}};

    auto set_generator = FirstFollowSetGenerator(grammar);
    auto first_set_actual = set_generator.generate_first_sets();
    auto first_set_correct = std::map<ProductionSymbol, std::set<ProductionSymbol>>{
        {e, {lparen, id}},      {ep, {plus, epsilon}}, {t, {lparen, id}},
        {tp, {times, epsilon}}, {f, {lparen, id}},
    };

    EXPECT_EQ(first_set_actual, first_set_correct);
}
