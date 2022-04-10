#ifndef pymebed_h__
#define pymebed_h__

#include <filesystem>
#include <boost/python.hpp>
#include <boost/serialization/singleton.hpp>

//!
//! 嵌入Python解释器的包装类
//! 
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

    //! @brief 初始化Python解释器
    //! @param pyhome 默认的“home”目录，即标准Python库的位置。
    //! @param initsigs 是否注册信号处理器
    //! @note 初始化失败是致命错误, 将终止程序
    void init(const std::filesystem::path& pyhome = "", bool initsigs = true);

    //! @brief 模拟发送SIGINT信号到Python
    //! @note Python解释器如果没有注册信号处理器则无法被SIGINT信号中断.
    void interrupt();

    //! @brief 添加Python初始化时导入解释器的内建模块
    //! @param name 模块名
    //! @param initfunc 模块初始化函数, 形式为: PyInit_{name}.
    //! @return 成功返回true, 否则false.
    //! note 必须在init()前调用
    bool append_inittab(
        const char* name,
        PyObject* (*initfunc)(void));
    
    struct pymoudle
    {
        const char* name;
        PyObject* (*initfunc)(void);
    };

    //! @brief 添加Python初始化时导入解释器的内建模块
    //! @param pymoudles 模块列表
    //! @return 成功返回true, 否则false.
    //! note 必须在init()前调用
    bool append_inittab(const std::vector<pymoudle>& pymoudles);

    //! @brief 注册扩展模块的异常处理器, 提供该接口的目的主要用于暴露
    //!        boost::python::detail::register_exception_handler().
    //! @param handler 异常处理函数, 它接收一个闭包, 并返回一个bool值, 其表示是否处理了异常.
    //! @note 该处理函数将被解释器一直持有直到解释器被释放.
    void register_exception_handler(
        const std::function<bool(std::function<void()>)>& handler);

    //! @brief 计算给定表达式的值并返回结果值
    //! @param expression Python 表达式
    //! @param pyerr 用于捕获Python错误信息的回调, 若该参数无效则通过write_stderr处理.
    //! @return 返回计算结果值
    boost::python::object eval(
        const std::string& expression,
        const std::function<void(std::string)>& pyerr = {});

    //! @brief 执行给定的代码(通常是一组语句)并返回结果
    //! @param command Python 代码
    //! @param pyerr 用于捕获Python错误信息的回调, 若该参数无效则通过write_stderr处理.
    //! @return 返回计算结果值
    boost::python::object exec(
        const std::string& command,
        const std::function<void(std::string)>& pyerr = {});

    //! @brief 执行包含在给定文件中的代码并返回结果
    //! @param script 文件名
    //! @param args 执行参数
    //! @param pyerr 用于捕获Python错误信息的回调, 若该参数无效则通过write_stderr处理.
    //! @return 返回计算结果值
    boost::python::object exec_file(
        const std::filesystem::path& script,
        const std::vector<std::string>& args = {},
        const std::function<void(std::string)>& pyerr = {});

    //! @brief 返回一个包含解释器当前全局上下文的字典
    //! @return
    boost::python::dict& global();

    //! @brief 分别是stdin, stdout, stderr 的重定向实现.
    //! @param bytes
    //! @param size
    virtual std::string readline_stdin(int size = -1);
    virtual void write_stdout(const std::string& bytes);
    virtual void write_stderr(const std::string& bytes);
};

//! @brief 用于获得pymebed实例的便捷函数
//! @tparam T
//! @return
template<class T = pymebed>
T& get_pymebed() {
    return T::get_mutable_instance();
}

#endif // pymebed_h__
