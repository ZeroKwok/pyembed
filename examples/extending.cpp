// Copyright Ralf W. Grosse-Kunstleve 2002-2004. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "pyembed.h"
#include <boost/python/class.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <iostream>
#include <string>

namespace { // Avoid cluttering the global namespace.

  // A friendly class.
  class hello
  {
    public:
      hello(const std::string& country) { this->country = country; }
      std::string greet() const { return "Hello from " + country; }
    private:
      std::string country;
  };

  // A function taking a hello object as an argument.
  std::string invite(const hello& w) {
    return w.greet() + "! Please come soon!";
  }
}

BOOST_PYTHON_MODULE(extending)
{
    using namespace boost::python;
    class_<hello>("hello", init<std::string>())
        // Add a regular member function.
        .def("greet", &hello::greet)
        // Add invite() as a member of hello!
        .def("invite", invite)
        ;
    
    // Also add invite() as a regular function to the module.
    def("invite", invite);
}

int main(int argc, char* argv[])
{
    std::cout << "Testing embedded python: " << std::endl;

    // 注册内建模块
    pyembed::get().append_inittab({ { "extending", PyInit_extending } });

    // 初始化并以程序当前目录作为解释器的家目录
    pyembed::get().init();

    // 添加搜索目录, test_extending.py内部的import指令需要
    std::filesystem::path script = __FILE__;
    script = script.parent_path() / "scripts";
    pyembed::get().exec(
        "import sys\n"
        "sys.path.append('" + script.generic_u8string() + "')");

    // 运行脚本
    pyembed::get().exec_file(script / "test_extending.py", {});

    return 0;
}