#ifndef pymebed_h__
#define pymebed_h__

#include <filesystem>
#include <boost/python.hpp>
#include <boost/serialization/singleton.hpp>

class pymebed : public 
    boost::serialization::singleton<pymebed>
{
    friend class pymebed_private;
    class pymebed_private* __private;
public:
    pymebed();
    virtual ~pymebed();

    pymebed(const pymebed&) = delete;
    pymebed& operator=(const pymebed&) = delete;

    void init(const std::filesystem::path& pyhome = L"");
    void interrupt();

    boost::python::object exec(const std::string& command);
    boost::python::object exec_file(
        const std::filesystem::path& script,
        const std::vector<std::wstring>& args);
    boost::python::dict& global();

    virtual void write_stdout(const std::string& text);
    virtual void write_stderr(const std::string& text);
    virtual std::string readline_stdin(int size = -1);
};

template<class T = pymebed>
T& get_pymebed() {
    return T::get_mutable_instance();
}

#endif // pymebed_h__
