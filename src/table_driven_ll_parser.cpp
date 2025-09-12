#include <jacc/table_driven_ll_parser.h>
#include <jacc/grammar.h>
#include <span>
#include <spdlog/spdlog.h>
#include <stack>

bool LLParser::parse(std::vector<ProductionSymbol> &input)
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
    return context.error == ParseContext::ErrorType::NOERROR;
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
        spdlog::debug("parse_table[{}][{}]:{}", top, current, parse_table[top][current]);
        if (parse_table[top][current].get_num_symbols() == 0) {
            spdlog::debug("no matching production");
            context.error = ParseContext::ErrorType::NOMATCHINGPRODUCTION;
            return;
        }
        context.parse_stack.pop();
        auto production = parse_table[top][current];
        push_production_to_stack(production);
    } else {
        spdlog::debug("terminal mismatch");
        context.error = ParseContext::ErrorType::TERMINALMISMATCH;
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
