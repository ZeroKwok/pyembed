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

#ifndef string_conv_iconv_h__
#define string_conv_iconv_h__

#include <errno.h>
#include <iconv.h>
#include <string.h>
#include <string>
#include <stdexcept>

namespace util {
namespace conv {

template<class InString, class OutString>
OutString& convert_with_iconv(
    const    InString& input,
            OutString& output,
    const std::string& in_encode,
    const std::string& out_encode,
    const         bool ignore_error = false)
{
    // 1. both GLIBC and libgnuiconv will use the locale's encoding 
    //    if ininbuf or outbuf is an empty string.
    // 2. In case of error, it sets errno and returns (iconv_t) -1.
    iconv_t conv = ::iconv_open(out_encode.c_str(), in_encode.c_str());

    if (conv == (iconv_t)-1)
    {
        if (errno == EINVAL)
        {
            throw std::runtime_error(
                "not supported from " + in_encode + " to " + out_encode);
        }
        else
        {
            int error = errno;
            throw std::runtime_error(
                "iconv_open() failed: " + std::to_string(error) + ", " + strerror(error));
        }
    }

    struct guard
    {
        iconv_t _conv;

        guard(iconv_t conv)
            : _conv(conv) {}
        ~guard() { ::iconv_close(_conv); }
    }
    _guard(conv);

    // copy the string to a buffer as iconv function requires a non-const char pointer.
    std::string in_buf(
        reinterpret_cast<const char*>(&input[0]), 
        input.size() * sizeof(typename InString::value_type));

    char * src_ptr  = &in_buf[0];
    size_t src_size = in_buf.size();

    std::string       buf(1024, 0);
    std::string       dst;

    while (0 < src_size)
    {
        char * dst_ptr  = &buf[0];
        size_t dst_size = buf.size();

        size_t res = ::iconv(conv, &src_ptr, &src_size, &dst_ptr, &dst_size);

        if (res == (size_t)-1)
        {
            if (errno == E2BIG) // 输出缓冲区没有更多的空间容纳下一个转换字符
            {
                // ignore this error
            }
            else if (ignore_error)
            {
                // skip character
                ++src_ptr;
                --src_size;
            }
            else
            {
                switch (errno)
                {
                case EILSEQ:  // 在输入中遇到了无效的多字节序列, *inbuf指向无效的多字节序列的开头.
                case EINVAL:  // 在输入中遇到了不完整的多字节序列, *inbuf指向不完整多字节序列的开头.
                    throw std::runtime_error("invalid or incomplete multibyte or wide character");

                default:
                {
                    int error = errno;
                    throw std::runtime_error(
                        "iconv() failed: " + std::to_string(error) + ", " + strerror(error));
                }
                }
            }
        }
        dst.append(&buf[0], buf.size() - dst_size);
    }

    output.assign(
        reinterpret_cast<typename OutString::value_type*>(&dst[0]),
        dst.size() / sizeof(typename OutString::value_type));

    return output;
}

} // conv
} // util

#endif // string_conv_iconv_h__
