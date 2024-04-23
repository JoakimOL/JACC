#include "base_table_generator.h"
#include "grammar.h"
#include <algorithm>

bool BaseTableGenerator::is_nullable(const ProductionSymbol &p) const
{
    if (p.is_epsilon())
        return true;
    else if (p.is_terminal())
        return false;
    auto productions = g.get_production(p).value();
    if (productions.rule_contains_epsilon_production())
        return true;
    return std::any_of(productions.get_productions().begin(), productions.get_productions().end(),
                       [this](Production p) { return is_nullable(p); });
}

bool BaseTableGenerator::is_nullable(const Production &p) const
{
    auto symbols = p.get_production_symbols();
    return p.is_epsilon() || std::all_of(symbols.begin(), symbols.end(),
                                         [this](ProductionSymbol p) { return is_nullable(p); });
}
