#include <iostream>
#include <filesystem>
#include "pymebed.h"

int main(int argc, char* argv[])
{
    get_pymebed().init();

    std::cout << "Testing embedded python:\n";
    get_pymebed().exec("print(\"Hello\")\n");
    get_pymebed().exec("print(\"World\")\n\n");

    // rise exception
    get_pymebed().exec("Hello\n", [](std::string error) {
        std::cerr << error;
    });
    get_pymebed().exec("print(input('input:'))\n");

    // loop, Ctrl+C
    get_pymebed().exec(
        "import time\n"
        "for i in range(1000) :\n"
        "   print(i)\n"
        "   time.sleep(0.5)\n");

    std::filesystem::path script = __FILE__;
    script = script.parent_path() / "scripts";

    get_pymebed().exec_file(script / L"中文.py", {});

    auto result = get_pymebed().exec_file(
        script / "script_args.py", { "-s=ooo", "-s", "-kj" });

    int a = boost::python::extract<int>(get_pymebed().global()["number"]);

    return 0;
}