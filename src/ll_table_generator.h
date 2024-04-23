#ifndef LL_TABLE_GENERATOR_H_
#define LL_TABLE_GENERATOR_H_

#include "first_follow_set_generator.h"
#include "grammar.h"

#include <map>

std::map<ProductionSymbol, std::map<ProductionSymbol, Production>>
generate_ll_table(Grammar &grammar,
                  FirstFollowSetGenerator& sets_generator);

bool is_nullable(const Production &p, const Grammar& g);
bool is_nullable(const ProductionSymbol &p, const Grammar& g);
#endif // LL_TABLE_GENERATOR_H_
