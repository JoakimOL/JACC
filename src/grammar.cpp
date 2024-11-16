#include "grammar.h"
#include <spdlog/spdlog.h>

ProductionSymbol ProductionSymbol::create_epsilon()
{
    return ProductionSymbol(std::nullopt, Kind::Terminal);
}

ProductionSymbol ProductionSymbol::create_EOI() { return ProductionSymbol("$", Kind::EndOfInput); }

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

std::optional<std::vector<Production>>
Grammar::get_rules_containing_symbol(const ProductionSymbol &p)
{
    std::vector<Production> rules_containing_symbol{};
    for (auto &rule : get_rules()) {
        auto LHS = rule.get_LHS();
        for (auto &production : rule.get_productions()) {
            auto symbols = production.get_production_symbols();
            spdlog::debug("looking for {} in {}", p, symbols);
            if (std::find(symbols.begin(), symbols.end(), p) != symbols.end()) {
                spdlog::debug("found {} in {}!", p, rule);
                rules_containing_symbol.push_back(production);
            }
        }
    }
    return rules_containing_symbol;
}

const std::optional<GrammarRule> Grammar::get_production(const ProductionSymbol &p) const
{
    for (const GrammarRule &gr : rules) {
        if (gr.get_LHS() == p) {
            return gr;
        }
    }
    return std::nullopt;
}

void PrintTo(const ProductionSymbol &p, std::ostream *out) {
    *out << fmt::format("{}", p);
}
void PrintTo(const Production &p, std::ostream *out) {
    *out << fmt::format("{}", p);
}
