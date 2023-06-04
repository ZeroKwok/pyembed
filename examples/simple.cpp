// This file is part of the pyembed distribution.
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

#include "pymebed.h"
#include <iostream>
#include <filesystem>
#include <boost/format.hpp>

// 修改下面的预处理条件，启用重定向标准流的示例
#if 1
inline pyembed& my_pyembed() {
    return get_pyembed();
}

#else

// 子类化是为了重写标准流的处理
class pyembed_ex : public pyembed
{
public:
    pyembed_ex(const std::type_info& type)
        : pyembed(type)
    {}

    std::string readline_stdin(int size = -1) override
    {
        std::string result = "Hello pyembed!\n";
        if (size != -1 && result.size() > size)
            result.resize(size);
        return result;
    }

    // 注意:
    // str 为utf-8编码，这里仅做示范：如何重定向标准流, 并未处理编码转换, 因此遇到中文时会出现乱码
    void write_stdout(const std::string& str) override
    {
        Py_BEGIN_ALLOW_THREADS
        std::cout << str;
        Py_END_ALLOW_THREADS
    }

    void write_stderr(const std::string& str) override
    {
        Py_BEGIN_ALLOW_THREADS
        std::cerr << str;
        Py_END_ALLOW_THREADS
    }
};

inline pyembed_ex& my_pyembed() {
    return get_pyembed<pyembed_ex>();
}
#endif

int main(int argc, char* argv[])
{
    namespace bp = boost::python;
    std::cout << "Testing embedded python: " << std::endl;

    // 初始化并以程序当前目录作为解释器的家目录
    my_pyembed().init();

    // 运行表达式 & 标准输出流
    my_pyembed().exec("print(\"Hello\")\n");
    my_pyembed().exec("print(\"World\")\n\n");

    // 抛出Python异常默认实现是输出到标准错误流，指定异常处理器可以覆写此逻辑
    my_pyembed().exec("Hello\n");
    my_pyembed().exec("Hello\n", 
        [](bp::object pyexception, const std::string& traceback) 
        {
            std::string exception = bp::extract<std::string>(pyexception);
            auto display = boost::str(boost::format(
                "my_pyembed().exec() failed, error: {%1%}\n%2%")
                % exception % traceback);
            std::cerr << display << std::endl;

            return true; // 返回false表示异常没有被处理，将被打印到标准错误中。
        });

    // 标准输入流
    my_pyembed().exec("print(input('input:'))\n");

    // 中断处理：按 Ctrl+C 中断循环 
    get_pyembed().exec(
        "import time\n"
        "for i in range(1000) :\n"
        "   print(i)\n"
        "   time.sleep(0.5)\n");

    // 运行脚本（包含中文文件名）
    std::filesystem::path script = __FILE__;
    script = script.parent_path() / "scripts";
    my_pyembed().exec_file(script / L"中文.py", {});

    // 参数传递、返回值、上下文数据
    auto result = my_pyembed().exec_file(
        script / "script_args.py", { "--filename=filename.ext", "-v", "-c" });
    int number = boost::python::extract<int>(my_pyembed().local()["number"]);
    std::cout << "number: " << number << std::endl;

#if defined(WIN32) || defined(_WIN32)
    ::system("pause");
#endif

    return 0;
}