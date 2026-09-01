#include <fstream>
#include <jacc/bearpiglexer.h>
#include <libbearpig/token.h>
#include <string>
#include <vector>

namespace
{
std::string trim_whitespace(const std::string &str)
{
    const std::string whitespace = " \t\n\r\f\v";

    const auto start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }

    const auto end = str.find_last_not_of(whitespace);

    return str.substr(start, end - start + 1);
}
} // namespace

bool get_tokens_from_file(std::filesystem::path path, std::vector<bp::TokenRule> &rules)
{
    std::ifstream f{path};
    if (!f.is_open())
        return false;

    std::string line;
    while (std::getline(f, line)) {
        auto found = line.find(':');
        if (found == std::string::npos) {
            return false;
        }
        rules.emplace_back(trim_whitespace(line.substr(0, found)),
                           trim_whitespace(line.substr(found + 1)));
    }
    return true;
}

void add_skip_whitespace_rule(std::vector<bp::TokenRule> &rules)
{
    rules.emplace_back("WS", "[\\ \t\\\n]+", bp::SkipStrategy::SKIP);
}
