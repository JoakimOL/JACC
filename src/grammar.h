#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include <cassert>
#include <optional>
#include <string>
#include <vector>

#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

class ProductionSymbol {
   public:
    enum class Kind { Uninitialized, NonTerminal, Terminal };
    ProductionSymbol(const std::string& symbol, Kind kind)
        : kind(kind), raw_symbol(symbol) {}

    ProductionSymbol() : kind(Kind::Uninitialized), raw_symbol("") {}

    bool isTerminal() const { return kind == Kind::Terminal; }
    bool isNonTerminal() const { return kind == Kind::NonTerminal; }
    bool isInitalized() const { return kind != Kind::Uninitialized; }
    bool isEpsilon()     const { return !raw_symbol.has_value(); }

   private:
    Kind kind;
    std::optional<std::string> raw_symbol;
    friend class fmt::formatter<ProductionSymbol>;
};

template <>
class fmt::formatter<ProductionSymbol> {
   public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }
    template <typename FmtContext>
    constexpr auto format(ProductionSymbol const& ps, FmtContext& ctx) const {
        return format_to(ctx.out(), "{}", ps.raw_symbol.value_or("epsilon"));
    }
};

/**
 * A production is a one-to-one mapping between LHS and RHS
 * A grammar rule is made of one or more productions
 */
class Production {
   public:
    Production() : RHS({}) {}
    Production(std::vector<ProductionSymbol> RHS) : RHS(RHS) {
        spdlog::info("constructing Production with RHS = {}", RHS);
    }
    Production(ProductionSymbol RHS)
        : Production(std::vector<ProductionSymbol>{RHS}) {
        spdlog::info("constructing Production with RHS = {}", RHS);
    }

   private:
    std::vector<ProductionSymbol> RHS;
    friend class fmt::formatter<Production>;
};

template <>
class fmt::formatter<Production> {
   public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }
    template <typename FmtContext>
    constexpr auto format(Production const& p, FmtContext& ctx) const {
        return format_to(ctx.out(), "{}", p.RHS);
    }
};

/**
 * A grammar rule is a LHS with one or more productions (RHS)
 */
class GrammarRule {
   public:
    GrammarRule() : LHS(), RHS({}) {}
    GrammarRule(ProductionSymbol LHS, std::vector<Production> RHSs)
        : LHS(LHS), RHS(RHSs) {}

    GrammarRule(ProductionSymbol LHS, Production RHS)
        : GrammarRule(LHS, std::vector<Production>{RHS}) {}

   private:
    ProductionSymbol LHS;
    std::vector<Production> RHS;
    friend class fmt::formatter<GrammarRule>;
};

template <>
class fmt::formatter<GrammarRule> {
   public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }
    template <typename FmtContext>
    constexpr auto format(GrammarRule const& g, FmtContext& ctx) const {
        return format_to(ctx.out(), "{} : {}", g.LHS, g.RHS);
    }
};

class Grammar {
   public:
    Grammar() : rules({}) {}
    Grammar(GrammarRule rule) : rules({rule}) {}
    Grammar(std::vector<GrammarRule> rules) : rules(rules) {}

   private:
    std::optional<std::string> grammar_string = std::nullopt;
    std::vector<GrammarRule> rules;
    friend class fmt::formatter<Grammar>;
};

template <>
class fmt::formatter<Grammar> {
   public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }
    template <typename FmtContext>
    constexpr auto format(Grammar const& g, FmtContext& ctx) const {
        return format_to(ctx.out(), "{}", g.rules);
    }
};

#endif  // GRAMMAR_H_
