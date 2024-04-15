#include "firstfollow.h"
#include <functional>

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

void pretty_print_first_set(std::map<ProductionSymbol, std::set<ProductionSymbol>> first_sets)
{
    spdlog::info(first_sets);
}

bool generate_first_set(const ProductionSymbol &LHS, const std::vector<Production> &RHS,
                        std::map<ProductionSymbol, std::set<ProductionSymbol>> &first_sets,
                        const Grammar &grammar)
{
    spdlog::info("{}:{} :: {}", LHS, RHS, __PRETTY_FUNCTION__);
    assert(LHS.is_nonTerminal());
    bool changed = false;
    bool should_contain_epsilon = false;
    for (auto line : RHS) {
        if (line.get_num_symbols() == 1 && line.contains_epsilon()) {
            // rule 2
            first_sets[LHS].emplace(ProductionSymbol::create_epsilon());
            should_contain_epsilon = true;
        } else if (auto front = line.get_production_symbols().front(); front.is_terminal()) {
            auto res = first_sets[LHS].emplace(front);
            if (res.second)
                changed = true;
        } else {
            for (const ProductionSymbol &symbol : line.get_production_symbols()) {
                /**
                 * A : B b c
                 *   | C b c
                 * B : w
                 * C : d
                 *   | epsilon
                 */
                if (symbol.is_nonTerminal()) {
                    auto first_rhs = first_sets[symbol];
                    auto old = first_sets[LHS];
                    bool contains_epsilon =
                        first_rhs.find(ProductionSymbol::create_epsilon()) != first_rhs.end();
                    first_sets[LHS].merge(first_sets[symbol]);
                    changed |= (old != first_sets[LHS]);
                    if (!contains_epsilon)
                        break;
                } else if (symbol.is_terminal() && !symbol.is_epsilon()) {
                    auto res = first_sets[LHS].emplace(symbol);
                    if (res.second)
                        changed = true;
                    break;
                }
            }

            if (!should_contain_epsilon)
                first_sets[LHS].erase(ProductionSymbol::create_epsilon());
        }
    }
    return changed;
}

bool generate_first_set(const GrammarRule &grammar_rule,
                        std::map<ProductionSymbol, std::set<ProductionSymbol>> &first_sets,
                        const Grammar &grammar)
{
    spdlog::info("{}::{}", grammar_rule, __PRETTY_FUNCTION__);
    auto productions = grammar_rule.get_productions();
    auto LHS = grammar_rule.get_LHS();
    bool changed = false;
    auto all_productions = std::vector<Production>{};
    for (auto production : productions) {
        if (LHS.is_nonTerminal() && first_sets.find(LHS) == first_sets.end()) {
            first_sets[LHS] = std::set<ProductionSymbol>{};
            changed = true;
        }
        all_productions.emplace_back(production);
    }
    changed |= generate_first_set(LHS, all_productions, first_sets, grammar);
    return changed;
}

std::map<ProductionSymbol, std::set<ProductionSymbol>> generate_first_sets(const Grammar &grammar)
{
    auto rules = grammar.get_rules();
    bool changed = true;
    std::map<ProductionSymbol, std::set<ProductionSymbol>> first_sets;
    int i = 0;
    while (changed) {
        spdlog::info("{}", i);
        for (auto rule : rules) {
            changed = generate_first_set(rule, first_sets, grammar);
        }
        i++;
    }
    pretty_print_first_set(first_sets);
    return first_sets;
}

