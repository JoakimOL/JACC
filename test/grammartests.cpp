#include "grammar.h"
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
        Production({{"f", ProductionSymbol::Kind::Terminal},
                    {std::nullopt, ProductionSymbol::Kind::Terminal}});

    auto production_with_epsilon_only =
        Production({std::nullopt, ProductionSymbol::Kind::Terminal});

    auto non_terminal_production_without_epsilon = Production(
        {{"G", ProductionSymbol::Kind::NonTerminal}, {"g", ProductionSymbol::Kind::Terminal}});

    auto non_terminal_production_with_epsilon =
        Production({{"F", ProductionSymbol::Kind::NonTerminal},
                    {std::nullopt, ProductionSymbol::Kind::Terminal}});

    EXPECT_FALSE(terminal_production_without_epsilon.contains_epsilon());
    EXPECT_TRUE(terminal_production_with_epsilon.contains_epsilon());
    EXPECT_TRUE(production_with_epsilon_only.contains_epsilon());
    EXPECT_FALSE(non_terminal_production_without_epsilon.contains_epsilon());
    EXPECT_TRUE(non_terminal_production_with_epsilon.contains_epsilon());
}

TEST(Grammars, FirstSetOfTerminalIsSelf)
{
    auto non_terminal = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};
    auto terminal = ProductionSymbol{"f", ProductionSymbol::Kind::Terminal};
    auto grammar = Grammar{GrammarRule{non_terminal, Production{terminal}}};

    auto first_set = grammar.generate_first_sets()[non_terminal];
    auto search = first_set.find(terminal);

    EXPECT_TRUE(search != first_set.end())
        << fmt::format("grammar:{}\nfirst_set:{}", grammar, first_set);
    EXPECT_EQ(*search, terminal);
}
