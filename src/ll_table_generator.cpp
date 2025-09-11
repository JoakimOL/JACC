#include "ll_table_generator.h"
#include "first_follow_set_generator.h"
#include "grammar.h"
#include <spdlog/spdlog.h>

std::map<ProductionSymbol, std::map<ProductionSymbol, Production>>
generate_ll_table(Grammar &grammar, FirstFollowSetGenerator &sets_generator)
{
    std::map<ProductionSymbol, std::map<ProductionSymbol, Production>> parsing_table;
    for (auto &rule : grammar.get_rules()) {
        auto LHS = rule.get_LHS();
        spdlog::debug("LHS: {}", LHS);
        bool contains_epsilon = false;
        for (auto &production : rule.get_productions()) {
            auto current_first_set = sets_generator.first(production);
            spdlog::debug("production: {}", production);
            spdlog::debug("first set: {}", current_first_set);
            spdlog::debug("entering loop");
            for (auto &production_symbol : current_first_set) {
                spdlog::debug("production symbol: {}", production_symbol);
                if (production_symbol.is_epsilon()) {
                    contains_epsilon = true;
                    spdlog::debug("found epsilon!");
                } else {
                    // For each terminal `production_symbol` in `current_first_set`,
                    // add `production` to parsing_table[LHS,production_symbol]
                    parsing_table[LHS][production_symbol] = production;
                    spdlog::debug("(no epsilon) wrote parsing_table[{}][{}] = {}", LHS,
                                  production_symbol, production);
                }
            }
            spdlog::debug("exiting loop");
        }
        if (contains_epsilon) {
            auto follow_set = sets_generator.generate_follow_sets();
            auto epsilon_production = Production(ProductionSymbol::create_epsilon());
            epsilon_production.synthesized_LHS = LHS;
            auto current_follow_set = follow_set[LHS];
            spdlog::debug("follow set: {}", current_follow_set);
            for (auto &production_symbol : current_follow_set) {
                parsing_table[LHS][production_symbol] = epsilon_production;
                spdlog::debug("(epsilon) wrote parsing_table[{}][{}] = {}", LHS, production_symbol,
                              epsilon_production);
            }
        }
    }
    return parsing_table;
}

bool is_nullable(const ProductionSymbol &p, const Grammar &g)
{
    if (p.is_epsilon())
        return true;
    else if (p.is_terminal())
        return false;
    auto productions = g.get_production(p).value();
    if (productions.rule_contains_epsilon_production())
        return true;
    return std::any_of(productions.get_productions().cbegin(), productions.get_productions().cend(),
                       [g](const Production& p) { return is_nullable(p, g); });
}

bool is_nullable(const Production &p, const Grammar &g)
{
    auto symbols = p.get_production_symbols();
    return p.is_epsilon() || std::all_of(symbols.begin(), symbols.end(),
                                         [g](ProductionSymbol p) { return is_nullable(p, g); });
}
