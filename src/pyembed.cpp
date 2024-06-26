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
#include "pyconvert.hpp"
#include "utility/utility.hpp"

#include <assert.h>
#include <signal.h>
#include <iostream>
#include <strstream>
#include <functional>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

namespace bp = boost::python;

enum pipe_type 
{
    pystdin,
    pystdout,
    pystderr
};

template<pipe_type>
class PYEMBED_LIB pyredirector
{
public:
    pyredirector()
    { }

    pyredirector(std::function<void(const std::string&)> f)
        : m_fwrite(f)
    { }

    pyredirector(std::function<std::string(int size)> f)
        : m_freadline(f)
    { }

    void write(const std::string& str)
    {
        if (m_fwrite)
            m_fwrite(str);
    }

    std::string readline(int size = -1)
    {
        if (m_freadline)
            return m_freadline(size);
        else
            return "";
    }

public:
    std::function<void(const std::string&)> m_fwrite;
    std::function<std::string(int size)>    m_freadline;
};

typedef pyredirector<pystdin>  stdin_redirector;
typedef pyredirector<pystdout> stdout_redirector;
typedef pyredirector<pystderr> stderr_redirector;

//////////////////////////////////////////////////////////////////////////

std::string pyembed::pyerror::format_exception() const
{
    bp::object pymodule = bp::import("traceback");
    bp::object formatted = pymodule.attr("format_exception")(pytype, pyexception, pytraceback);
    bp::object content = bp::str("").join(formatted);

    return bp::extract<std::string>(content) BOOST_EXTRACT_WORKAROUND;
}

// 私有类
class pyembed_private 
{
public:
    pyembed_private(pyembed* p) {
        _public = p;
    }

    ~pyembed_private()
    {
        _stdin.reset();
        _stdout.reset();
        _stderr.reset();

        _public = nullptr;
    }

    void init()
    {
        _stdin = boost::make_shared<stdin_redirector>(
            [&](int size) -> std::string {
                return _public->readline_stdin(size);
            });

        _stdout = boost::make_shared<stdout_redirector>(
            [&](const std::string& str) {
                _public->write_stdout(str);
            });

        _stderr = boost::make_shared<stderr_redirector>(
            [&](const std::string& str) {
                _public->write_stderr(str);
            });

        // Retrieve the main module
        _main_module = std::make_shared<bp::object>(bp::import("__main__"));

        // Retrieve the main module's namespace
        _global = std::make_shared<bp::dict>(_main_module->attr("__dict__"));
        _local = std::make_shared<bp::dict>();
        *_local = *_global;

        string_from_python_type();
        string_from_python_base_exception();
        //string_from_python_traceback();
    }

    void exec_for(
        const std::function<void()>& f, 
        const std::function<bool(const pyembed::pyerror&)>& e = {})
    {
        try 
        {
            f();
        }
        catch (const bp::error_already_set&)
        {
            if (e)
            {
                // https://docs.python.org/zh-cn/3.6/c-api/exceptions.html#c.PyErr_Fetch
                // 将清除错误指示器
                PyObject* exc_type, * exc_value, * exc_traceback;
                PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
                PyErr_NormalizeException(&exc_type, &exc_value, &exc_traceback);

                auto pycast = [](PyObject* type) -> bp::object {
                    bp::handle<> obj(bp::allow_null(type));
                    if (obj.get() == nullptr)
                        return {};
                    return bp::object(obj);
                };

                pyembed::pyerror pyerr = {
                    pycast(exc_type),
                    pycast(exc_value),
                    pycast(exc_traceback),
                };

                if (e(pyerr))
                    return;

                PyErr_Restore(exc_type, exc_value, exc_traceback);

                Py_XINCREF(exc_type);
                Py_XINCREF(exc_value);
                Py_XINCREF(exc_traceback);
            }

            PyErr_Print();
        }
    }

    static void signal_handler(int signum)
    {
        if (signum == SIGINT)
            PyErr_SetInterrupt();

        signal(signum, &pyembed_private::signal_handler);
    }

public:
    bool  _initsigs;   // 是否初始化信号处理器
    bool  _redirect;   // 是否重定向标准流

    std::shared_ptr<bp::object> _main_module;
    std::shared_ptr<bp::dict>   _global;
    std::shared_ptr<bp::dict>   _local;
    
    static pyembed* _public;
    static boost::shared_ptr<stdin_redirector>  _stdin;
    static boost::shared_ptr<stdout_redirector> _stdout;
    static boost::shared_ptr<stderr_redirector> _stderr;
};

pyembed* pyembed_private::_public = nullptr;
boost::shared_ptr<stdin_redirector>  pyembed_private::_stdin;
boost::shared_ptr<stdout_redirector> pyembed_private::_stdout;
boost::shared_ptr<stderr_redirector> pyembed_private::_stderr;

//////////////////////////////////////////////////////////////////////////

boost::shared_ptr<stdin_redirector>  get_stdin(){
    return pyembed_private::_stdin;
}
boost::shared_ptr<stdout_redirector> get_stdout() {
    return pyembed_private::_stdout;
}
boost::shared_ptr<stderr_redirector> get_stderr() {
    return pyembed_private::_stderr;
}

BOOST_PYTHON_MODULE(redirector)
{
    using namespace boost::python;

    class_<stdin_redirector>("stdin",
        "This class redirects python's standard input to the pyembed.",
        init<>("initialize the stdin_redirector."))
        .def("__init__", make_constructor(get_stdin), "initialize the redirector.")
        .def("readline", &stdin_redirector::readline, arg("size") = -1, "readline sys.stdin redirection.");

    class_<stdout_redirector>("stdout",
        "This class redirects python's standard output to the pyembed.",
        init<>("initialize the stdout_redirector."))
        .def("__init__", make_constructor(get_stdout), "initialize the redirector.")
        .def("write", &stdout_redirector::write, "write sys.stdout redirection.");

    class_<stderr_redirector>("stderr",
        "This class redirects python's error output to the pyembed.",
        init<>("initialize the stderr_redirector."))
        .def("__init__", make_constructor(get_stderr), "initialize the redirector.")
        .def("write", &stderr_redirector::write, "write sys.stderr redirection.");
}

//////////////////////////////////////////////////////////////////////////

pyembed::pyembed(const std::type_info& type)
{
    __private = new pyembed_private(this);
    __private->_redirect = (type != typeid(pyembed)); // 只有在子类化之后才需要重定向
}

pyembed::~pyembed()
{
    // Boost.Python doesn't support Py_Finalize yet, so don't call it!

    delete __private;
           __private = nullptr;
}

pyembed* pyembed::get_ptr()
{
    return pyembed_private::_public;
}

void pyembed::init(
    const std::filesystem::path& pyhome /*= ""*/,
    bool initsigs /*= true*/)
{
    // Register the module with the interpreter; must be called before Py_Initialize.
    /* ToDo: C style cast to avoid compiler warning about
       deprecated conversion from string constant to 'char*' */
    if (PyImport_AppendInittab((char*)"redirector", PyInit_redirector) == -1) {
        throw std::runtime_error(
            "Failed to add embedded_python to python's "
            "interpreter built-in modules");
    }

    // https://docs.python.org/zh-cn/3/c-api/init.html#c.Py_SetPythonHome
    if (!pyhome.empty())
    {
        static std::wstring home = pyhome.wstring(); 
        Py_SetPythonHome(&home[0]); // In Python 3.6, it must be of the wchar_t*
    }

    // Set initsigs to 0 to skips initialization registration of signal handlers.
    // It is a fatal error if the initialization fails.
    // https://docs.python.org/3/c-api/init.html?highlight=py_initializeex#c.Py_InitializeEx
    // Bug: 3.8, 3.9. 3.10
    // https://bugs.python.org/issue41686
    Py_InitializeEx(__private->_initsigs = initsigs);

#if PY_VERSION_HEX < 0x3070000
    // For Python 3.6 and older
    // https://docs.python.org/3/c-api/init.html?highlight=pyeval_initthreads#c.PyEval_InitThreads
    PyEval_InitThreads();
#endif

    __private->init();

    if (__private->_redirect)
    {
        static const char* const redirect_py =
            "import sys\n"
            "import redirector\n"
            "sys.stdin  = redirector.stdin()\n"
            "sys.stdout = redirector.stdout()\n";
            "sys.stderr = redirector.stderr()\n";

#if 1
        /* FixMe: exception thrown, mmh - seems a bug in boost.python, see
         * http://www.nabble.com/Problems-with-Boost::Python-Embedding-Tutorials-td18799129.html */
        exec(redirect_py);
#else
        PyRun_String(redirect_py,
            Py_file_input,
            __private->_global.ptr(), __private->_global.ptr());
#endif
    }
}

void pyembed::interrupt()
{
    assert(__private->_initsigs);

    // 若初始化时没有注册信号处理(Py_InitializeEx(0)), 则无法通过PyErr_SetInterrupt()中断
    // https://docs.python.org/3/c-api/exceptions.html?highlight=pyerr_setinterrupt#c.PyErr_SetInterrupt
    PyErr_SetInterrupt();
}

bool pyembed::append_inittab(
    const char* name, PyObject* (*initfunc)(void))
{
    // Register the module with the interpreter
    if (PyImport_AppendInittab(name, initfunc) == -1)
    {
        write_stderr(
            boost::str(boost::format(
                "Failed to add %1% to the interpreter's built-in modules") 
                % name));
        return false;
    }

    return true;
}

bool pyembed::append_inittab(const std::vector<pymoudle>& pymoudles)
{
    for (const auto& item : pymoudles ) 
    {
        if (!append_inittab(item.name, item.initfunc))
            return false;
    }

    return true;
}

void pyembed::register_exception_handler(
    const std::function<bool(std::function<void()>)>& handler)
{
    namespace bpy = boost::python::detail;
    bpy::register_exception_handler([=](
        bpy::exception_handler const& e,
        boost::function0<void> const& f) -> bool {
            return e([=]{ handler(f); });
        });
}

boost::python::object pyembed::eval(
    const std::string& expression,
    const std::function<bool(const pyerror&)>& exception_handler /*= {} */)
{
    boost::python::object result;
    __private->exec_for([&]() {
        result = bp::eval(expression.c_str(),
            *__private->_global,
            *__private->_local);
        }, exception_handler);
    return result;
}

boost::python::object pyembed::exec(
    const std::string& snippets,
    const std::function<bool(const pyerror&)>& exception_handler /*= {} */)
{
    boost::python::object result;
    __private->exec_for([&]() {
        result = bp::exec(snippets.c_str(),
            *__private->_global, 
            *__private->_local);
        }, exception_handler);
    return result;
}

boost::python::object pyembed::exec_file(
    const std::filesystem::path& script, 
    const std::vector<std::string>& args /*= {}*/,
    const std::function<bool(const pyerror&)>& exception_handler /*= {} */)
{
    // 脚本文件获取绝对路径
    std::error_code ecode;
    std::filesystem::path filename =
        std::filesystem::canonical(script);

    boost::python::object result;
    __private->exec_for([=, &result]()
    {
        std::vector<std::wstring> argd(1, filename.wstring());
        for (auto& i : args)
        {
            std::wstring output;
            util::conv::utf8_to_wstring(i, output);
            argd.push_back(output);
        }

        std::vector<wchar_t*> argv;
        for (const auto& i : argd)
            argv.push_back((wchar_t*)i.c_str());

        // PySys_SetArgvEx()要求参数argv[0]须为执行的脚本文件
        // https://docs.python.org/3/c-api/init.html?highlight=pysys_setargvex#c.PySys_SetArgvEx
        // Set sys.argv based on argc and argv. 
        // These parameters are similar to those passed to the program's main() 
        // function with the difference that the first entry should refer to 
        // the script file to be executed rather than the executable hosting 
        // the Python interpreter. If there isn't a script that will be run, 
        // the first entry in argv can be an empty string. 
        PySys_SetArgvEx(argv.size(), (wchar_t**)&argv[0], 1);
        struct _scope {
            ~_scope() {
                std::vector<wchar_t*> argv(1, L"");
                PySys_SetArgvEx(argv.size(), (wchar_t**)&argv[0], 1); // 重置参数
            }
        } _clean;

#if OS_WIN
        // 运行脚本
        // https://docs.python.org/3/c-api/veryhigh.html?highlight=pyrun_string#c.PyRun_FileExFlags
        FILE* fs = _Py_wfopen(filename.c_str(), L"r");
        PyObject* pyobj = PyRun_FileExFlags(
            fs,
            filename.u8string().c_str(),
            Py_file_input,
            __private->_global->ptr(), 
            __private->_local->ptr(),
            true,
            nullptr);

        if (!pyobj)
            throw bp::error_already_set();
        result = bp::object(bp::handle<>(pyobj));
#else
        // 不支持宽字节
        result = bp::exec_file(
           filename.string().c_str(), *__private->_global, *__private->_local);
#endif

    }, exception_handler);

    return result;
}

void pyembed::exec_for(
    const std::function<void()>& action,
    const std::function<bool(const pyerror&)>& exception_handler /*= {} */)
{
    __private->exec_for(action, exception_handler);
}

boost::python::dict& pyembed::global()
{
    return *__private->_global;
}

boost::python::dict& pyembed::local()
{
    return *__private->_local;
}

void pyembed::clean()
{
    boost::python::object builtins = global()["__builtins__"];

    local().clear();
    global().clear();
    global()["__builtins__"] = builtins;
    local() = global();
}

void pyembed::write_stdout(const std::string& str)
{
    // 如果派生类没有实现此接口，那么在调试模式下通过异常通知用户，否则通过输出信息。
    const char* msg = "You need to implement the write_stdout() interface.\n";
#if defined(DEBUG) || defined(_DEBUG)
    throw std::runtime_error(msg);
#endif
    std::cout << msg;
}

void pyembed::write_stderr(const std::string& str)
{
    const char* msg = "You need to implement the write_stderr() interface.\n";
#if defined(DEBUG) || defined(_DEBUG)
    throw std::runtime_error(msg);
#endif
    std::cerr << msg;
}

std::string pyembed::readline_stdin(int size /*= -1*/)
{
    const char* msg = "You need to implement the readline_stdin() interface.\n";
#if defined(DEBUG) || defined(_DEBUG)
    throw std::runtime_error(msg);
#endif
    std::cerr << msg;

    return {};
}
