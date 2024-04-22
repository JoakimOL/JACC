#include "grammar.h"
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
