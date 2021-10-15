#include "pymebed.h"

#include <boost/python.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <functional>
#include <iostream>

#include "filesystem/path_util.h"

namespace python = boost::python;

namespace embedded_python
{
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

        void write(const std::string& text)
        {
            if (m_write_fn)
                m_write_fn(text);
        }

        std::string readline(int size = -1)
        {
            return "hello\n";
        }

    public:
        std::function<void(const std::string&)> m_write_fn;
        //std::function<void(int size)> m_write_fn;
    };

    typedef py_redirector<pystdin>  stdin_redirector;
    typedef py_redirector<pystdout> stdout_redirector;
    typedef py_redirector<pystderr> stderr_redirector;


    static std::shared_ptr<stdin_redirector>  get_stdin_redirector()
    {
        return std::make_shared<stdin_redirector>();
    }

    static std::shared_ptr<stdout_redirector> get_stdout_redirector();
    static std::shared_ptr<stderr_redirector> get_stderr_redirector();
}

BOOST_PYTHON_MODULE(redirector)
{
    using namespace embedded_python;
    using namespace python;

    class_<stdin_redirector>("stdin",
        "This class redirects python's standard input "
        "to the console.",
        init<>("initialize the redirector."))
        .def("__init__", make_constructor(get_stdin_redirector), "initialize the redirector.")
        .def("readline", &stdin_redirector::readline, arg("size") = -1, "readline sys.stdin redirection.");

    class_<stdout_redirector>("stdout",
        "This class redirects python's standard output "
        "to the console.",
        init<>("initialize the redirector."))
        .def("__init__", make_constructor(get_stdout_redirector), "initialize the redirector.")
        .def("write", &stdout_redirector::write, "write sys.stdout redirection.");

    class_<stderr_redirector>("stderr",
        "This class redirects python's error output "
        "to the console.",
        init<>("initialize the redirector."))
        .def("__init__", make_constructor(get_stderr_redirector), "initialize the redirector.")
        .def("write", &stderr_redirector::write, "write sys.stderr redirection.");
}

class EmbeddedPython
    : public boost::serialization::singleton<EmbeddedPython>
{
public:
    EmbeddedPython()
    {}

    ~EmbeddedPython() {
        // Boost.Python doesn't support Py_Finalize yet, so don't call it!
    }

    void init()
    {
        __asm int 3;

        m_stdout = std::make_shared<embedded_python::stdout_redirector>(
            [&](const std::string& text) {
                write_stdout(text);
            });
        m_stderr = std::make_shared<embedded_python::stderr_redirector>(
            [&](const std::string& text) {
                write_stderr(text);
            });

        // Register the module with the interpreter; must be called before Py_Initialize.
        /* ToDo: C style cast to avoid compiler warning about
           deprecated conversion from string constant to 'char*' */
        if (PyImport_AppendInittab((char*)"redirector", PyInit_redirector) == -1) {
            throw std::runtime_error("Failed to add embedded_python to python's "
                "interpreter builtin modules");
        }

        Py_Initialize();

        // Retrieve the main module
        m_main_module = python::import("__main__");

        // Retrieve the main module's namespace
        m_main_global = python::dict(m_main_module.attr("__dict__"));

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
                m_main_global.ptr(), m_main_global.ptr());
#endif
        }
    }

    python::object exec(const std::string& command)
    {
        python::object result;
        if (python::handle_exception([&]() {
            result = python::exec(
                command.c_str(), m_main_global, m_main_global);
            }))
        {
            if (PyErr_Occurred())
            {
                PyErr_Print();
                PyErr_Clear();
            }
            else
            {
                std::cerr << "A C++ exception was thrown for which "
                    << "there was no exception handler registered.\n";
            }
        }

            return result;
    }

    python::object exec_file(const std::string& script)
    {
        python::object result;
        if (python::handle_exception([&]() {
            result = python::exec_file(
                script.c_str(), m_main_global, m_main_global);
            }))
        {
            if (PyErr_Occurred())
            {
                PyErr_Print();
                PyErr_Clear();
            }
            else
            {
                std::cerr << "A C++ exception was thrown for which "
                    << "there was no exception handler registered.\n";
            }
        }

            return result;
    }

    virtual void write_stdout(const std::string& text) {
        std::cout << text;
    }

    virtual void write_stderr(const std::string& text) {
        std::cerr << text;
    }

    virtual std::string readline_stdin(int size = -1)
    {
        std::string result;
        std::cin >> result;
        if (size != -1 && result.size() > size)
            result.resize(size);

        return result;
    }

    std::shared_ptr<embedded_python::stdout_redirector> get_stdout() { return m_stdout; }
    std::shared_ptr<embedded_python::stderr_redirector> get_stderr() { return m_stderr; }

private:
    python::object m_main_module;
    python::dict   m_main_global;

    std::shared_ptr<embedded_python::stdin_redirector>  m_stdin;
    std::shared_ptr<embedded_python::stdout_redirector> m_stdout;
    std::shared_ptr<embedded_python::stderr_redirector> m_stderr;
};

std::shared_ptr<embedded_python::stdout_redirector> embedded_python::get_stdout_redirector()
{
    return EmbeddedPython::get_mutable_instance().get_stdout();
}

std::shared_ptr<embedded_python::stderr_redirector> embedded_python::get_stderr_redirector()
{
    return EmbeddedPython::get_mutable_instance().get_stderr();
}

int test()
{
    EmbeddedPython::get_mutable_instance().init();

    std::cout << "Testing embedded python:\n";
    EmbeddedPython::get_mutable_instance().exec("print(\"Hello\")\n");
    EmbeddedPython::get_mutable_instance().exec("print(\"World\")\n\n");

    // rise exception
    EmbeddedPython::get_mutable_instance().exec("Hello\n");
    EmbeddedPython::get_mutable_instance().exec("print(input('input:'))\n");

    return 0;
}

