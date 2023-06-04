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

#ifndef pyembed_h__
#define pyembed_h__

// export interface
#ifdef PYEMBED_SHARED_LIB
#    define PYEMBED_LIB __declspec(dllexport)
#else
#    ifdef PYEMBED_STATIC_LIB
#        define PYEMBED_LIB
#    else
#        define PYEMBED_LIB __declspec(dllimport)
#    endif // PYEMBED_BUILD_STATIC_LIB
#endif // PYEMBED_BUILD_SHARED_LIB


#include <filesystem>
#include <boost/python.hpp>
#include <boost/serialization/singleton.hpp>

//!
//! 嵌入式Python解释器
//! 
class pyembed
{
    template <typename T>
    friend T& get_pyembed();
    friend class pyembed_private;
    class pyembed_private* __private;

protected:
    PYEMBED_LIB explicit pyembed(const std::type_info& type);
    PYEMBED_LIB static   pyembed* get_ptr();

public:
    PYEMBED_LIB virtual ~pyembed();

    pyembed(const pyembed&) = delete;
    pyembed& operator=(const pyembed&) = delete;

    //! @brief 初始化解释器
    //! @param pyhome 指定Python的家目录，即标准Python库的位置，参考：Py_SetPythonHome()。
    //! @param initsigs 指定是否注册信号处理器，当作为嵌入解释器时，可能不希望执行绪被 Ctrl+C 中断。
    //! @note 该方法没有返回值，初始化失败是致命错误，将立即终止程序!
    PYEMBED_LIB void init(const std::filesystem::path& pyhome = "", bool initsigs = true);

    //! @brief 模拟发送SIGINT信号到解释器。
    //! @note 解释器如果没有注册信号处理器则无法被SIGINT信号中断。
    PYEMBED_LIB void interrupt();

    //! @brief 添加初始化时导入解释器的内建模块
    //! @param name 模块名
    //! @param initfunc 模块初始化函数，形式为: PyInit_{name}。
    //! @return 成功返回true，否则false。
    //! note 该方法应该在init()前调用。
    PYEMBED_LIB bool append_inittab(
        const char* name,
        PyObject* (*initfunc)(void));
    
    struct pymoudle {
        const char* name;
        PyObject* (*initfunc)(void);
    };

    //! @brief 添加初始化时导入解释器的内建模块
    //! @param pymoudles 模块列表
    //! @return 成功返回true，否则false。
    //! note 该方法应该在init()前调用。
    PYEMBED_LIB bool append_inittab(const std::vector<pymoudle>& pymoudles);

    //! @brief 注册扩展模块的异常处理器，提供该接口的目的主要用于暴露。
    //!        boost::python::detail::register_exception_handler()。
    //! @param handler 异常处理函数，它接收一个闭包，并返回一个bool值，其表示是否处理了异常。
    //! @note 1. 该处理函数将被解释器一直持有直到解释器被释放。
    //!       2. 可以注册多个异常处理器，boost.python在内部维护了一个异常处理链表，当Python调用C++的相关代码时，
    //!          会将异常处理链表在调用栈中展开，即最后注册的处理器最先捕获异常，没有捕获的将依次向上展开。
    //!          伪代码表示为：fristHandler(secondHandler(thirdHandler(f())));
    PYEMBED_LIB void register_exception_handler(
        const std::function<bool(std::function<void()>)>& handler);

    //! @brief 计算给定表达式的值并返回结果值
    //! @param expression Python 表达式
    //! @param exception_handler 异常处理器，发生异常时将被调用，回调返回true表示异常已处理，
    //!     否则将打印到错误输出，签名如下：
    //!     bool(const boost::python::object& pyexception, const std::string& traceback);
    //! @return 返回计算结果值
    PYEMBED_LIB boost::python::object eval(
        const std::string& expression,
        const std::function<
            bool(const boost::python::object&, 
                 const std::string&)>& exception_handler = {}
    );

    //! @brief 执行给定的代码（通常是一组表达式）并返回结果
    //! @param snippets Python 代码片段
    //! @param exception_handler 异常处理器，发生异常时将被调用，回调返回true表示异常已处理，
    //!     否则将打印到错误输出，签名如下：
    //!     bool(const boost::python::object& pyexception, const std::string& traceback);
    //! @return 返回计算结果值
    PYEMBED_LIB boost::python::object exec(
        const std::string& snippets,
        const std::function<
            bool(const boost::python::object&,
                 const std::string&)>& exception_handler = {}
    );

    //! @brief 执行包含在给定文件中的代码并返回结果
    //! @param script 文件名
    //! @param args 执行参数
    //! @param exception_handler 异常处理器，发生异常时将被调用，回调返回true表示异常已处理，
    //!     否则将打印到错误输出，签名如下：
    //!     bool(const boost::python::object& pyexception, const std::string& traceback);
    //! @return 返回计算结果值
    PYEMBED_LIB boost::python::object exec_file(
        const std::filesystem::path& script,
        const std::vector<std::string>& args = {},
        const std::function<
            bool(const boost::python::object&,
                 const std::string&)>& exception_handler = {}
    );

    //! @brief 获得解释器的全局或局部上下文
    //! @return 返回全局上下文的字典对象
    PYEMBED_LIB boost::python::dict& global();
    PYEMBED_LIB boost::python::dict& local();

    //! @brief 清除解释器状态
    //! @note 
    PYEMBED_LIB void clean();

    //! @brief sys.stdin.readline()的重定向接口
    //! @param size 要输入的字节数
    //! @note pyembed默认不会启动重定向机制，除非通过子类化并重写虚函数。
    PYEMBED_LIB virtual std::string readline_stdin(int size = -1);

    //! @brief sys.stdout.write()的重定向接口
    //! @param str 输出的内容，utf-8编码
    //! @note pyembed默认不会启动重定向机制，除非通过子类化并重写虚函数。
    PYEMBED_LIB virtual void write_stdout(const std::string& str);

    //! @brief sys.stderr.write()的重定向接口
    //! @param size 要输入的字节数
    //! @note pyembed默认不会启动重定向机制，除非通过子类化并重写虚函数。
    PYEMBED_LIB virtual void write_stderr(const std::string& str);
};

//!
//! 用于获得pyembed实例的便捷函数
//! 注意：
//!     线程不安全，应当在进入多线程之前获取实例
//!
template<class T = pyembed>
T& get_pyembed() {
    if (T::get_ptr() == nullptr)
        new T(typeid(T));
    return static_cast<T&>(*T::get_ptr());
}

#endif // pyembed_h__
