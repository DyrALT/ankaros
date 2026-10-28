#include "command.h"

#include <cctype>

namespace ankaros {

bool splitCommand(std::string_view command, std::vector<std::string> &arguments, std::string &error)
{
    arguments.clear();
    std::string current;
    bool inArgument = false;
    char quote = 0;

    for (size_t i = 0; i < command.size(); ++i) {
        const char c = command[i];
        if (quote == '\'') {
            if (c == '\'')
                quote = 0;
            else
                current += c;
        } else if (c == '\\' && i + 1 < command.size()) {
            current += command[++i];
            inArgument = true;
        } else if (quote == '"') {
            if (c == '"')
                quote = 0;
            else
                current += c;
        } else if (c == '\'' || c == '"') {
            quote = c;
            inArgument = true;
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            if (inArgument) {
                arguments.push_back(current);
                current.clear();
                inArgument = false;
            }
        } else {
            current += c;
            inArgument = true;
        }
    }

    if (quote != 0) {
        error = std::string("kapanmayan tırnak: ") + quote;
        return false;
    }
    if (inArgument)
        arguments.push_back(current);
    return true;
}

} // namespace ankaros
