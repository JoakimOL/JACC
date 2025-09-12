#ifndef DRIVER_HH
#define DRIVER_HH
#include <string>

#include <jacc/grammar.h>
#include "parser.h"

/*
 * Shamelessly "inspired" by GNU Bison example code
 */

// magic that makes both the lexer and parser happy
#define YY_DECL yy::parser::symbol_type yylex(Driver &drv)
YY_DECL;

class Driver
{
  public:
    Driver();

    int parse(const std::string &f);

    std::string file;
    // Whether to generate parser debug traces.
    bool trace_parsing;

    // Handling the scanner.
    void scan_begin();
    void scan_end();
    // Whether to generate scanner debug traces.
    bool trace_scanning;
    // The token's location used by the scanner.
    yy::location location;

    Grammar grammar;
};
#endif // DRIVER_HH
