#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

class ProductionSymbol
{
  public:
    enum class Kind { Uninitialized, NonTerminal, Terminal };
    ProductionSymbol(const std::optional<std::string> &symbol, Kind kind)
        : kind(kind), raw_symbol(symbol)
    {
    }

    ProductionSymbol() : kind(Kind::Uninitialized) {}

    bool is_terminal() const { return kind == Kind::Terminal; }
    bool is_nonTerminal() const { return kind == Kind::NonTerminal; }
    bool is_initialized() const { return kind != Kind::Uninitialized; }
    bool is_epsilon() const { return !raw_symbol.has_value(); }

    static ProductionSymbol create_epsilon()
    {
        return ProductionSymbol(std::nullopt, Kind::Terminal);
    }

    bool operator<(const ProductionSymbol &other) const
    {
        return this->raw_symbol < other.raw_symbol;
    }
    bool operator==(const ProductionSymbol &other) const
    {
        return this->raw_symbol == other.raw_symbol && this->kind == other.kind;
    }

  private:
    Kind kind;
    std::optional<std::string> raw_symbol;
    friend class fmt::formatter<ProductionSymbol>;
    friend void PrintTo(const ProductionSymbol &p, std::ostream *out)
    {
        *out << fmt::format("{}", p);
    }
};

template <> class fmt::formatter<ProductionSymbol>
{
  public:
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FmtContext>
    constexpr auto format(ProductionSymbol const &ps, FmtContext &ctx) const
    {
        return format_to(ctx.out(), "{}", ps.raw_symbol.value_or("epsilon"));
    }
};

/**
 * A production is a one-to-one mapping between LHS and RHS
 * A grammar rule is made of one or more productions
 *
 * A : B b <- only this line
 *   | C c; <- or only this line
 */
class Production
{
  public:
    Production() : production_symbols({}) {}

    Production(std::vector<ProductionSymbol> RHS) : production_symbols(RHS)
    {
        spdlog::debug("constructing Production with RHS = {}", RHS);
    }

    Production(ProductionSymbol RHS) : Production(std::vector<ProductionSymbol>{RHS})
    {
        spdlog::debug("constructing Production with RHS = {}", RHS);
    }

    /**
     * this function looks for productions that look like this
     * A : a
     *   | ε;
     */
    bool contains_epsilon() const
    {
        return (std::find_if(production_symbols.cbegin(), production_symbols.cend(),
                             [](ProductionSymbol p) { return p.is_epsilon(); }) !=
                std::end(production_symbols));
    }
    /**
     * this function looks for productions that look like this
     * A : ε;
     */
    bool is_epsilon() const
    {
        return production_symbols.size() == 1 && production_symbols.front().is_epsilon();
    }

    const std::vector<ProductionSymbol> &get_production_symbols() const
    {
        return production_symbols;
    }

    size_t get_num_symbols() const { return production_symbols.size(); }

  private:
    std::vector<ProductionSymbol> production_symbols;
    friend class fmt::formatter<Production>;
};

template <> class fmt::formatter<Production>
{
  public:
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FmtContext> constexpr auto format(Production const &p, FmtContext &ctx) const
    {
        return format_to(ctx.out(), "{}", p.production_symbols);
    }
};

/**
 * A grammar rule is a ProductionSymbol (LHS) with one or more productions (RHS)
 * A : B b
 *   | C c;
 */
class GrammarRule
{
  public:
    GrammarRule() : LHS(), RHS({}) {}
    GrammarRule(ProductionSymbol LHS, std::vector<Production> RHSs) : LHS(LHS), RHS(RHSs) {}

    GrammarRule(ProductionSymbol LHS, Production RHS)
        : GrammarRule(LHS, std::vector<Production>{RHS})
    {
    }

    const ProductionSymbol &get_LHS() const { return LHS; }
    const std::vector<Production> &get_productions() const { return RHS; }

  private:
    ProductionSymbol LHS;
    std::vector<Production> RHS;
    friend class fmt::formatter<GrammarRule>;
};

template <> class fmt::formatter<GrammarRule>
{
  public:
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FmtContext>
    constexpr auto format(GrammarRule const &g, FmtContext &ctx) const
    {
        return format_to(ctx.out(), "{} : {}", g.LHS, g.RHS);
    }
};

class Grammar
{
  public:
    template <class T> using set_map = std::map<ProductionSymbol, std::set<T>>;
    Grammar() : rules({}) {}
    Grammar(GrammarRule rule) : rules({rule}) {}
    Grammar(std::vector<GrammarRule> rules) : rules(rules) {}

    const std::vector<GrammarRule> &get_rules() const { return rules; }
    const std::optional<GrammarRule> get_rule(const ProductionSymbol &p) const;
    Grammar::set_map<ProductionSymbol> generate_first_sets();

  private:
    std::set<ProductionSymbol> first(const ProductionSymbol &p);
    std::set<ProductionSymbol> first(const Production &p);
    std::optional<std::string> grammar_string = std::nullopt;
    std::vector<GrammarRule> rules;
    set_map<ProductionSymbol> first_sets;
    friend class fmt::formatter<Grammar>;
};

template <> class fmt::formatter<Grammar>
{
  public:
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FmtContext> constexpr auto format(Grammar const &g, FmtContext &ctx) const
    {
        return format_to(ctx.out(), "{}", g.rules);
    }
};

#endif // GRAMMAR_H_
