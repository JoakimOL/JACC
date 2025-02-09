#include "driver.h"

#include "parser.h"

/*
 * Shamelessly "inspired" from GNU Bison example code
 */

Driver::Driver() : trace_parsing(false), trace_scanning(false) {}

int Driver::parse(const std::string &f)
{
    file = f;
    location.initialize(&file);
    scan_begin();
    yy::parser parse(*this);
    parse.set_debug_level(trace_parsing);
    int res = parse();
    scan_end();
    return res;
}
