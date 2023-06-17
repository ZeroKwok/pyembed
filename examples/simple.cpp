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

#include "pyembed.h"
#include <iostream>
#include <filesystem>
#include <boost/format.hpp>
#include <boost/detail/lightweight_test.hpp>

// 修改下面的预处理条件，启用重定向标准输入/输出的示例
#if 1
inline pyembed& my_pyembed() {
    return pyembed::get();
}

#else

// 子类化是为了重定向标准输入/输出的处理
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
    // str 为utf-8编码，这里仅示范："如何重定向标准输入输出"。
    // 并未处理编码转换，因此遇到中文时会出现乱码。
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
    return pyembed::get<pyembed_ex>();
}
#endif

int main(int argc, char* argv[])
{
    namespace bp = boost::python;
    std::cout << "Testing embedded python: " << std::endl;

    // 初始化并以程序当前目录作为解释器的家目录
    my_pyembed().init();

    // 调试器查看Python对象
    {
        bp::str address0 = "6 East Changan Avenue PeKing";
        bp::str address1 = "NO.70 dong feng dong Rd.Guangzhou";

        bp::list hobby;
        hobby.append("Coding");
        hobby.append("Swimming");

        bp::dict personInfo;
        personInfo["Name"] = "Zero";
        personInfo["Ages"] = 26;
        personInfo["Male"] = "Man";
        personInfo["Hobby"] = hobby;
        personInfo["Address"] = bp::make_tuple(address0, address1);

        // 在这里下断点, 然后观察personInfo对象的值
        bp::len(personInfo);
    }

    // 运行表达式 & 标准输出流
    my_pyembed().exec("print('Hello World')");

    // 抛出Python异常默认实现是输出到标准错误流，指定异常处理器可以覆写此逻辑
    my_pyembed().exec("Hello");
    my_pyembed().exec("Hello",
        [](const pyembed::pyerror& pyerr)
        {
            std::string message = bp::extract<std::string>(pyerr.pyexception);
            std::string exception = pyerr.format_exception();

            auto display = boost::str(boost::format(
                "\n"
                "my_pyembed().exec() failed, error: %1%\n"
                "%2%")
                % message % exception);
            std::cerr << display << std::endl;

            return true; // 返回false表示异常没有被处理，将被打印到标准错误中。
        });

    // 调用Python接口
    my_pyembed().exec(
        "def getAddress(name):\n"
        "   if name == 'jack':\n"
        "       return '6 East Changan Avenue PeKing'\n"
        "   return 'NO.70 dong feng dong Rd.Guangzhou'"
    );
    auto getAddress = my_pyembed().local()["getAddress"];
    auto address0 = bp::extract<std::string>(getAddress("jack"))();
    auto address1 = bp::extract<std::string>(getAddress("null"))();
    BOOST_TEST(address0 == "6 East Changan Avenue PeKing");
    BOOST_TEST(address1 == "NO.70 dong feng dong Rd.Guangzhou");

    // 标准输入流
    my_pyembed().exec("print(input('input:'))\n");

    // 中断处理：按 Ctrl+C 中断循环 
    my_pyembed().exec(
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