#include <jacc/first_follow_set_generator.h>
#include <jacc/grammar.h>
#include <jacc/ll_table_generator.h>
#include <fmt/core.h>
#include <gtest/gtest.h>
// #include <spdlog/spdlog.h>

// TEST(TableGeneration, YeahIDunno) {

//     // non terminals
//     auto s = ProductionSymbol{"S", ProductionSymbol::Kind::NonTerminal};
//     auto l = ProductionSymbol{"L", ProductionSymbol::Kind::NonTerminal};
//     auto lp = ProductionSymbol{"L'", ProductionSymbol::Kind::NonTerminal};

//     // terminals
//     auto epsilon = ProductionSymbol::create_epsilon();
//     auto eoi = ProductionSymbol::create_EOI();
//     auto a = ProductionSymbol{"a", ProductionSymbol::Kind::Terminal};
//     auto lparen = ProductionSymbol{"(", ProductionSymbol::Kind::Terminal};
//     auto rparen = ProductionSymbol{")", ProductionSymbol::Kind::Terminal};

//     // rules
//     auto s_rule = GrammarRule{s, {Production{{lparen, l, rparen}}, Production{a}}};
//     auto l_rule = GrammarRule{l, Production{{s,lp}}};
//     auto lp_rule = GrammarRule{lp, {Production{{rparen, s, lp}}, Production{epsilon}}};

//     auto grammar = Grammar{{s_rule, l_rule, lp_rule}};

//     auto set_generator = FirstFollowSetGenerator(grammar);
//     auto first_set = set_generator.generate_first_sets();
//     auto follow_set = set_generator.generate_follow_sets();
//     spdlog::info("first set: {}", first_set);
//     spdlog::info("follow set: {}", follow_set);

//     auto parse_table_actual = generate_ll_table(grammar, set_generator);
//     spdlog::info("parse_table: {}", parse_table_actual);
// }

TEST(TableGeneration, AbleToGenerateLLParseTableForExpressionGrammar)
{

    // non terminals
    auto e = ProductionSymbol{"E", ProductionSymbol::Kind::NonTerminal};
    auto ep = ProductionSymbol{"E'", ProductionSymbol::Kind::NonTerminal};
    auto t = ProductionSymbol{"T", ProductionSymbol::Kind::NonTerminal};
    auto tp = ProductionSymbol{"T'", ProductionSymbol::Kind::NonTerminal};
    auto f = ProductionSymbol{"F", ProductionSymbol::Kind::NonTerminal};

    // terminals
    auto epsilon = ProductionSymbol::create_epsilon();
    auto eoi = ProductionSymbol::create_EOI();
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

    auto parse_table_actual = generate_ll_table(grammar, set_generator);
    auto parse_table_correct = std::map<ProductionSymbol, std::map<ProductionSymbol, Production>>{
        {e, std::map<ProductionSymbol, Production>{{lparen, Production{{t, ep}}},
                                                   {id, Production{{t, ep}}}}},
        {ep, std::map<ProductionSymbol, Production>{{plus, Production{{plus, t, ep}}},
                                                    {rparen, Production{epsilon}},
                                                    {eoi, Production{epsilon}}}},
        {t, std::map<ProductionSymbol, Production>{{lparen, Production{{f, tp}}},
                                                   {id, Production{{f, tp}}}}},
        {tp, std::map<ProductionSymbol, Production>{{plus, Production{epsilon}},
                                                    {times, Production{{times, f, tp}}},
                                                    {rparen, Production{epsilon}},
                                                    {eoi, Production{epsilon}}}},
        {f, std::map<ProductionSymbol, Production>{{lparen, Production{{lparen, e, rparen}}},
                                                   {id, Production{id}}}},
    };

    EXPECT_EQ(parse_table_actual, parse_table_correct);
}
TEST(TableGeneration, GrammarsCanIdentifyNullableNonTerminals)
{
    auto a = ProductionSymbol{"A", ProductionSymbol::Kind::NonTerminal};
    auto b = ProductionSymbol{"B", ProductionSymbol::Kind::NonTerminal};
    auto c = ProductionSymbol{"C", ProductionSymbol::Kind::NonTerminal};

    // terminals
    auto epsilon = ProductionSymbol::create_epsilon();
    auto foo = ProductionSymbol{"foo", ProductionSymbol::Kind::Terminal};

    // rules
    auto a_rule = GrammarRule{a, Production{epsilon}};
    auto b_rule = GrammarRule{b, {Production{{foo}}, Production{epsilon}}};
    auto c_rule = GrammarRule{c, {Production{foo}}};
    auto indirectly_nullable1 = GrammarRule{a, {Production{b}}};
    auto indirectly_nullable2 = GrammarRule{b, {Production{epsilon}}};

    // grammar
    auto directly_nullable_grammar = Grammar{{a_rule}};
    auto nullable_grammar = Grammar{{a_rule, b_rule}};
    auto non_nullable_grammar = Grammar{{c_rule}};
    auto indirectly_nullable_grammar = Grammar{{indirectly_nullable1, indirectly_nullable2}};

    FirstFollowSetGenerator sets_generator(directly_nullable_grammar);
    FirstFollowSetGenerator::set_map<ProductionSymbol> first_sets =
        sets_generator.generate_first_sets();
    FirstFollowSetGenerator::set_map<ProductionSymbol> follow_sets =
        sets_generator.generate_follow_sets();

    EXPECT_TRUE(is_nullable(a, directly_nullable_grammar));

    EXPECT_TRUE(is_nullable(a, nullable_grammar));
    EXPECT_TRUE(is_nullable(b, nullable_grammar));
    EXPECT_FALSE(is_nullable(c, non_nullable_grammar));
    EXPECT_TRUE(is_nullable(a, indirectly_nullable_grammar));
    EXPECT_TRUE(is_nullable(b, indirectly_nullable_grammar));
}
