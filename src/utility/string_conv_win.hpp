// This file is part of the utility distribution.
// Copyright (c) 2018-2023 Zero Kwok.
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
// 
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this software; 
// If not, see <http://www.gnu.org/licenses/>.
//
// Author:  Zero Kwok
// Contact: zero.kwok@foxmail.com 
// 

#ifndef string_conv_win_h__
#define string_conv_win_h__

#include <assert.h>
#include <windows.h>

namespace util {
namespace conv {
namespace detail {

inline const uintptr_t _ansi2utf16(const char* input, wchar_t* output)
{
    assert(input);
    int len = ::MultiByteToWideChar(CP_ACP, 0, input, -1, NULL, 0);

    if( !output )
        return (uintptr_t)len;

    ::MultiByteToWideChar(CP_ACP, 0, input, -1, (LPWSTR)output, len);
    return (uintptr_t)output;
}

inline const uintptr_t _utf162ansi(const wchar_t* input, char* output)
{
    assert(input);
    int len = ::WideCharToMultiByte(CP_ACP, 0, input, -1, NULL, 
        0, NULL, NULL);

    if( !output )
        return (uintptr_t)len;
    ::WideCharToMultiByte(CP_ACP, 0, input, -1, output, len, NULL, NULL);
    return (uintptr_t)output;
}

inline const uintptr_t _utf82utf16(const char* input, wchar_t* output)
{
    assert(input);
    int len = ::MultiByteToWideChar(CP_UTF8, 0, input, -1, NULL, 0);

    if( !output )
        return (uintptr_t)len;

    ::MultiByteToWideChar(CP_UTF8, 0, input, -1, (LPWSTR)output, len);
    return (uintptr_t)output;
}

inline const uintptr_t _utf162utf8(const wchar_t* input, char* output)
{
    assert(input);
    int len = ::WideCharToMultiByte(CP_UTF8, 0, input, -1, NULL, 
        0, NULL, NULL);

    if( !output )
        return (uintptr_t)len;
    ::WideCharToMultiByte(CP_UTF8, 0, input, -1, output, len, NULL, NULL);
    return (uintptr_t)output;
}

} // detail

inline std::wstring& string_to_wstring(
    const std::string& input, std::wstring& output)
{
    size_t len = detail::_ansi2utf16(input.c_str(), 0);

    output.resize(len);
    detail::_ansi2utf16(input.c_str(), const_cast<wchar_t*>(output.data()));

    if (output[output.size() - 1] == '\0')
        output.erase(output.begin() + (output.size() - 1));

    return output;
}

inline std::string& wstring_to_string(
    const std::wstring& input, std::string& output)
{
    size_t len = detail::_utf162ansi(input.c_str(), 0);

    output.resize(len);
    detail::_utf162ansi(input.c_str(), const_cast<char*>(output.data()));

    if (output[output.size() - 1] == '\0')
        output.erase(output.begin() + (output.size() - 1));

    return output;
}

inline std::wstring& utf8_to_wstring(
    const std::string& input, std::wstring& output)
{
    size_t len = detail::_utf82utf16(input.c_str(), 0);

    output.resize(len);
    detail::_utf82utf16(input.c_str(), const_cast<wchar_t*>(output.data()));

    if (output[output.size() - 1] == '\0')
        output.erase(output.begin() + (output.size() - 1));

    return output;
}

inline std::string& wstring_to_utf8(
    const std::wstring& str, std::string& output)
{
    size_t len = detail::_utf162utf8(str.c_str(), 0);

    output.resize(len);
    detail::_utf162utf8(str.c_str(), const_cast<char*>(output.data()));

    if (output[output.size() - 1] == '\0')
        output.erase(output.begin() + (output.size() - 1));

    return output;
}

inline std::string& utf8_to_string(
    const std::string& input, std::string& output)
{
    std::wstring buff;
    utf8_to_wstring(input, buff);
    wstring_to_string(buff, output);

    return output;
}

inline std::string& string_to_utf8(
    const std::string& input, std::string& output)
{
    std::wstring buff;
    string_to_wstring(input, buff);
    wstring_to_utf8(buff, output);

    return output;
}

} // conv
} // util

#endif // string_conv_win_h__