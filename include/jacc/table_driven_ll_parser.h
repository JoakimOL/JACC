#ifndef TABLE_DRIVEN_LL_PARSER_H_
#define TABLE_DRIVEN_LL_PARSER_H_
#include <jacc/grammar.h>
#include <map>
#include <spdlog/spdlog.h>
#include <stack>
class LLParser
{
    using ParseTable = std::map<ProductionSymbol, std::map<ProductionSymbol, Production>>;

  public:
    bool parse(std::vector<ProductionSymbol> &input);
    LLParser(ParseTable table, ProductionSymbol start_symbol)
        : parse_table(table), context(start_symbol)
    {
    }
    bool done() const { return context.done; }
    void reset() { context.reset(); };

  private:
    struct ParseContext {
        enum class ErrorType {
            NOERROR,
            NOMATCHINGPRODUCTION,
            TERMINALMISMATCH,
        };
        std::string parse_error_to_string(ErrorType err)
        {
            switch (err) {
            case ErrorType::NOERROR:
                return "No Error";
            case ErrorType::NOMATCHINGPRODUCTION:
                return "No matching production";
            case ErrorType::TERMINALMISMATCH:
                return "Terminal mismatch";
            default:
                return "what the hell";
            }
        };
        ParseContext(ProductionSymbol start_symbol) : start_symbol(start_symbol) {};
        bool done = false;
        ErrorType error = ErrorType::NOERROR;
        size_t inputIndex = 0;
        std::stack<ProductionSymbol> parse_stack;
        ProductionSymbol start_symbol;
        void reset()
        {
            done = false;
            error = ErrorType::NOERROR;
            inputIndex = 0;
            parse_stack = std::stack<ProductionSymbol>();
        }
    };
    void handle_current_symbol(const ProductionSymbol &current, const ProductionSymbol &top);
    void push_production_to_stack(const Production &production);
    ParseTable parse_table;
    ParseContext context;
    ProductionSymbol eoi_symbol = ProductionSymbol::create_EOI();
};

#endif // TABLE_DRIVEN_LL_PARSER_H_
