#include "driver.h"
#include "first_follow_set_generator.h"
#include "grammar.h"
#include <fmt/core.h>
#include <gtest/gtest.h>

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

TEST(FirstSetGeneration, FirstOfTerminalIsSelf)
{
    // If X is a terminal then First(X) is just X!
    // rule 1
    auto non_terminal = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};
    auto terminal = ProductionSymbol{"f", ProductionSymbol::Kind::Terminal};
    auto grammar = Grammar{GrammarRule{non_terminal, Production{terminal}}};

    auto set_generator = FirstFollowSetGenerator(grammar);
    auto first_set = set_generator.first(terminal);
    auto first_set_correct = std::set<ProductionSymbol>{terminal};

    EXPECT_EQ(first_set, first_set_correct)
        << fmt::format("grammar:{}\nfirst_set:{}", grammar, first_set);
}

TEST(FirstSetGeneration, FirstSetOfRuleWithEpsilonContainsEpsilon)
{
    // If there is a Production X → ε then add ε to first(X)
    // rule 2
    auto non_terminal_f = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};
    auto non_terminal_g = ProductionSymbol{"G", ProductionSymbol::Kind::NonTerminal};
    auto epsilon = ProductionSymbol::create_epsilon();
    auto terminal_g = ProductionSymbol{"f", ProductionSymbol::Kind::Terminal};
    auto grammar =
        Grammar{{GrammarRule{non_terminal_f,
                             {Production{epsilon}, Production{{non_terminal_g, non_terminal_g}}}},
                 GrammarRule{non_terminal_g, Production{terminal_g}}}};

    auto set_generator = FirstFollowSetGenerator(grammar);
    auto first_set = set_generator.generate_first_sets()[non_terminal_f];
    auto first_set_for_f_correct = std::set<ProductionSymbol>{epsilon, terminal_g};

    EXPECT_EQ(first_set, first_set_for_f_correct)
        << fmt::format("grammar:{}\nfirst_set:{}", grammar, first_set);
}

TEST(FirstSetGeneration, FirstOfSeveralSymbolsIsFirstOfFirstIfNoEpsilon)
{
    // First(Y1Y2..Yk) is either
    // First(Y1) (if First(Y1) doesn't contain ε)
    // pretty much rule 3 or 4 idunno lol
    auto start = ProductionSymbol{"S", ProductionSymbol::Kind::NonTerminal};
    auto non_terminal_f = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};
    auto non_terminal_g = ProductionSymbol{"G", ProductionSymbol::Kind::NonTerminal};
    auto epsilon = ProductionSymbol::create_epsilon();
    auto terminal_g = ProductionSymbol{"g", ProductionSymbol::Kind::Terminal};
    auto grammar = Grammar{{GrammarRule{start,
                                        // note the order of f and g here
                                        {Production{{non_terminal_g, non_terminal_f}}}},
                            GrammarRule{non_terminal_g, Production{terminal_g}},
                            GrammarRule{non_terminal_f, Production{epsilon}}}};

    auto set_generator = FirstFollowSetGenerator(grammar);
    auto first_set = set_generator.generate_first_sets()[start];
    auto first_set_for_start_correct = std::set<ProductionSymbol>{terminal_g};
    EXPECT_EQ(first_set, first_set_for_start_correct)
        << fmt::format("grammar:{}\nfirst_set:{}", grammar, first_set);
}

TEST(FirstSetGeneration, FirstOfSeveralSymbolsIsFirstOfAllEpsilonSymbolsUntilNonEpsilon)
{
    // pretty much rule 3 or 4 idunno lol
    //  First(Y1Y2..Yk) is either
    //  (if First(Y1) does contain ε) then First (Y1Y2..Yk) is everything in
    //  First(Y1) <except for ε > as well as everything in First(Y2..Yk)
    auto start = ProductionSymbol{"S", ProductionSymbol::Kind::NonTerminal};
    auto non_terminal_f = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};
    auto non_terminal_g = ProductionSymbol{"G", ProductionSymbol::Kind::NonTerminal};
    auto epsilon = ProductionSymbol::create_epsilon();
    auto terminal_f = ProductionSymbol{"f", ProductionSymbol::Kind::Terminal};
    auto terminal_g = ProductionSymbol{"g", ProductionSymbol::Kind::Terminal};
    auto grammar =
        Grammar{{GrammarRule{start,
                             // note the order of f and g here
                             {Production{{non_terminal_f, non_terminal_g}}}},
                 GrammarRule{non_terminal_g, Production{terminal_g}},
                 GrammarRule{non_terminal_f, {Production{epsilon}, Production{terminal_f}}}}};

    auto set_generator = FirstFollowSetGenerator(grammar);
    auto first_set = set_generator.generate_first_sets()[start];

    auto first_set_for_start_correct = std::set<ProductionSymbol>{terminal_f, terminal_g};
    EXPECT_EQ(first_set, first_set_for_start_correct)
        << fmt::format("grammar:{}\nfirst_set:{}", grammar, first_set);
}

TEST(FollowSetGeneration, CanGenerateFollowSetFromExpressionExample)
{
    // non terminals
    auto e = ProductionSymbol{"E", ProductionSymbol::Kind::NonTerminal};
    auto ep = ProductionSymbol{"E'", ProductionSymbol::Kind::NonTerminal};
    auto t = ProductionSymbol{"T", ProductionSymbol::Kind::NonTerminal};
    auto tp = ProductionSymbol{"T'", ProductionSymbol::Kind::NonTerminal};
    auto f = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};

    // terminals
    auto epsilon = ProductionSymbol::create_epsilon();
    auto end_of_input = ProductionSymbol::create_EOI();
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
    auto follow_set_actual = set_generator.generate_follow_sets();
    auto follow_set_correct = std::map<ProductionSymbol, std::set<ProductionSymbol>>{
        {e, {rparen, end_of_input}},
        {ep, {rparen, end_of_input}},
        {t, {plus, rparen, end_of_input}},
        {tp, {plus, rparen, end_of_input}},
        {f, {plus, times, rparen, end_of_input}},
    };

    EXPECT_EQ(follow_set_actual, follow_set_correct);
}

TEST(FollowSetGeneration, CanHandleEOIBaseCase)
{
    // non terminals
    auto s_non_terminal = ProductionSymbol{"S", ProductionSymbol::Kind::NonTerminal};

    // terminals
    auto end_of_input = ProductionSymbol::create_EOI();
    auto s_terminal = ProductionSymbol{"s", ProductionSymbol::Kind::Terminal};

    // rules
    auto s_rule = GrammarRule{s_non_terminal, Production{{s_terminal}}};

    // grammar
    auto grammar = Grammar{s_rule};

    auto set_generator = FirstFollowSetGenerator(grammar);
    auto follow_set_actual = set_generator.generate_follow_sets();
    auto follow_set_correct = std::map<ProductionSymbol, std::set<ProductionSymbol>>{
        {s_non_terminal, {end_of_input}},
    };

    EXPECT_EQ(follow_set_actual, follow_set_correct);
}
