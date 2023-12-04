%skeleton "lalr1.cc"
%require "3.8.2"
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>
    #include <spdlog/spdlog.h>
    #include "grammar.h"
    class Driver;
}

// The parsing context.
%param { Driver& drv }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
    #include "driver.h"
}

%define api.token.prefix {TOK_}
%token
  ALTERNATIVE
  COLON
  SEMICOLON
  EPSILON
;

%token <std::string> TERMINAL
%token <std::string> NONTERMINAL

%%
%nterm <Grammar> Grammar;
Grammar : GrammarRuleList
            {
                $$ = Grammar($1);
                drv.grammar = $$;
                spdlog::debug("parsed grammar!");
            }
        ;

%nterm <std::vector<GrammarRule>> GrammarRuleList;
GrammarRuleList : GrammarRule SEMICOLON
                    {
                        $$ = {$1};
                        spdlog::debug("parsed singleton GrammarRuleList!");
                    }
                | GrammarRuleList GrammarRule SEMICOLON
                    {
                        $$ = $1; $$.emplace_back($2);
                        spdlog::debug("parsed GrammarRuleList!");
                    }
                ;

%nterm <GrammarRule> GrammarRule;
GrammarRule : LHS COLON RHS
                {
                    $$ = GrammarRule($1,$3);
                    spdlog::debug("parsed GrammarRule!");
                }
            ;

%nterm <ProductionSymbol> LHS;
LHS : NONTERMINAL
        {
            $$ = ProductionSymbol($1, ProductionSymbol::Kind::NonTerminal);
            spdlog::debug("parsed LHS!");
        }
    ;

%nterm <std::vector<Production>> RHS;
RHS : ProductionList
        { $$ = $1; spdlog::debug("parsed RHS!"); }
    ;

%nterm <std::vector<Production>> ProductionList;
ProductionList : Production
                   { $$ = {$1}; spdlog::debug("Parsed Singleton Productionlist!"); }
               | ProductionList ALTERNATIVE Production
                   {
                       $$ = $1; $$.emplace_back($3);
                       spdlog::debug("Parsed Alternative Productionlist!");
                   }
               ;

%nterm <Production> Production;
Production : SymbolList
               { $$ = Production($1); spdlog::debug("parsed Production!"); }
           ;

%nterm <std::vector<ProductionSymbol>> SymbolList;
SymbolList : Symbol
               {$$ = {$1}; spdlog::debug("parsed singleton SymbolList!"); }
           | SymbolList Symbol
               {$$ = $1; $$.emplace_back($2); spdlog::debug("parsed SymbolList!"); }
           ;

%nterm <ProductionSymbol> Symbol;
Symbol : NONTERMINAL
           {
               $$ = ProductionSymbol($1, ProductionSymbol::Kind::NonTerminal);
               spdlog::debug("parsed Nonterminal Symbol!");
           }
       | TERMINAL
           {
               $$ = ProductionSymbol($1, ProductionSymbol::Kind::Terminal);
               spdlog::debug("parsed Terminal Symbol!");
           }
       | EPSILON
           {
               $$ = ProductionSymbol::create_epsilon();
               spdlog::debug("parsed epsilon!");
           }
       ;
%%

void
yy::parser::error (const location_type& loc, const std::string& message)
{
  std::cerr << loc << ": " << message << '\n';
}
