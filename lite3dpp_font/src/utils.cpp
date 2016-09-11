#include <utils.h>
#include <iostream>
#include <fstream>

namespace nw
{
    std::string ReadFileFromString(const std::string &_fname)
    {
        std::ifstream file(_fname.c_str(), std::ios::in | std::ios::binary);
        if(file.fail()) return std::string();

        file.seekg(0, std::ios::end);
        std::ifstream::pos_type new_size = file.tellg();

        std::string ret;
        ret.resize(static_cast<unsigned int>(new_size));

        file.seekg(0, std::ios::beg);
        file.read(&ret[0], new_size);
        file.close();

        return ret;
    }

    std::string& Replace(std::string &_where, const std::string &_what, const std::string &_to)
    {
        std::string::size_type currPos = 0;

        while((currPos = _where.find(_what, currPos)) != _where.npos)
        {
            _where.replace(currPos, _what.size(), _to);
            currPos += _to.size();
        }

        return _where;
    }

    std::string& LTrim(std::string &_str, const std::string &_what)
    {
        _str.erase(0, _str.find_first_not_of(_what));
        return _str;
    }

    std::string& RTrim(std::string &_str, const std::string &_what)
    {
        _str.erase(_str.find_last_not_of(_what) + 1);
        return _str;
    }

    std::string& Trim(std::string &_str, const std::string &_what)
    {
        return LTrim(RTrim(_str, _what), _what);
    }
}

