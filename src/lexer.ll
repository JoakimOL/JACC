%{
# include <string>

# include <spdlog/spdlog.h>
# include "driver.h"
# include "parser.h"
%}

%{
%}

%option noyywrap nounput noinput batch debug

ALT "|"
SEMICOLON ";"
COLON ":"

NONTERMINAL [A-Z][A-Z0-9_\-']*
TERMINAL [^ \r\n\t:;#\/\\]+

WHITESPACE [ \r\t]*
COMMENT "\/\/"[^\r\n]*

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}


%%


%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = drv.location;
  // Code run each time yylex is called.
  loc.step ();
%}

{WHITESPACE}   loc.step ();
\n+            loc.lines (yyleng); loc.step ();
{COMMENT}      loc.lines (yyleng); loc.step ();

{ALT}          { spdlog::debug("lexed ALT");                     return yy::parser::make_ALTERNATIVE (loc);}
{COLON}        { spdlog::debug("lexed COLON");                   return yy::parser::make_COLON       (loc);}
{SEMICOLON}    { spdlog::debug("lexed SEMICOLON");               return yy::parser::make_SEMICOLON   (loc);}
{NONTERMINAL}  { spdlog::debug("lexed NONTERMINAL: {}", yytext); return yy::parser::make_NONTERMINAL (yytext, loc);}
{TERMINAL}     { spdlog::debug("lexed TERMINAL: {}", yytext);    return yy::parser::make_TERMINAL    (yytext, loc);}

<<EOF>>    return yy::parser::make_YYEOF (loc);

%%

void
Driver::scan_begin ()
{
    yy_flex_debug = trace_scanning;
    if (!(yyin = fopen (file.c_str (), "r"))){
        spdlog::error("File opening failed! Does it exist?");
        exit(EXIT_FAILURE);
    }
}

void
Driver::scan_end ()
{
    fclose(yyin);
}
