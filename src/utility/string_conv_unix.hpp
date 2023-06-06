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

#ifndef string_conv_unix_h__
#define string_conv_unix_h__

#include <assert.h>
#include "string_conv_iconv.hpp"

namespace util {
namespace conv {

inline std::wstring& utf8_to_wstring(
    const std::string& input, std::wstring& output)
{
    return convert_with_iconv(input, output, "UTF-8", "WCHAR_T", false);
}

inline std::wstring& string_to_wstring(
    const std::string& input, std::wstring& output)
{
    return utf8_to_wstring(input, output);
}

inline std::string& wstring_to_utf8(
    const std::wstring& input, std::string& output)
{
    return convert_with_iconv(input, output, "WCHAR_T", "UTF-8", false);
}

inline std::string& wstring_to_string(
    const std::wstring& input, std::string& output)
{
    return wstring_to_utf8(input, output);
}

inline std::string& utf8_to_string(
    const std::string& input, std::string& output)
{
    return output = input;
}

inline std::string& string_to_utf8(
    const std::string& input, std::string& output)
{
    return output = input;
}

} // conv
} // util

#endif // string_conv_unix_h__