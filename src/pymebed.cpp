#include "pymebed.h"
#include <assert.h>
#include <signal.h>
#include <iostream>
#include <strstream>
#include <functional>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <string/string_conv.h>
#include <string/string_conv_easy.hpp>

namespace python = boost::python;

enum direction_type {
    pystdin,
    pystdout,
    pystderr
};

template<direction_type>
class py_redirector
{
public:
    py_redirector()
    { }

    py_redirector(std::function<void(const std::string&)> f)
        : m_write_fn(f)
    { }

    py_redirector(std::function<std::string(int size)> f)
        : m_readline_fn(f)
    { }

    void write(const std::string& text)
    {
        if (m_write_fn)
            m_write_fn(text);
    }

    std::string readline(int size = -1)
    {
        if (m_readline_fn)
            return m_readline_fn(size);
        else
            return "";
    }

public:
    std::function<void(const std::string&)> m_write_fn;
    std::function<std::string(int size)>    m_readline_fn;
};

typedef py_redirector<pystdin>  stdin_redirector;
typedef py_redirector<pystdout> stdout_redirector;
typedef py_redirector<pystderr> stderr_redirector;

//////////////////////////////////////////////////////////////////////////

class pymebed_private 
{
    pymebed* __public;
public:
    pymebed_private(pymebed* p)
        : __public(p)
    {}

    void exec_some(
        const std::function<void()>& f,
        const std::function<void(std::string)>& pyerr)
    {
        if (boost::python::handle_exception(f))
        {
            auto fncopy = _stderr->m_write_fn;
            auto stream = std::make_shared<std::ostringstream>();
            if (pyerr)
            {
                _stderr->m_write_fn = [&](const std::string& text) {
                    *stream << text;
                };
            }

            if (PyErr_Occurred())
            {
                PyErr_Print();
            }
            else
            {
                auto msg =
                    "A C++ exception was thrown for which "
                    "there was no exception handler registered.\n";
                if (pyerr)
                    pyerr(msg);
                else
                    std::cerr << msg;
            }

            if (pyerr)
            {
                pyerr(stream->str());
                std::swap(_stderr->m_write_fn, fncopy);
            }
        }
    }

    static void signal_handler(int signum)
    {
        if (signum == SIGINT)
            PyErr_SetInterrupt();

        signal(signum, &pymebed_private::signal_handler);
    }

    static pymebed_private* get_instance() {
        return get_pymebed().__private;
    }

public:
    bool           _initsigs;
    python::object _main_module;
    python::dict   _main_global;

    std::shared_ptr<stdin_redirector>  _stdin;
    std::shared_ptr<stdout_redirector> _stdout;
    std::shared_ptr<stderr_redirector> _stderr;
};

//////////////////////////////////////////////////////////////////////////

std::shared_ptr<stdin_redirector>  get_stdin(){
    return pymebed_private::get_instance()->_stdin;
}
std::shared_ptr<stdout_redirector> get_stdout() {
    return pymebed_private::get_instance()->_stdout;
}
std::shared_ptr<stderr_redirector> get_stderr() {
    return pymebed_private::get_instance()->_stderr;
}

BOOST_PYTHON_MODULE(redirector)
{
    using namespace boost::python;

    class_<stdin_redirector>("stdin",
        "This class redirects python's standard input to the console.",
        init<>("initialize the stdin_redirector."))
        .def("__init__", make_constructor(get_stdin), "initialize the redirector.")
        .def("readline", &stdin_redirector::readline, arg("size") = -1, "readline sys.stdin redirection.");

    class_<stdout_redirector>("stdout",
        "This class redirects python's standard output to the console.",
        init<>("initialize the stdout_redirector."))
        .def("__init__", make_constructor(get_stdout), "initialize the redirector.")
        .def("write", &stdout_redirector::write, "write sys.stdout redirection.");

    class_<stderr_redirector>("stderr",
        "This class redirects python's error output to the console.",
        init<>("initialize the stderr_redirector."))
        .def("__init__", make_constructor(get_stderr), "initialize the redirector.")
        .def("write", &stderr_redirector::write, "write sys.stderr redirection.");
}

//////////////////////////////////////////////////////////////////////////

pymebed::pymebed()
{
    __private = new pymebed_private(this);
}

pymebed::~pymebed()
{
    // Boost.Python doesn't support Py_Finalize yet, so don't call it!

    delete __private;
           __private = nullptr;
}

void pymebed::init(
    const std::filesystem::path& pyhome /*= ""*/,
    bool initsigs /*= true*/)
{
    __private->_stdin = std::make_shared<stdin_redirector>(
        [&](int size) -> std::string {
            return readline_stdin(size);
        });
    __private->_stdout = std::make_shared<stdout_redirector>(
        [&](const std::string& text) {
            write_stdout(text);
        });
    __private->_stderr = std::make_shared<stderr_redirector>(
        [&](const std::string& text) {
            write_stderr(text);
        });

    // Register the module with the interpreter; must be called before Py_Initialize.
    /* ToDo: C style cast to avoid compiler warning about
       deprecated conversion from string constant to 'char*' */
    if (PyImport_AppendInittab((char*)"redirector", PyInit_redirector) == -1) {
        throw std::runtime_error(
            "Failed to add embedded_python to python's "
            "interpreter builtin modules");
    }

    if (!pyhome.empty())
        Py_SetPythonHome(pyhome.c_str());

    // Set initsigs to 0 to skips initialization registration of signal handlers.
    // It is a fatal error if the initialization fails.
    // https://docs.python.org/3/c-api/init.html?highlight=py_initializeex#c.Py_InitializeEx
    // Bug: 3.8, 3.9. 3.10
    // https://bugs.python.org/issue41686
    Py_InitializeEx(__private->_initsigs = initsigs);
    PyEval_InitThreads();

    // Retrieve the main module
    __private->_main_module = python::import("__main__");

    // Retrieve the main module's namespace
    __private->_main_global = python::dict(__private->_main_module.attr("__dict__"));

    {
        static const char* const redirect_py =
            "import sys\n"
            "import redirector\n"
            "sys.stdin  = redirector.stdin()\n"
            "sys.stdout = redirector.stdout()\n"
            "sys.stderr = redirector.stderr()\n";

#if 1
        /* FixMe: exception thrown, mmh - seems a bug in boost.python, see
         * http://www.nabble.com/Problems-with-Boost::Python-Embedding-Tutorials-td18799129.html */
        exec(redirect_py);
#else
        PyRun_String(redirect_py,
            Py_file_input,
            __private->_main_global.ptr(), __private->_main_global.ptr());
#endif
    }
}

void pymebed::interrupt()
{
    assert(__private->_initsigs);

    // 若初始化时没有注册信号处理(Py_InitializeEx(0)), 则无法通过PyErr_SetInterrupt()中断
    // https://docs.python.org/3/c-api/exceptions.html?highlight=pyerr_setinterrupt#c.PyErr_SetInterrupt
    PyErr_SetInterrupt();
}

bool pymebed::append_inittab(
    const char* name, PyObject* (*initfunc)(void))
{
    // Register the module with the interpreter
    if (PyImport_AppendInittab(name, initfunc) == -1)
    {
        write_stderr(
            boost::str(boost::format(
                "Failed to add {1} to the interpreter's "
                "builtin modules") % name));
        return false;
    }

    return true;
}

bool pymebed::append_inittab(const std::vector<pymoudle>& pymoudles)
{
    for (const auto& item : pymoudles ) 
    {
        if (!append_inittab(item.name, item.initfunc))
            return false;
    }

    return true;
}

void pymebed::register_exception_handler(
    const std::function<bool(std::function<void()>)>& handler)
{
    namespace bpy = boost::python::detail;
    bpy::register_exception_handler([=](
        bpy::exception_handler const& e,
        boost::function0<void> const& f) -> bool {
            return handler(f);
        });
}

boost::python::object pymebed::eval(
    const std::string& expression,
    const std::function<void(std::string)>& pyerr /*= {}*/)
{
    boost::python::object result;
    __private->exec_some([&]() {
        result = python::eval(expression.c_str(),
            __private->_main_global,
            __private->_main_global);
        },
        pyerr);
    return result;
}

boost::python::object pymebed::exec(
    const std::string& command,
    const std::function<void(std::string)>& pyerr /*= {}*/)
{
    boost::python::object result;
    __private->exec_some([&]() {
        result = python::exec(command.c_str(),
            __private->_main_global, 
            __private->_main_global);
        },
        pyerr);
    return result;
}

boost::python::object pymebed::exec_file(
    const std::filesystem::path& script, 
    const std::vector<std::string>& args /*= {}*/,
    const std::function<void(std::string)>& pyerr /*= {}*/)
{
    // 脚本文件获取绝对路径
    // PySys_SetArgvEx()要求参数argv[0]必须为脚本所在目录
    // https://docs.python.org/3/c-api/init.html?highlight=pysys_setargvex#c.PySys_SetArgvEx
    std::error_code ecode;
    std::filesystem::path filename =
        std::filesystem::canonical(script);
    std::filesystem::path directory = filename.parent_path();

    boost::python::object result;
    __private->exec_some([=, &result]()
    {
        // 安装参数
        std::vector<wchar_t*> argv(1, (wchar_t*)directory.c_str()) ;
        std::vector<std::wstring> argd;
                                  argd.reserve(args.size());
        for (auto& a : args)
        {
            using namespace util::conv::easy;
            argd.push_back(_2wstr(a, util::format_utf8));
            argv.push_back((wchar_t*)argd.back().c_str());
        }

        PySys_SetArgvEx(argv.size(), (wchar_t**)&argv[0], 1);

        // 运行脚本
        // 是否需要显式关闭文件, 或者PyRun_FileExFlags()的closeit参数应该为true?
        FILE* fs = _Py_wfopen(filename.c_str(), L"r");
        PyObject* pyobj = PyRun_File(fs,
            filename.u8string().c_str(),
            Py_file_input,
            __private->_main_global.ptr(), 
            __private->_main_global.ptr());
        if (!pyobj)
            throw python::error_already_set();

        // 不支持宽字节
        //result = python::exec_file(
        //    filename.string().c_str(), m_main_global, m_main_global);

        // 重置参数
        argv.clear();
        argv.push_back(L"");
        PySys_SetArgvEx(argv.size(), (wchar_t**)&argv[0], 1);

        result = python::object(python::handle<>(pyobj));
    },
    pyerr);

    return result;
}

boost::python::dict& pymebed::global()
{
    return __private->_main_global;
}

void pymebed::write_stdout(const std::string& bytes)
{
    Py_BEGIN_ALLOW_THREADS
    std::string output;
    std::cout << util::conv::utf8_to_string(bytes, output);
    Py_END_ALLOW_THREADS
}

void pymebed::write_stderr(const std::string& bytes)
{
    Py_BEGIN_ALLOW_THREADS
    std::string output;
    std::cerr << util::conv::utf8_to_string(bytes, output);
    Py_END_ALLOW_THREADS
}

std::string pymebed::readline_stdin(int size /*= -1*/)
{
    std::string result;
    Py_BEGIN_ALLOW_THREADS
    std::cin >> result;
    if (size != -1 && int(result.size()) > size)
        result.resize(size);
    Py_END_ALLOW_THREADS
    return result;
}
