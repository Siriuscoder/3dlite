/*
 * File JSON.cpp part of the SimpleJSON Library - http://mjpa.in/json
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

#include <lite3dpp/json/JSON.h>
#include <lite3d/lite3d_alloc.h>

/**
 * Parses a complete JSON encoded string
 * This is just a wrapper around the UNICODE Parse().
 *
 * @access public
 *
 * @param char* data The JSON text
 *
 * @return JSONValue* Returns a JSON Value representing the root, or NULL on error
 */
std::shared_ptr<JSONValue> JSON::Parse(const char *data, size_t size)
{
    size_t length = size + 1;
    wchar_t *w_data = static_cast<wchar_t*>(lite3d_malloc(length * sizeof(wchar_t)));
    #if defined(WIN32) && !defined(__GNUC__)
        size_t ret_value = 0;
        if (mbstowcs_s(&ret_value, w_data, length, data, size) != 0)
        {
            lite3d_free(w_data);
            return nullptr;
        }
    #elif defined(ANDROID)
        // mbstowcs seems to misbehave on android
        for(size_t i = 0; i<size; i++)
            w_data[i] = (wchar_t)data[i];
    #else
        if (mbstowcs(w_data, data, size) == (size_t)-1)
        {
            lite3d_free(w_data);
            return NULL;
        }
    #endif
    
    w_data[size] = 0;
    std::shared_ptr<JSONValue> value = JSON::Parse(w_data);
    lite3d_free(w_data);
    return value;
}

/**
 * Parses a complete JSON encoded string (UNICODE input version)
 *
 * @access public
 *
 * @param wchar_t* data The JSON text
 *
 * @return JSONValue* Returns a JSON Value representing the root, or NULL on error
 */
std::shared_ptr<JSONValue> JSON::Parse(const wchar_t *data)
{
    // Skip any preceding whitespace, end of data = no JSON = fail
    if (!SkipWhitespaceAndComments(&data))
        return NULL;

    // We need the start of a value here now...
    std::shared_ptr<JSONValue> value = JSONValue::Parse(&data);
    if (value)
        return value;
    
    // Can be white space now and should be at the end of the string then...
    if (SkipWhitespaceAndComments(&data))
    {
        return std::shared_ptr<JSONValue>();
    }
    
    // We're now at the end of the string
    return value;
}

/**
 * Turns the passed in JSONValue into a JSON encode string
 *
 * @access public
 *
 * @param JSONValue* value The root value
 *
 * @return lite3dpp::lited3dpp_wstring Returns a JSON encoded string representation of the given value
 */
lite3dpp::WString JSON::Stringify(const std::shared_ptr<JSONValue> &value)
{
    if (value)
        return value->Stringify();
    else
        return L"";
}

/**
 * Skips over any whitespace characters (space, tab, \r or \n) defined by the JSON spec
 *
 * @access protected
 *
 * @param wchar_t** data Pointer to a wchar_t* that contains the JSON text
 *
 * @return bool Returns true if there is more data, or false if the end of the text was reached
 */
bool JSON::SkipWhitespace(const wchar_t **data)
{
    while (**data != 0 && (**data == L' ' || **data == L'\t' || **data == L'\r' || **data == L'\n'))
        (*data)++;
    
    return **data != 0;
}

bool JSON::SkipWhitespaceAndComments(const wchar_t **data)
{
    if(!SkipWhitespace(data))
        return false;

    while (**data == L'/' && *((*data)+1) == L'/')
    {
        while (**data != 0 && (**data != L'\n'))
            (*data)++;
    
        if(!SkipWhitespace(data))
            return false;
    }

    return true;
}
/**
 * Extracts a JSON String as defined by the spec - "<some chars>"
 * Any escaped characters are swapped out for their unescaped values
 *
 * @access protected
 *
 * @param wchar_t** data Pointer to a wchar_t* that contains the JSON text
 * @param lite3dpp::lited3dpp_wstring& str Reference to a lite3dpp::lited3dpp_wstring to receive the extracted string
 *
 * @return bool Returns true on success, false on failure
 */
bool JSON::ExtractString(const wchar_t **data, lite3dpp::WString &str)
{
    str = L"";
    
    while (**data != 0)
    {
        // Save the char so we can change it if need be
        wchar_t next_char = **data;
        
        // Escaping something?
        if (next_char == L'\\')
        {
            // Move over the escape char
            (*data)++;
            
            // Deal with the escaped char
            switch (**data)
            {
                case L'"': next_char = L'"'; break;
                case L'\\': next_char = L'\\'; break;
                case L'/': next_char = L'/'; break;
                case L'b': next_char = L'\b'; break;
                case L'f': next_char = L'\f'; break;
                case L'n': next_char = L'\n'; break;
                case L'r': next_char = L'\r'; break;
                case L't': next_char = L'\t'; break;
                case L'u':
                {
                    // We need 5 chars (4 hex + the 'u') or its not valid
                    if (!simplejson_wcsnlen(*data, 5))
                        return false;
                    
                    // Deal with the chars
                    next_char = 0;
                    for (int i = 0; i < 4; i++)
                    {
                        // Do it first to move off the 'u' and leave us on the
                        // final hex digit as we move on by one later on
                        (*data)++;
                        
                        next_char <<= 4;
                        
                        // Parse the hex digit
                        if (**data >= '0' && **data <= '9')
                            next_char |= (**data - '0');
                        else if (**data >= 'A' && **data <= 'F')
                            next_char |= (10 + (**data - 'A'));
                        else if (**data >= 'a' && **data <= 'f')
                            next_char |= (10 + (**data - 'a'));
                        else
                        {
                            // Invalid hex digit = invalid JSON
                            return false;
                        }
                    }
                    break;
                }
                
                // By the spec, only the above cases are allowed
                default:
                    return false;
            }
        }
        
        // End of the string?
        else if (next_char == L'"')
        {
            (*data)++;
            str.shrink_to_fit(); // Remove unused capacity
            return true;
        }
        
        // Disallowed char?
        else if (next_char < L' ' && next_char != L'\t')
        {
            // SPEC Violation: Allow tabs due to real world cases
            return false;
        }
        
        // Add the next char
        str += next_char;
        
        // Move on
        (*data)++;
    }
    
    // If we're here, the string ended incorrectly
    return false;
}

/**
 * Parses some text as though it is an integer
 *
 * @access protected
 *
 * @param wchar_t** data Pointer to a wchar_t* that contains the JSON text
 *
 * @return float Returns the float value of the number found
 */
float JSON::ParseInt(const wchar_t **data)
{
    float integer = 0;
    while (**data != 0 && **data >= '0' && **data <= '9')
        integer = integer * 10 + (*(*data)++ - '0');
    
    return integer;
}

/**
 * Parses some text as though it is a decimal
 *
 * @access protected
 *
 * @param wchar_t** data Pointer to a wchar_t* that contains the JSON text
 *
 * @return float Returns the float value of the decimal found
 */
float JSON::ParseDecimal(const wchar_t **data)
{
    float decimal = 0.0;
    float factor = 0.1;
    while (**data != 0 && **data >= '0' && **data <= '9')
  {
    int digit = (*(*data)++ - '0');
        decimal = decimal + digit * factor;
    factor *= 0.1;
  }
    return decimal;
}

lite3dpp::String JSON::wStringToString(const lite3dpp::WString &str)
{
    lite3dpp::String strDest(str.begin(), str.end());
    return strDest;
}
