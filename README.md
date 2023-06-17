# Pyembed

一个简单的C++库，用于简化嵌入Python解释器到C++项目，基于Boost.Python封装。

使用Boost.Python将解释器嵌入到C++应用程序，通常我们需要直接使用CPython的API（如`Py_Initialize(), Py_SetPythonHome()` 等），还会遇到如下问题：

- 如何重定向解释器的标准输入、输出？
- 如何在C++中处理Python异常，而不是打印到标准输出?
- 如何处理非ASCII字符的文件名(比如：在Windows下含有中文的文件名)?
- 如何在执行Python文件时，指定命令行参数?
- 如何在Visual Studio调试器中查看Python对象(PyObject*)的值?

**Pyembed** 的出现就是为了解决以上问题，更多细节请参阅：[C/C++脚本化: 使用Python作为现有项目的扩展](https://7-0.cc/cpp_scripting_exploration_2/)。

## Build && Install

环境需求

- C++ 17
- Boost （建议版本1.6及以上）
- Python（建议版本3.8及以上）

> 注意:
> 目前已知Boost 1.81, 1.82存在Bug，建议选择靠后的发行版。

### Windows

1. 克隆仓库

```shell
$ git clone https://github.com/xxxxxx.git pyembed
```

2. 构建并编译(需要修改成自己的配置)

```shell
$ cd pyembed && mkdir build && cd build
$ cmake -G "Visual Studio 16 2019" -A Win32 .. -DCMAKE_INSTALL_PREFIX:PATH="G:\install\pyembed" -DCMAKE_PREFIX_PATH="G:\local\boost\boost_1_82_0-msvc-14.2-32;G:\local\python\3.10.11"
$ cmake --build . --config Debug
$ cmake --build . --config RelWithDebInfo
```

3. 安装

```shell
$ cmake --install . --config Debug
$ cmake --install . --config RelWithDebInfo
```

### Linux

1. 克隆仓库

```shell
$ git clone https://github.com/xxxxxx.git pyembed
```

2. 构建、编译、安装(需要修改成自己的配置)

```shell
$ cd pyembed && mkdir build && cd build
$ cmake .. -DCMAKE_INSTALL_PREFIX:PATH="$HOME/installed/pyembed-0.2" -DCMAKE_BUILD_TYPE=RelWithDebInfo
$ cmake --build . --target install --config RelWithDebInfo
```

## Run Environment

对于嵌入式解释器来说，通常使用嵌入包（Windows embeddable package (32 or 64 bit)）作为运行环境，与完整包（Windows installer (32 or 64 bit)）不同的是前者为最小环境包，仅包含运行所需的文件，不提供开发所需的头文件与库文件。

因此在开发时需要用到的是完整包，而实际生产环境中需要用到的是嵌入包，通常可以将嵌入包中的文件解压至程序根目录，也可以放在一个独立的目录中。

下面是建议的可运行环境结构（带星号的部分是可选文件）：

```txt
bin
|-- python
|   |-- LICENSE.txt
|   |-- python.cat
|   |-- python.exe  *
|   |-- pythonw.exe *
|   |-- python3.dll *
|   |-- python310.dll
|   |-- python310.zip
|   |-- python310._pth
|   |-- libcrypto-1_1.dll
|   |-- libffi-7.dll
|   |-- libssl-1_1.dll
|   |-- sqlite3.dll
|   |-- vcruntime140.dll
|   `-- *.pyd
| 
|-- python310.dll
|-- python310._pth
`-- simple_for_pyembed.exe
```

bin/python310._pth:

```_pth
python/python310.zip
python
.
```
