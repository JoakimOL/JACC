#include "first_follow_set_generator.h"
#include "grammar.h"
#include <algorithm>
#include <iterator>
#include <spdlog/spdlog.h>

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
    if (first_initialized)
        return first_sets;
    set_map<ProductionSymbol> first_sets;
    for (auto &rule : grammar.get_rules()) {
        auto LHS = rule.get_LHS();
        std::set<ProductionSymbol> current_first_set{};
        spdlog::debug("loop {}", LHS);
        auto first_set = first(LHS);
        first_sets[LHS] = first_set;
    }
    first_initialized = true;
    return first_sets;
}

std::set<ProductionSymbol> FirstFollowSetGenerator::first(const Production &p)
{
    std::set<ProductionSymbol> first_sets;
    bool all_contains_epsilon = true; // To keep track of rule 3
    for (const ProductionSymbol &symbol : p.get_production_symbols()) {

        auto first_set = first(symbol);
        bool contains_epsilon = set_contains_epsilon(first_set);
        first_sets.merge(first_set);
        if (!contains_epsilon) {
            all_contains_epsilon = false;
            break;
        }
    }
    if (!all_contains_epsilon)
        first_sets.erase(ProductionSymbol::create_epsilon());
    return first_sets;
}
std::set<ProductionSymbol> FirstFollowSetGenerator::first(const ProductionSymbol &p)
{
    spdlog::debug("{}({})", __func__, p);
    // rule 1
    if (p.is_terminal())
        return {p};
    else if (first_sets.find(p) != first_sets.end()) {
        spdlog::debug("found cached value {}. returning", first_sets[p]);
        return first_sets[p];
    }
    assert(p.is_nonTerminal());
    std::set<ProductionSymbol> first_set{};

    bool should_contain_epsilon = false; // to keep track of rule 2
    if (auto rule = grammar.get_production(p); rule.has_value()) {
        auto productions = rule.value().get_productions();
        for (const auto &production : productions) {
            if (production.is_epsilon())
                should_contain_epsilon = true;
            else if (production.get_production_symbols().front() == p)
                break;

            auto set = first(production);
            first_set.merge(set);
        }
    }

    if (!should_contain_epsilon)
        first_set.erase(ProductionSymbol::create_epsilon());
    first_sets[p] = first_set;
    return first_set;
}

// Rules for Follow Sets
// 1. First put $ (the end of input marker) in Follow(S) (S is the start symbol)
// 2. If there is a production A → aBb, (where a can be a whole string) then everything in FIRST(b)
// except for ε is placed in FOLLOW(B).
// 3. If there is a production A → aB, then everything in FOLLOW(A) is in FOLLOW(B)
// 4. If there is a production A → aBb, where FIRST(b) contains ε, then everything in FOLLOW(A) is
// in FOLLOW(B)
FirstFollowSetGenerator::set_map<ProductionSymbol> FirstFollowSetGenerator::generate_follow_sets()
{
    if (follow_initialized)
        return follow_sets;
    auto rules = grammar.get_rules();
    // assuming start symbol is the first symbol
    follow_sets[rules.front().get_LHS()] =
        std::set<ProductionSymbol>{ProductionSymbol::create_EOI()};

    bool keep_going = true;
    while (keep_going) {
        keep_going = false;

        for (auto &rule : rules) {
            auto LHS = rule.get_LHS();
            spdlog::debug("rule {}", rule);
            if (follow_sets.find(LHS) == follow_sets.end())
                follow_sets[LHS] = std::set<ProductionSymbol>{};

            auto temp = follow(LHS);
            spdlog::debug("merging {} with {}", follow_sets[LHS], temp);
            auto old_length = follow_sets[LHS].size();
            follow_sets[LHS].insert(temp.cbegin(), temp.cend());
            if (old_length != follow_sets[LHS].size()) {
                keep_going = true;
            }
        }
    }
    follow_initialized = true;
    return follow_sets;
}

std::set<ProductionSymbol> FirstFollowSetGenerator::follow(const ProductionSymbol &p)
{
    spdlog::debug("{}({})", __func__, p);
    auto follow_set = std::set<ProductionSymbol>{};
    auto productions_with_symbol = grammar.get_rules_containing_symbol(p);
    if (!productions_with_symbol.has_value()) {
        spdlog::debug("didnt find any productions with {}", p);
        return {};
    }
    for (auto &production : productions_with_symbol.value()) {
        auto LHS = production.synthesized_LHS.value();
        auto RHS = production.get_production_symbols();
        auto production_it = std::find(RHS.begin(), RHS.end(), p);
        while (production_it != RHS.end()) {
            auto next_it = production_it + 1;
            while (next_it != RHS.end()) {
                auto next_symbol = *next_it;
                auto first_of_next = first(next_symbol);
                spdlog::debug("{}:{} - current symbol: {}, next_symbol: {}. first_of_next: {}", LHS,
                              production, *production_it, next_symbol, first_of_next);

                std::copy_if(first_of_next.cbegin(), first_of_next.cend(),
                             std::inserter(follow_set, follow_set.end()),
                             [](ProductionSymbol val) { return !val.is_epsilon(); });

                if (!set_contains_epsilon(first_of_next))
                    break;
                next_it++;
            }

            if (next_it == RHS.end()) {
                follow_set.insert(follow_sets[LHS].cbegin(), follow_sets[LHS].cend());
            }
            production_it = std::find(production_it + 1, RHS.end(), p);
        }
    }
    return follow_set;
}
