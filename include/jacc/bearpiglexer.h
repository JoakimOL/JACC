#ifndef BEARPIGLEXER_H__
#define BEARPIGLEXER_H__

#include <filesystem>
#include <libbearpig/lexer.h>


bool get_tokens_from_file(std::filesystem::path path, std::vector<bp::TokenRule> &rules);
void add_skip_whitespace_rule(std::vector<bp::TokenRule> &rules);

#endif // BEARPIGLEXER_H__
