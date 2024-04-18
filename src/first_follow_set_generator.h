#ifndef FIRST_FOLLOW_SET_GENERATOR_H_
#define FIRST_FOLLOW_SET_GENERATOR_H_

#include "grammar.h"
#include <map>
#include <set>
class FirstFollowSetGenerator
{
  public:
    explicit FirstFollowSetGenerator(const Grammar &g) : grammar(g){};
    template <class T> using set_map = std::map<ProductionSymbol, std::set<T>>;
    std::set<ProductionSymbol> first(const Production &p);
    std::set<ProductionSymbol> first(const ProductionSymbol &p);
    set_map<ProductionSymbol> generate_first_sets();
    set_map<ProductionSymbol> first_sets{};
    const Grammar grammar;
};

#endif // FIRST_FOLLOW_SET_GENERATOR_H_
