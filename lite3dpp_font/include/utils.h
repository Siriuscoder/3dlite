#ifndef NWE_UTILS_H
#define NWE_UTILS_H

#include <string>

namespace nw
{
    std::string ReadFileFromString(const std::string& _fname);
    std::string& Replace(std::string& _where, const std::string& _what, const std::string& _to);
    std::string& LTrim(std::string& _str, const std::string& _what);
    std::string& Trim(std::string& _str, const std::string& _what);
}

#endif
