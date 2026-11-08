#include "command.h"

#include <cctype>

namespace ankaros {

std::string stripControlCharacters(std::string_view text)
{
    std::string result;
    for (size_t i = 0; i < text.size(); ++i) {
        const auto c = static_cast<unsigned char>(text[i]);
        if (c < 0x20 || c == 0x7f) {
            result += '?';
        } else if (c == 0xC2 && i + 1 < text.size() &&
                   (static_cast<unsigned char>(text[i + 1]) & 0xE0) == 0x80) {
            result += '?'; // C2 80 - C2 9F: tek bir C1 karakteri
            ++i;
        } else {
            result += text[i];
        }
    }
    return result;
}

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
