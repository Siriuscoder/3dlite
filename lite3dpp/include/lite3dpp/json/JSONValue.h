/*
 * File JSONValue.h part of the SimpleJSON Library - http://mjpa.in/json
 *
 * Copyright (C) 2010 Mike Anchor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _JSONVALUE_H_
#define _JSONVALUE_H_

#include <vector>
#include <string>

#include <lite3dpp/json/JSON.h>

class JSON;

enum JSONType { JSONType_Null, JSONType_String, JSONType_Bool, JSONType_Number, JSONType_Array, JSONType_Object };

class JSONValue : public lite3dpp::Manageable
{
    friend class JSON;

    public:
        JSONValue(/*NULL*/);
        JSONValue(const wchar_t *m_char_value);
        JSONValue(const lite3dpp::WString &m_string_value);
        JSONValue(bool m_bool_value);
        JSONValue(float m_number_value);
        JSONValue(const JSONArray &m_array_value);
        JSONValue(const JSONObject &m_object_value);

        void setValue(/*NULL*/);
        void setValue(const wchar_t *m_char_value);
        void setValue(const lite3dpp::WString &m_string_value);
        void setValue(bool m_bool_value);
        void setValue(float m_number_value);
        void setValue(const JSONArray &m_array_value);
        void setValue(const JSONObject &m_object_value);

        bool IsNull() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsNumber() const;
        bool IsArray() const;
        bool IsObject() const;

        const lite3dpp::WString &AsString() const;
        bool AsBool() const;
        float AsNumber() const;
        int AsInt() const;
        const JSONArray &AsArray() const;
        const JSONObject &AsObject() const;

        std::size_t CountChildren() const;
        bool HasChild(std::size_t index) const;
        std::shared_ptr<JSONValue> Child(std::size_t index);
        bool HasChild(const wchar_t* name) const;
        std::shared_ptr<JSONValue> Child(const wchar_t* name);
        std::vector<lite3dpp::WString> ObjectKeys() const;

        lite3dpp::WString Stringify(bool const prettyprint = false) const;

    protected:
        static std::shared_ptr<JSONValue> Parse(const wchar_t **data);

    private:
        static lite3dpp::WString StringifyString(const lite3dpp::WString &str);
        lite3dpp::WString StringifyImpl(size_t const indentDepth) const;
        static lite3dpp::WString Indent(size_t depth);

        JSONType type;
        lite3dpp::WString string_value;
        bool bool_value;
        float number_value;
        JSONArray array_value;
        JSONObject object_value;
};

#endif
