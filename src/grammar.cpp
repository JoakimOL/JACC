#include "grammar.h"
#include <spdlog/spdlog.h>

ProductionSymbol ProductionSymbol::create_epsilon()
{
    return ProductionSymbol(std::nullopt, Kind::Terminal);
}

const std::optional<GrammarRule> Grammar::get_rule(const ProductionSymbol &p) const
{
    for (const GrammarRule &gr : rules) {
        if (gr.get_LHS() == p) {
            return gr;
        }
    }
    return std::nullopt;
}
bool Production::is_epsilon() const
{
    return production_symbols.size() == 1 && production_symbols.front().is_epsilon();
}

const std::vector<ProductionSymbol> &Production::get_production_symbols() const
{
    return production_symbols;
}

size_t Production::get_num_symbols() const { return production_symbols.size(); }

const bool GrammarRule::rule_contains_epsilon_production() const
{
    return std::find_if(RHS.begin(), RHS.end(), [](Production p) { return p.is_epsilon(); }) !=
           RHS.end();
}
