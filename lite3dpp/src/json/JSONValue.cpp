/*
 * File JSONValue.cpp part of the SimpleJSON Library - http://mjpa.in/json
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <math.h>

#include <lite3dpp/json/JSONValue.h>

/**
 * Parses a JSON encoded value to a JSONValue object
 *
 * @access protected
 *
 * @param wchar_t** data Pointer to a wchar_t* that contains the data
 *
 * @return JSONValue* Returns a pointer to a JSONValue object on success, NULL on error
 */
std::shared_ptr<JSONValue> JSONValue::Parse(const wchar_t **data)
{
    // Is it a string?
    if (**data == '"')
    {
        lite3dpp::WString str;
        if (!JSON::ExtractString(&(++(*data)), str))
            return NULL;
        else
            return std::make_shared<JSONValue>(str);
    }

    // Is it a boolean?
    else if ((simplejson_wcsnlen(*data, 4) && wcsncasecmp(*data, L"true", 4) == 0) || (simplejson_wcsnlen(*data, 5) && wcsncasecmp(*data, L"false", 5) == 0))
    {
        bool value = wcsncasecmp(*data, L"true", 4) == 0;
        (*data) += value ? 4 : 5;
        return std::make_shared<JSONValue>(value);
    }

    // Is it a null?
    else if (simplejson_wcsnlen(*data, 4) && wcsncasecmp(*data, L"null", 4) == 0)
    {
        (*data) += 4;
        return std::make_shared<JSONValue>();
    }

    // Is it a number?
    else if (**data == L'-' || (**data >= L'0' && **data <= L'9'))
    {
        // Negative?
        bool neg = **data == L'-';
        if (neg) (*data)++;

        float number = 0.0;

        // Parse the whole part of the number - only if it wasn't 0
        if (**data == L'0')
            (*data)++;
        else if (**data >= L'1' && **data <= L'9')
            number = JSON::ParseInt(data);
        else
            return std::shared_ptr<JSONValue>();

        // Could be a decimal now...
        if (**data == '.')
        {
            (*data)++;

            // Not get any digits?
            if (!(**data >= L'0' && **data <= L'9'))
                return std::shared_ptr<JSONValue>();

            // Find the decimal and sort the decimal place out
            // Use ParseDecimal as ParseInt won't work with decimals less than 0.1
            // thanks to Javier Abadia for the report & fix
            float decimal = JSON::ParseDecimal(data);

            // Save the number
            number += decimal;
        }

        // Could be an exponent now...
        if (**data == L'E' || **data == L'e')
        {
            (*data)++;

            // Check signage of expo
            bool neg_expo = false;
            if (**data == L'-' || **data == L'+')
            {
                neg_expo = **data == L'-';
                (*data)++;
            }

            // Not get any digits?
            if (!(**data >= L'0' && **data <= L'9'))
                return std::shared_ptr<JSONValue>();

            // Sort the expo out
            float expo = JSON::ParseInt(data);
            for (float i = 0.0; i < expo; i++)
                number = neg_expo ? (number / 10.0) : (number * 10.0);
        }

        // Was it neg?
        if (neg) number *= -1;

        return std::make_shared<JSONValue>(number);
    }

    // An object?
    else if (**data == L'{')
    {
        JSONObject object;

        (*data)++;

        while (**data != 0)
        {
            // Whitespace at the start?
            if (!JSON::SkipWhitespaceAndComments(data))
            {
                return std::shared_ptr<JSONValue>();
            }

            // Special case - empty object
            if (object.size() == 0 && **data == L'}')
            {
                (*data)++;
                return std::make_shared<JSONValue>(object);
            }

            // We want a string now...
            lite3dpp::WString name;
            if (!JSON::ExtractString(&(++(*data)), name))
            {
                return std::shared_ptr<JSONValue>();
            }

            // More whitespace?
            if (!JSON::SkipWhitespaceAndComments(data))
            {
                return std::shared_ptr<JSONValue>();
            }

            // Need a : now
            if (*((*data)++) != L':')
            {
                return std::shared_ptr<JSONValue>();
            }

            // More whitespace?
            if (!JSON::SkipWhitespaceAndComments(data))
            {
                return std::shared_ptr<JSONValue>();
            }

            // The value is here
            std::shared_ptr<JSONValue> value = Parse(data);
            if (!value)
            {
                return value;
            }

            object[name] = value;

            // More whitespace?
            if (!JSON::SkipWhitespaceAndComments(data))
            {
                return std::shared_ptr<JSONValue>();
            }

            // End of object?
            if (**data == L'}')
            {
                (*data)++;
                return std::make_shared<JSONValue>(object);
            }

            // Want a , now
            if (**data != L',')
            {
                return std::shared_ptr<JSONValue>();
            }

            (*data)++;
        }

        // Only here if we ran out of data
        return std::shared_ptr<JSONValue>();
    }

    // An array?
    else if (**data == L'[')
    {
        JSONArray array;

        (*data)++;

        while (**data != 0)
        {
            // Whitespace at the start?
            if (!JSON::SkipWhitespaceAndComments(data))
            {
                return std::shared_ptr<JSONValue>();
            }

            // Special case - empty array
            if (array.size() == 0 && **data == L']')
            {
                (*data)++;
                return std::make_shared<JSONValue>(array);
            }

            // Get the value
            std::shared_ptr<JSONValue> value = Parse(data);
            if (!value)
            {
                return value;
            }

            // Add the value
            array.emplace_back(value);

            // More whitespace?
            if (!JSON::SkipWhitespaceAndComments(data))
            {
                return std::shared_ptr<JSONValue>();
            }

            // End of array?
            if (**data == L']')
            {
                (*data)++;
                return std::make_shared<JSONValue>(array);
            }

            // Want a , now
            if (**data != L',')
            {
                return std::shared_ptr<JSONValue>();
            }

            (*data)++;
        }

        // Only here if we ran out of data
        return std::shared_ptr<JSONValue>();
    }

    // Ran out of possibilites, it's bad!
    else
    {
        return std::shared_ptr<JSONValue>();
    }
}

void JSONValue::setValue(/*NULL*/)
{
    type = JSONType_Null;
}

void JSONValue::setValue(const wchar_t *m_char_value)
{
    type = JSONType_String;
    string_value = lite3dpp::WString(m_char_value);
}

void JSONValue::setValue(const lite3dpp::WString &m_string_value)
{
    type = JSONType_String;
    string_value = m_string_value;
}

void JSONValue::setValue(bool m_bool_value)
{
    type = JSONType_Bool;
    bool_value = m_bool_value;
}

void JSONValue::setValue(float m_number_value)
{
    type = JSONType_Number;
    number_value = m_number_value;
}

void JSONValue::setValue(const JSONArray &m_array_value)
{
    type = JSONType_Array;
    array_value = m_array_value;
}

void JSONValue::setValue(const JSONObject &m_object_value)
{
    type = JSONType_Object;
    object_value = m_object_value;
}


/**
 * Basic constructor for creating a JSON Value of type NULL
 *
 * @access public
 */
JSONValue::JSONValue(/*NULL*/)
{
    setValue();
}

/**
 * Basic constructor for creating a JSON Value of type String
 *
 * @access public
 *
 * @param wchar_t* m_char_value The string to use as the value
 */
JSONValue::JSONValue(const wchar_t *m_char_value)
{
    setValue(m_char_value);
}

/**
 * Basic constructor for creating a JSON Value of type String
 *
 * @access public
 *
 * @param lite3dpp::lited3dpp_wstring m_string_value The string to use as the value
 */
JSONValue::JSONValue(const lite3dpp::WString &m_string_value)
{
    setValue(m_string_value);
}

/**
 * Basic constructor for creating a JSON Value of type Bool
 *
 * @access public
 *
 * @param bool m_bool_value The bool to use as the value
 */
JSONValue::JSONValue(bool m_bool_value)
{
    setValue(m_bool_value);
}

/**
 * Basic constructor for creating a JSON Value of type Number
 *
 * @access public
 *
 * @param float m_number_value The number to use as the value
 */
JSONValue::JSONValue(float m_number_value)
{
    setValue(m_number_value);
}

/**
 * Basic constructor for creating a JSON Value of type Array
 *
 * @access public
 *
 * @param JSONArray m_array_value The JSONArray to use as the value
 */
JSONValue::JSONValue(const JSONArray &m_array_value)
{
    setValue(m_array_value);
}

/**
 * Basic constructor for creating a JSON Value of type Object
 *
 * @access public
 *
 * @param JSONObject m_object_value The JSONObject to use as the value
 */
JSONValue::JSONValue(const JSONObject &m_object_value)
{
    setValue(m_object_value);
}

/**
 * Checks if the value is a NULL
 *
 * @access public
 *
 * @return bool Returns true if it is a NULL value, false otherwise
 */
bool JSONValue::IsNull() const
{
    return type == JSONType_Null;
}

/**
 * Checks if the value is a String
 *
 * @access public
 *
 * @return bool Returns true if it is a String value, false otherwise
 */
bool JSONValue::IsString() const
{
    return type == JSONType_String;
}

/**
 * Checks if the value is a Bool
 *
 * @access public
 *
 * @return bool Returns true if it is a Bool value, false otherwise
 */
bool JSONValue::IsBool() const
{
    return type == JSONType_Bool;
}

/**
 * Checks if the value is a Number
 *
 * @access public
 *
 * @return bool Returns true if it is a Number value, false otherwise
 */
bool JSONValue::IsNumber() const
{
    return type == JSONType_Number;
}

/**
 * Checks if the value is an Array
 *
 * @access public
 *
 * @return bool Returns true if it is an Array value, false otherwise
 */
bool JSONValue::IsArray() const
{
    return type == JSONType_Array;
}

/**
 * Checks if the value is an Object
 *
 * @access public
 *
 * @return bool Returns true if it is an Object value, false otherwise
 */
bool JSONValue::IsObject() const
{
    return type == JSONType_Object;
}

/**
 * Retrieves the String value of this JSONValue
 * Use IsString() before using this method.
 *
 * @access public
 *
 * @return lite3dpp::lited3dpp_wstring Returns the string value
 */
const lite3dpp::WString &JSONValue::AsString() const
{
    return string_value;
}

/**
 * Retrieves the Bool value of this JSONValue
 * Use IsBool() before using this method.
 *
 * @access public
 *
 * @return bool Returns the bool value
 */
bool JSONValue::AsBool() const
{
    return bool_value;
}

/**
 * Retrieves the Number value of this JSONValue
 * Use IsNumber() before using this method.
 *
 * @access public
 *
 * @return float Returns the number value
 */
float JSONValue::AsNumber() const
{
    return number_value;
}

int JSONValue::AsInt() const
{
    return (int)AsNumber();
}

/**
 * Retrieves the Array value of this JSONValue
 * Use IsArray() before using this method.
 *
 * @access public
 *
 * @return JSONArray Returns the array value
 */
const JSONArray &JSONValue::AsArray() const
{
    return array_value;
}

/**
 * Retrieves the Object value of this JSONValue
 * Use IsObject() before using this method.
 *
 * @access public
 *
 * @return JSONObject Returns the object value
 */
const JSONObject &JSONValue::AsObject() const
{
    return object_value;
}

/**
 * Retrieves the number of children of this JSONValue.
 * This number will be 0 or the actual number of children
 * if IsArray() or IsObject().
 *
 * @access public
 *
 * @return The number of children.
 */
std::size_t JSONValue::CountChildren() const
{
    switch (type)
    {
        case JSONType_Array:
            return array_value.size();
        case JSONType_Object:
            return object_value.size();
        default:
            return 0;
    }
}

/**
 * Checks if this JSONValue has a child at the given index.
 * Use IsArray() before using this method.
 *
 * @access public
 *
 * @return bool Returns true if the array has a value at the given index.
 */
bool JSONValue::HasChild(std::size_t index) const
{
    if (type == JSONType_Array)
    {
        return index < array_value.size();
    }
    else
    {
        return false;
    }
}

/**
 * Retrieves the child of this JSONValue at the given index.
 * Use IsArray() before using this method.
 *
 * @access public
 *
 * @return JSONValue* Returns JSONValue at the given index or NULL
 *                    if it doesn't exist.
 */
std::shared_ptr<JSONValue> JSONValue::Child(std::size_t index)
{
    if (index < array_value.size())
    {
        return array_value[index];
    }
    else
    {
        return std::shared_ptr<JSONValue>();
    }
}

/**
 * Checks if this JSONValue has a child at the given key.
 * Use IsObject() before using this method.
 *
 * @access public
 *
 * @return bool Returns true if the object has a value at the given key.
 */
bool JSONValue::HasChild(const wchar_t* name) const
{
    if (type == JSONType_Object)
    {
        return object_value.find(name) != object_value.end();
    }
    else
    {
        return false;
    }
}

/**
 * Retrieves the child of this JSONValue at the given key.
 * Use IsObject() before using this method.
 *
 * @access public
 *
 * @return JSONValue* Returns JSONValue for the given key in the object
 *                    or NULL if it doesn't exist.
 */
std::shared_ptr<JSONValue> JSONValue::Child(const wchar_t* name)
{
    JSONObject::const_iterator it = object_value.find(name);
    if (it != object_value.end())
    {
        return it->second;
    }
    else
    {
        return std::shared_ptr<JSONValue>();
    }
}

/**
 * Retrieves the keys of the JSON Object or an empty vector
 * if this value is not an object.
 *
 * @access public
 *
 * @return std::vector<lite3dpp::lited3dpp_wstring> A vector containing the keys.
 */
std::vector<lite3dpp::WString> JSONValue::ObjectKeys() const
{
    std::vector<lite3dpp::WString> keys;

    if (type == JSONType_Object)
    {
        JSONObject::const_iterator iter = object_value.begin();
        while (iter != object_value.end())
        {
            keys.emplace_back(iter->first);

            iter++;
        }
    }

    return keys;
}

/**
 * Creates a JSON encoded string for the value with all necessary characters escaped
 *
 * @access public
 *
 * @param bool prettyprint Enable prettyprint
 *
 * @return lite3dpp::lited3dpp_wstring Returns the JSON string
 */
lite3dpp::WString JSONValue::Stringify(bool const prettyprint) const
{
    size_t const indentDepth = prettyprint ? 1 : 0;
    return StringifyImpl(indentDepth);
}


/**
 * Creates a JSON encoded string for the value with all necessary characters escaped
 *
 * @access private
 *
 * @param size_t indentDepth The prettyprint indentation depth (0 : no prettyprint)
 *
 * @return lite3dpp::lited3dpp_wstring Returns the JSON string
 */
lite3dpp::WString JSONValue::StringifyImpl(size_t const indentDepth) const
{
    lite3dpp::WString ret_string;
    size_t const indentDepth1 = indentDepth ? indentDepth + 1 : 0;
    lite3dpp::WString const indentStr = Indent(indentDepth);
    lite3dpp::WString const indentStr1 = Indent(indentDepth1);

    switch (type)
    {
        case JSONType_Null:
            ret_string = L"null";
            break;

        case JSONType_String:
            ret_string = StringifyString(string_value);
            break;

        case JSONType_Bool:
            ret_string = bool_value ? L"true" : L"false";
            break;

        case JSONType_Number:
        {
            if (isinf(number_value) || isnan(number_value))
                ret_string = L"null";
            else
            {
                lite3dpp::WStringstream ss;
                ss.precision(10);
                ss << std::fixed << number_value;
                ret_string = ss.str();
            }
            break;
        }

        case JSONType_Array:
        {
            ret_string = indentDepth ? L"[\n" + indentStr1 : L"[";
            JSONArray::const_iterator iter = array_value.begin();
            while (iter != array_value.end())
            {
                ret_string += (*iter)->StringifyImpl(indentDepth1);

                if ((*iter)->IsObject())
                {
                    // Not at the end - add a separator
                    if (++iter != array_value.end())
                        ret_string += L",\n" + indentStr1;
                }
                else
                {
                    // Not at the end - add a separator
                    if (++iter != array_value.end())
                        ret_string += L",";
                }
            }
            ret_string += indentDepth ? L"\n" + indentStr + L"]" : L"]";
            break;
        }

        case JSONType_Object:
        {
            ret_string = indentDepth ? L"{\n" + indentStr1 : L"{";
            JSONObject::const_iterator iter = object_value.begin();
            while (iter != object_value.end())
            {
                ret_string += StringifyString((*iter).first);
                ret_string += L":";
                ret_string += (*iter).second->StringifyImpl(indentDepth1);

                // Not at the end - add a separator
                if (++iter != object_value.end())
                    ret_string += L",\n" + indentStr1;
            }
            ret_string += indentDepth ? L"\n" + indentStr + L"}" : L"}";
            break;
        }
    }

    return ret_string;
}

/**
 * Creates a JSON encoded string with all required fields escaped
 * Works from http://www.ecma-internationl.org/publications/files/ECMA-ST/ECMA-262.pdf
 * Section 15.12.3.
 *
 * @access private
 *
 * @param lite3dpp::lited3dpp_wstring str The string that needs to have the characters escaped
 *
 * @return lite3dpp::lited3dpp_wstring Returns the JSON string
 */
lite3dpp::WString JSONValue::StringifyString(const lite3dpp::WString &str)
{
    lite3dpp::WString str_out = L"\"";

    lite3dpp::WString::const_iterator iter = str.begin();
    while (iter != str.end())
    {
        wchar_t chr = *iter;

        if (chr == L'"' || chr == L'\\')
        {
            str_out += L'\\';
            str_out += chr;
        }
        else if (chr == L'\b')
        {
            str_out += L"\\b";
        }
        else if (chr == L'\f')
        {
            str_out += L"\\f";
        }
        else if (chr == L'\n')
        {
            str_out += L"\\n";
        }
        else if (chr == L'\r')
        {
            str_out += L"\\r";
        }
        else if (chr == L'\t')
        {
            str_out += L"\\t";
        }
        else if (chr < L' ' || chr > 126)
        {
            str_out += L"\\u";
            for (int i = 0; i < 4; i++)
            {
                int value = (chr >> 12) & 0xf;
                if (value >= 0 && value <= 9)
                    str_out += (wchar_t)('0' + value);
                else if (value >= 10 && value <= 15)
                    str_out += (wchar_t)('A' + (value - 10));
                chr <<= 4;
            }
        }
        else
        {
            str_out += chr;
        }

        iter++;
    }

    str_out += L"\"";
    return str_out;
}

/**
* Creates the indentation string for the depth given
*
* @access private
*
* @param size_t indent The prettyprint indentation depth (0 : no indentation)
*
* @return lite3dpp::lited3dpp_wstring Returns the string
*/
lite3dpp::WString JSONValue::Indent(size_t depth)
{
    const size_t indent_step = 2;
    depth ? --depth : 0;
    lite3dpp::WString indentStr(depth * indent_step, ' ');
    return indentStr;
}
