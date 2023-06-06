// Copyright Stefan Seefeld 2005.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/python.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <filesystem>
#include "pyembed.h"

namespace python = boost::python;

// An abstract base class
class Base : public boost::noncopyable
{
public:
    virtual ~Base() {};
    virtual std::string hello() = 0;
};

// C++ derived class
class CppDerived : public Base
{
public:
    virtual ~CppDerived() {}
    virtual std::string hello() { return "Hello from C++!"; }
};

// Familiar Boost.Python wrapper class for Base
struct BaseWrap : Base, python::wrapper<Base>
{
    virtual std::string hello()
    {
#if BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
        // workaround for VC++ 6.x or 7.0, see
        // http://boost.org/libs/python/doc/tutorial/doc/html/python/exposing.html#python.class_virtual_functions
        return python::call<std::string>(this->get_override("hello").ptr());
#else
        return this->get_override("hello")();
#endif
    }
};

// Pack the Base class wrapper into a module
BOOST_PYTHON_MODULE(embedded_hello)
{
    python::class_<BaseWrap, boost::noncopyable> base("Base");
}

class TestCppException
{
public:
    TestCppException()
    {}

    void throwException() {
        throw std::runtime_error("runtime_error");
    }
};

BOOST_PYTHON_MODULE(TestCppException)
{
    using namespace boost::python;
    class_<TestCppException>("TestCppException", init<>())
        .def("throwException", &TestCppException::throwException);
}

int main(int argc, char** argv)
{
    std::filesystem::path floder = __FILE__;
    floder = floder.parent_path() / "scripts";
    std::string script = (floder / "script.py").string();

    // Register the module with the interpreter
    pyembed::get().append_inittab({
        {"embedded_hello", PyInit_embedded_hello},
        {"TestCppException", PyInit_TestCppException},
    });

    // Initialize the interpreter
    pyembed::get().init();

    // eval
    {
        auto result = pyembed::get().eval("'abcdefg'.upper()");
        std::string value = python::extract<std::string>(result) BOOST_EXTRACT_WORKAROUND;
        BOOST_TEST(value == "ABCDEFG");
    }

    // exec
    {
        // Define the derived class in Python.
        auto result = pyembed::get().exec(
            "from embedded_hello import *        \n"
            "class PythonDerived(Base):          \n"
            "    def hello(self):                \n"
            "        return 'Hello from Python!' \n");

        auto PythonDerived = pyembed::get().local()["PythonDerived"];

        // Creating and using instances of the C++ class is as easy as always.
        CppDerived cpp;
        BOOST_TEST(cpp.hello() == "Hello from C++!");

        // But now creating and using instances of the Python class is almost
        // as easy!
        auto py_base = PythonDerived();
        Base& py = python::extract<Base&>(py_base) BOOST_EXTRACT_WORKAROUND;

        // Make sure the right 'hello' method is called.
        BOOST_TEST(py.hello() == "Hello from Python!");

        pyembed::get().register_exception_handler(
            [](std::function<void()> f) -> bool 
            {
                try
                {
                    f();
                    return false; // 没有异常返回false
                }
                catch (const std::runtime_error& e) 
                {
                    // 仅拦截感兴趣的异常，并转换为Python错误，其他异常将在上游异常处理器链上处理
                    // 如果没有注册其他异常处理的话，最顶层的异常处理器是:
                    // boost\python\errors.hpp: handle_exception_impl()
                    PyErr_SetString(PyExc_RuntimeError, e.what());
                }

                return true;
            });

        result = pyembed::get().exec(
            "from TestCppException import *   \n"
            "cppObj = TestCppException()      \n"
            "cppObj.throwException()          \n"
        );
    }

    // exec
    {
        pyembed::get().exec(
            "hello = open('hello.txt', 'w')\n"
            "hello.write('Hello world!')\n"
            "hello.close()");
    }

    // exec_file
    {
        // Run a python script in an empty environment.
        auto result = pyembed::get().exec_file(script);

        // Extract an object the script stored in the local dictionary.
        BOOST_TEST(python::extract<int>(pyembed::get().local()["number"]) == 42);
    }

    // exec_test_error
    {
        auto result = pyembed::get().exec("print(unknown) \n");
    }

    // Boost.Python doesn't support Py_Finalize yet.
    // Py_Finalize();
    return boost::report_errors();
}

// Including this file makes sure
// that on Windows, any crashes (e.g. null pointer dereferences) invoke
// the debugger immediately, rather than being translated into structured
// exceptions that can interfere with debugging.
#include "module_tail.cpp"