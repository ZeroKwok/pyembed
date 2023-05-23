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

#ifndef config_h__
#define config_h__

/*
*   config.h 
*   
*   v0.1    2018-4 By Zero
*   v0.2    2020-8 By Zero
*/

// Define platform macro
#if defined(WIN32) || defined(_WIN32)
#   define OS_WIN      1
#elif defined(__APPLE__) && (defined(__i386__) || defined(__ARMEL__))
#   define OS_IOS      1
#elif defined(__APPLE__)
#   define OS_MACOSX   1
#elif defined(__linux__)
#   define OS_LINUX    1
#elif defined(__ANDROID__)
#   define OS_ANDROID  1
#elif defined(__FreeBSD__)
#   define OS_FREEBSD  1
#else
#   error Please add support for your platform in config.h
#endif

// For access to standard POSIXish features, use OS_POSIX instead of a
// more specific macro.
#if defined(OS_MACOSX) || defined(OS_IOS) || defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_FREEBSD)
#   define OS_POSIX    1
#endif

// Compiler detection.
#if defined(__GNUC__)
#   define COMPILER_GCC 1
#elif defined(_MSC_VER)
#   define COMPILER_MSVC 1
#else
#   error Please add support for your compiler in config.h
#endif

// Processor architecture detection.  For more info on what's defined, see:
// http://msdn.microsoft.com/en-us/library/b0084kay.aspx
// http://www.agner.org/optimize/calling_conventions.pdf
// or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#   define ARCH_CPU_X86_FAMILY 1
#   define ARCH_CPU_X86_64     1
#   define ARCH_CPU_64_BITS    1
#elif defined(_M_IX86) || defined(__i386__)
#   define ARCH_CPU_X86_FAMILY 1
#   define ARCH_CPU_X86        1
#   define ARCH_CPU_32_BITS    1
#elif defined(__ARMEL__)
#   define ARCH_CPU_ARM_FAMILY 1
#   define ARCH_CPU_ARMEL      1
#   define ARCH_CPU_32_BITS    1
#   define WCHAR_T_IS_UNSIGNED 1
#else
#   error Please add support for your architecture in config.h
#endif

// GNU libc offers the helpful header <endian.h> which defines
// __BYTE_ORDER
#if defined(__GLIBC__)
#   include <endian.h>
#   if (__BYTE_ORDER == __LITTLE_ENDIAN)
#       define CPU_LITTLE_ENDIAN    1
#   elif (__BYTE_ORDER == __BIG_ENDIAN)
#       define CPU_BIG_ENDIAN   1
#   elif (__BYTE_ORDER == __PDP_ENDIAN)
#       define CPU_PDP_ENDIAN   1
#   else
#       error Unknown machine endianness detected.
#   endif
#   define CPU_BYTE_ORDER __BYTE_ORDER
#elif defined(_BIG_ENDIAN)
#   define CPU_BIG_ENDIAN       1
#   define CPU_BYTE_ORDER       4321
#elif defined(_LITTLE_ENDIAN)
#    define CPU_LITTLE_ENDIAN   1
#    define CPU_BYTE_ORDER      1234
#elif defined(__sparc) || defined(__sparc__) \
   || defined(_POWER) || defined(__powerpc__) \
   || defined(__ppc__) || defined(__hpux) \
   || defined(_MIPSEB) || defined(_POWER) \
   || defined(_MIPSEB) || defined(_POWER) \
   || defined(__s390__)
#   define CPU_BIG_ENDIAN       1
#   define CPU_BYTE_ORDER       4321
#elif defined(__i386__) || defined(__alpha__) \
    || defined(__ia64) || defined(__ia64__) \
    || defined(_M_IX86) || defined(_M_IA64) \
    || defined(_M_ALPHA) || defined(__amd64) \
    || defined(__amd64__) || defined(_M_AMD64) \
    || defined(__x86_64) || defined(__x86_64__) \
    || defined(_M_X64) || defined (__ARMEL__)
#   define CPU_LITTLE_ENDIAN    1
#   define CPU_BYTE_ORDER       1234
#else
#   error Please set up your byte order config.h.
#endif

#if defined(COMPILER_MSVC)
#   if _MSC_VER > 1600 // vs2010之后的版本使用自身携带的inttypes.h
#      include <inttypes.h>
#   else 
#      include "msinttypes/inttypes.h"
#   endif
#else
#   include <stddef.h>
#   include <inttypes.h>
#endif

// MSVC 
#if defined(COMPILER_MSVC)

// Microsoft Visual C++各版本的_MSC_VER值定义
// https://zh.wikipedia.org/zh-hans/Microsoft_Visual_C%2B%2B
// https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
// https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170
// 
                                  // _MSC_VER == 1932 (Visual Studio 2022 version 17.2      MSVC++ 14.32)
                                  // _MSC_VER == 1931 (Visual Studio 2022 version 17.1      MSVC++ 14.31)
#   define  _MSVC_143       1930  // _MSC_VER == 1930 (Visual Studio 2022 RTW (17.0)        MSVC++ 14.30)

                                  // _MSC_VER == 1929 (Visual Studio 2019 Version 16.10/11  MSVC++ 14.29)
                                  // _MSC_VER == 1928 (Visual Studio 2019 Version 16.8/9    MSVC++ 14.28)
                                  // _MSC_VER == 1927 (Visual Studio 2019 Version 16.7      MSVC++ 14.27)
                                  // 
                                  // _MSC_VER == 1926 (Visual Studio 2019 Version 16.6 MSVC++ 14.26)
                                  // _MSC_VER == 1925 (Visual Studio 2019 Version 16.5 MSVC++ 14.25)
                                  // _MSC_VER == 1924 (Visual Studio 2019 Version 16.4 MSVC++ 14.24)
                                  // _MSC_VER == 1923 (Visual Studio 2019 Version 16.3 MSVC++ 14.23)
                                  // _MSC_VER == 1922 (Visual Studio 2019 Version 16.2 MSVC++ 14.22)
                                  // _MSC_VER == 1921 (Visual Studio 2019 Version 16.1 MSVC++ 14.21)

#   define  _MSVC_142       1920  // _MSC_VER == 1920 (Visual Studio 2019 Version 16.0 MSVC++ 14.20)

                                  // _MSC_VER == 1916 (Visual Studio 2017 version 15.9 MSVC++ 14.16)
                                  // _MSC_VER == 1915 (Visual Studio 2017 version 15.8 MSVC++ 14.15)
                                  // _MSC_VER == 1914 (Visual Studio 2017 version 15.7 MSVC++ 14.14)
                                  // _MSC_VER == 1913 (Visual Studio 2017 version 15.6 MSVC++ 14.13)
                                  // _MSC_VER == 1912 (Visual Studio 2017 version 15.5 MSVC++ 14.12)
                                  // _MSC_VER == 1911 (Visual Studio 2017 version 15.3 MSVC++ 14.11)

#   define  _MSVC_141       1910  // _MSC_VER == 1910 (Visual Studio 2017 version 15.0 MSVC++ 14.1)
#   define  _MSVC_140       1900  // _MSC_VER == 1900 (Visual Studio 2015 version 14.0 MSVC++ 14.0)
#   define  _MSVC_120       1800  // _MSC_VER == 1800 (Visual Studio 2013 version 12.0 MSVC++ 12.0)
#   define  _MSVC_110       1700  // _MSC_VER == 1700 (Visual Studio 2012 version 11.0 MSVC++ 11.0)
#   define  _MSVC_100       1600  // _MSC_VER == 1600 (Visual Studio 2010 version 10.0 MSVC++ 10.0)
#   define  _MSVC_90        1500  // _MSC_VER == 1500 (Visual Studio 2008 version 9.0  MSVC++ 9.0 )
#   define  _MSVC_80        1400  // _MSC_VER == 1400 (Visual Studio 2005 version 8.0  MSVC++ 8.0 )

// noexcept
// https://zh.cppreference.com/w/cpp/compiler_support/11
#   if _MSC_VER < _MSVC_140 
#       define noexcept throw()
#   endif

// nullptr
#   if _MSC_VER <= _MSVC_100 
#      ifndef nullptr
#          define nullptr NULL
#      endif
#      ifndef constexpr
#          define constexpr
#      endif
#   endif

// 去除Windows.h中携带的min, max宏定义, 且在后面禁止包含
#   ifndef  NOMINMAX
#      define  NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#      define  WIN32_LEAN_AND_MEAN 
#   endif
#   ifdef min
#      undef min
#   endif
#   ifdef max
#      undef max
#   endif
#endif // if defined(COMPILER_MSVC)

// SDK Version
#if OS_WIN
#   if defined(_WIN32_WINNT)
#       include <winsdkver.h>
#       include <sdkddkver.h>
#   endif
#endif

#endif // config_h__
