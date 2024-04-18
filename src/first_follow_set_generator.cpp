#include "first_follow_set_generator.h"

namespace
{
bool set_contains_epsilon(const std::set<ProductionSymbol> &set)
{
    return set.find(ProductionSymbol::create_epsilon()) != set.end();
}

} // namespace


/**
 * 1. If X is a terminal then First(X) is just X!
 * 2. If there is a Production X → ε then add ε to first(X)
 * 3. If there is a Production X → Y1Y2..Yk then add first(Y1Y2..Yk) to first(X)
 * 4. First(Y1Y2..Yk) is either
 *  - First(Y1) (if First(Y1) doesn't contain ε)
 *    OR (if First(Y1) does contain ε) then First (Y1Y2..Yk) is everything in
 * First(Y1) <except for ε > as well as everything in First(Y2..Yk)
 *  - If First(Y1) First(Y2)..First(Yk) all contain ε then add ε to First(Y1Y2..Yk) as well.
 */
FirstFollowSetGenerator::set_map<ProductionSymbol> FirstFollowSetGenerator::generate_first_sets()
{
    set_map<ProductionSymbol> first_sets;
    for (auto &rule : grammar.get_rules()) {
        auto LHS = rule.get_LHS();
        std::set<ProductionSymbol> current_first_set{};
        spdlog::debug("loop {}", LHS);
        auto first_set = first(LHS);
        first_sets[LHS] = first_set;
    }
    return first_sets;
}

std::set<ProductionSymbol> FirstFollowSetGenerator::first(const Production &p)
{
    std::set<ProductionSymbol> first_sets;
    for (const ProductionSymbol &symbol : p.get_production_symbols()) {

        // rule 1
        if (symbol.is_terminal()) {
            first_sets.emplace(symbol);
            break;
        } else if (symbol.is_nonTerminal()) {
            auto first_set = first(symbol);
            bool contains_epsilon = set_contains_epsilon(first_set);
            first_sets.merge(first_set);
            if (!contains_epsilon) {
                break;
            }
        }
    }
    return first_sets;
}
std::set<ProductionSymbol> FirstFollowSetGenerator::first(const ProductionSymbol &p)
{
    spdlog::debug("{}({})", __func__, p);
    if (p.is_terminal())
        return {p};
    else if (first_sets.find(p) != first_sets.end()) {
        spdlog::debug("found cached value {}. returning", first_sets[p]);
        return first_sets[p];
    }
    assert(p.is_nonTerminal());
    std::set<ProductionSymbol> first_set{};

    bool all_contains_epsilon = true;    // To keep track of rule 3
    bool should_contain_epsilon = false; // to keep track of rule 2
    if (auto rule = grammar.get_rule(p); rule.has_value()) {
        auto productions = rule.value().get_productions();
        for (const auto &p : productions) {
            spdlog::debug("first({})", p);
            if (p.is_epsilon())
                should_contain_epsilon = true;

            auto set = first(p);
            if (set_contains_epsilon(set))
                all_contains_epsilon = false;
            spdlog::debug("about to merge {} with {}", set, first_set);
            first_set.merge(set);
        }
    }

    if (!should_contain_epsilon && !all_contains_epsilon) {
        spdlog::debug("Not all sets included epsilon. removing");
        first_set.erase(ProductionSymbol::create_epsilon());
    }
    spdlog::debug("about to return {} ", first_set);
    first_sets[p] = first_set;
    return first_set;
}
