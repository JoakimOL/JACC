#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include <algorithm>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "fmt/ranges.h"
#include "fmt/format.h"
#include "fmt/base.h"

class ProductionSymbol
{
  public:
    enum class Kind { Uninitialized, NonTerminal, Terminal, EndOfInput };
    ProductionSymbol(const std::optional<std::string> &symbol, Kind kind)
        : kind(kind), raw_symbol(symbol)
    {
    }

    ProductionSymbol() : kind(Kind::Uninitialized) {}

    bool is_terminal() const { return kind == Kind::Terminal; }
    bool is_nonTerminal() const { return kind == Kind::NonTerminal; }
    bool is_initialized() const { return kind != Kind::Uninitialized; }
    bool is_epsilon() const { return !raw_symbol.has_value(); }
    bool is_EOI() const { return kind == Kind::EndOfInput; }
    const std::optional<std::string>& get_raw_symbol() const {
      return raw_symbol;
    }

    static ProductionSymbol create_epsilon();
    static ProductionSymbol create_EOI();

    bool operator<(const ProductionSymbol &other) const
    {
        return this->raw_symbol < other.raw_symbol;
    }
    bool operator==(const ProductionSymbol &other) const
    {
        return this->raw_symbol == other.raw_symbol && this->kind == other.kind;
    }
    bool operator!=(const ProductionSymbol &other) const { return !(*this == other); }

  private:
    Kind kind;
    std::optional<std::string> raw_symbol;
    friend struct fmt::formatter<ProductionSymbol>;
};

template <> struct fmt::formatter<ProductionSymbol> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FormatContext>
    auto format(const ProductionSymbol & ps, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", ps.raw_symbol.value_or("epsilon"));
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

    explicit Production(std::vector<ProductionSymbol> RHS) : production_symbols(RHS) {}

    explicit Production(ProductionSymbol RHS) : Production(std::vector<ProductionSymbol>{RHS}) {}

    /**
     * this function looks for productions that look like this
     * A : ε;
     */
    bool is_epsilon() const;

    const std::vector<ProductionSymbol> &get_production_symbols() const;

    size_t get_num_symbols() const;

    std::optional<ProductionSymbol> synthesized_LHS;

    bool operator==(const Production &other) const
    {
        auto num_symbols = get_num_symbols();
        if (num_symbols != other.get_num_symbols())
            return false;
        for (size_t i = 0; i < num_symbols; i++) {
            if (production_symbols[i] != other.production_symbols[i])
                return false;
        }
        return true;
    }

  private:
    std::vector<ProductionSymbol> production_symbols;
    friend struct fmt::formatter<Production>;
};

template <>
struct fmt::formatter<Production> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FormatContext>
    auto format(const Production& p, FormatContext& ctx) const {
        std::string rhs_str = std::accumulate(
            p.get_production_symbols().begin(),
            p.get_production_symbols().end(),
            std::string{},
            [](const std::string& lhs, const ProductionSymbol& rhs){
                return lhs.empty() ? fmt::format("{}", rhs) : fmt::format("{} {}", lhs, rhs);
            }
        );
        
        // return fmt::format_to(ctx.out(), "{} -> {}", p.synthesized_LHS.has_value() ? p.synthesized_LHS->get_raw_symbol().value() : "ε", rhs_str);
        return format_to(ctx.out(), "{}", rhs_str);
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
    explicit GrammarRule(ProductionSymbol LHS, std::vector<Production> RHSs) : LHS(LHS), RHS(RHSs)
    {
        std::for_each(RHS.begin(), RHS.end(), [LHS](Production &p) { p.synthesized_LHS = LHS; });
    }

    explicit GrammarRule(ProductionSymbol LHS, Production RHS)
        : GrammarRule(LHS, std::vector<Production>{RHS})
    {
    }

    const ProductionSymbol &get_LHS() const { return LHS; }
    const std::vector<Production> &get_productions() const { return RHS; }
    const bool rule_contains_epsilon_production() const;

  private:
    ProductionSymbol LHS;
    std::vector<Production> RHS;
    friend class fmt::formatter<GrammarRule>;
};


template <>
struct fmt::formatter<GrammarRule> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FormatContext>
    auto format(const GrammarRule& gr, FormatContext& ctx) const {
        std::string rhs_str = std::accumulate(
            gr.get_productions().begin(),
            gr.get_productions().end(),
            std::string{},
            [](const std::string& lhs, const Production& rhs){
                return lhs.empty() ? fmt::format("{}", rhs) : fmt::format("{} | {}", lhs, rhs);
            }
        );
        
        return fmt::format_to(ctx.out(), "{} -> {}", gr.get_LHS(), rhs_str);
    }
};

class Grammar
{
  public:
    Grammar() : rules({}) {}
    explicit Grammar(GrammarRule rule) : rules({rule}) {}
    explicit Grammar(std::vector<GrammarRule> rules) : rules(rules) {}

    const std::vector<GrammarRule> &get_rules() const { return rules; }
    const std::optional<GrammarRule> get_production(const ProductionSymbol &p) const;
    std::optional<std::vector<Production>> get_rules_containing_symbol(const ProductionSymbol &p);

  private:
    std::optional<std::string> grammar_string = std::nullopt;
    std::vector<GrammarRule> rules;
    friend class fmt::formatter<Grammar>;
};

template <>
struct fmt::formatter<Grammar> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.end(); }
    template <typename FormatContext>
    auto format(const Grammar& g, FormatContext& ctx) const {
        std::string rules_str = std::accumulate(
            g.get_rules().begin(),
            g.get_rules().end(),
            std::string{},
            [](const std::string& lhs, const GrammarRule& rhs){
                return lhs.empty() ? fmt::format("{};", rhs) : fmt::format("{} {};", lhs, rhs);
            }
        );
        return fmt::format_to(ctx.out(), "{}", rules_str);
    }
};

#endif // GRAMMAR_H_
