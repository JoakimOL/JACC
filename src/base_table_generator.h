#ifndef TABLE_GENERATOR_BASE_H_
#define TABLE_GENERATOR_BASE_H_

#include "first_follow_set_generator.h"
#include "grammar.h"
#include <map>

enum class TableType {
    LL,
    LR,
};

class BaseTableGenerator
{
  public:
    std::map<ProductionSymbol, std::map<ProductionSymbol, Production>>
    virtual generate_table(Grammar &grammar,
                      FirstFollowSetGenerator::set_map<ProductionSymbol> first_set,
                      FirstFollowSetGenerator::set_map<ProductionSymbol> follow_set) = 0;

    bool is_nullable(const ProductionSymbol &p) const;

  private:
    Grammar g;
    bool is_nullable(const Production &p) const;
};

#endif // TABLE_GENERATOR_BASE_H_
