#include <jacc/table_driven_ll_parser.h>
#include <jacc/grammar.h>
#include <span>
#include <spdlog/spdlog.h>
#include <stack>

std::expected<void, ParseError> LLParser::parse(std::vector<ProductionSymbol> &input)
{
    input.push_back(eoi_symbol);
    context.parse_stack.push(eoi_symbol);
    context.parse_stack.push(context.start_symbol);

    spdlog::debug("parse_table:{}", parse_table);
    while (!context.done && context.error == ParseContext::ErrorType::NOERROR) {
        spdlog::debug("remaining inputs: ");
        auto span = std::span{input};
        spdlog::debug(span.subspan(context.inputIndex));
        spdlog::debug("parsestack: {}", context.parse_stack);
        auto top_of_stack = context.parse_stack.top();
        auto current_input = input[context.inputIndex];
        spdlog::debug("current: {}", current_input);
        handle_current_symbol(current_input, top_of_stack);
    }

    if (context.parse_stack.empty()) {
        spdlog::debug("stack empty");
        spdlog::debug("context says its complete: {}", context.done);
    } else {
        spdlog::debug("not empty");
        spdlog::debug("context says its complete: {}", context.done);
        if (context.inputIndex < input.size()) {
            auto span = std::span{input};
            spdlog::debug(span.subspan(context.inputIndex));
        }
    }
    spdlog::info("context has error: {}", context.parse_error_to_string(context.error));
    if(context.error == ParseContext::ErrorType::NOERROR){
        return {};
    }
    return std::unexpected(context.error_detail.value());
}

void LLParser::handle_current_symbol(const ProductionSymbol &current, const ProductionSymbol &top)
{
    if (top == current) {
        spdlog::debug("top of stack ('{}') matched input ('{}'). Popping '{}'", top, current,
                      context.parse_stack.top());
        context.parse_stack.pop();
        context.inputIndex++;
        if (top == eoi_symbol && context.parse_stack.empty()) {
            context.done = true;
        }
        else{
            spdlog::debug("wot {}", current.get_raw_symbol().value_or("epsilon"));
        }
    } else if (top.is_nonTerminal()) {
        // spdlog::debug("parse_table[{}]:{}", top, parse_table[top]);
        auto top_it = parse_table.find(top);
        if (top_it == parse_table.end() || !top_it->second.contains(current)) {
            std::vector<std::string> expected;
            if (top_it != parse_table.end())
                for (const auto &[symbol, production] : top_it->second)
                    expected.push_back(symbol.get_raw_symbol().value_or("epsilon"));
            context.error = ParseContext::ErrorType::NOMATCHINGPRODUCTION;
            context.error_detail = build_parse_error(current, expected);
            return;
        }
        context.parse_stack.pop();
        push_production_to_stack(top_it->second.at(current));
    } else {
        spdlog::debug("terminal mismatch");
        context.error = ParseContext::ErrorType::TERMINALMISMATCH;
        context.error_detail = build_parse_error(current, {top.get_raw_symbol().value_or("epsilon")});
    }
}

void LLParser::push_production_to_stack(const Production &production)
{
    spdlog::debug("what {}->{}", production.synthesized_LHS.value().get_raw_symbol().value_or("epsilon"), production);
    if (production.is_epsilon()) {
        spdlog::debug("pushing epsilon production to stack");
        return;
    }
    spdlog::debug("pushing {}->{} to stack in reversed order", production.synthesized_LHS.value(), production);
    for (auto it = production.get_production_symbols().rbegin();
         it != production.get_production_symbols().rend(); ++it) {
        context.parse_stack.push(*it);
    }
}

ParseError LLParser::build_parse_error(const ProductionSymbol & current, const std::vector<std::string>& expected){
    auto pos = current.get_loc();
    std::string got = pos && !pos->lexeme.empty()
                          ? fmt::format("'{}'", pos->lexeme)
                          : fmt::format("'{}'", current.get_raw_symbol().value_or("epsilon"));
    std::string message = expected.empty() ? fmt::format("unexpected {}", got)
                                           : fmt::format("unexpected {}, expected one of: {}", got,
                                                         fmt::join(expected, ", "));
    return ParseError{message, pos ? pos->offset : 0, pos ? pos->line : 0, pos ? pos->column : 0};
}
