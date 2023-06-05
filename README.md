# pyembed

一个简单的C++库，用于简化嵌入Python解释器到C++项目，基于Boost.Python封装。

使用Boost.Python将解释器嵌入到C++应用程序，通常我们需要直接使用CPython的API（如`Py_Initialize(), Py_SetPythonHome()` 等），还会遇到如下问题：

- 如何重定向标准输入、输出？
- 如何在C++中处理Python异常，而不是打印到标准输出?
- 如何在Visual Studio调试器中查看Python对象(PyObject*)的值?
- 如何处理非ASCII字符的文件名(比如：在Windows下含有中文的文件名)?
- 如何在执行Python文件时，指定命令行参数?

**pymebed**的出现就是为了解决以上问题。


## 快速开始

环境需求

- Boost （建议版本1.6及以上）
- Python（建议版本3.8及以上）

### Windows

```shell
$ git clone https://github.com/xxxxxx.git pymebed
$ cd pymebed && mkdir build && cd build
$ cmake -G "Visual Studio 16 2019" -A Win32 .. -DCMAKE_INSTALL_PREFIX:PATH="G:\install\pymebed" -DCMAKE_PREFIX_PATH="G:\local\boost\boost_1_82_0-msvc-14.2-32;G:\local\python\3.10.11"
$ cmake --build   . --config RelWithDebInfo
$ cmake --install . --config RelWithDebInfo
```