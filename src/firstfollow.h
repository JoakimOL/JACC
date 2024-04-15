
#ifndef FIRSTFOLLOW_H_
#define FIRSTFOLLOW_H_

#include <map>
#include <set>

#include "grammar.h"

std::map<ProductionSymbol, std::set<ProductionSymbol>> generate_first_sets(const Grammar &grammar);

bool generate_first_set(const ProductionSymbol &LHS, const std::vector<Production> &RHS,
                        std::map<ProductionSymbol, std::set<ProductionSymbol>> &first_sets,
                        const Grammar &grammar);

bool generate_first_set(const GrammarRule &grammar_rule,
                        std::map<ProductionSymbol, std::set<ProductionSymbol>> &first_sets,
                        const Grammar &grammar);

#endif // FIRSTFOLLOW_H_
