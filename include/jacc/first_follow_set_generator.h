#ifndef FIRST_FOLLOW_SET_GENERATOR_H_
#define FIRST_FOLLOW_SET_GENERATOR_H_

#include <jacc/grammar.h>
#include <map>
#include <set>

class FirstFollowSetGenerator
{
  public:
    explicit FirstFollowSetGenerator(const Grammar &g) : grammar(g) {};
    template <class T> using set_map = std::map<ProductionSymbol, std::set<T>>;
    std::set<ProductionSymbol> first(const Production &p);
    std::set<ProductionSymbol> first(const ProductionSymbol &p);
    std::set<ProductionSymbol> follow(const ProductionSymbol &p);
    set_map<ProductionSymbol> generate_first_sets();
    set_map<ProductionSymbol> generate_follow_sets();
    set_map<ProductionSymbol> first_sets{};
    set_map<ProductionSymbol> follow_sets{};
    Grammar grammar;

  private:
    bool first_initialized = false;
    bool follow_initialized = false;
};

#endif // FIRST_FOLLOW_SET_GENERATOR_H_
