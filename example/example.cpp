#include <iostream>
#include "pymebed.h"

int main(int argc, char* argv[])
{
        get_pymebed().init();

        std::cout << "Testing embedded python:\n";
        get_pymebed().exec("print(\"Hello\")\n");
        get_pymebed().exec("print(\"World\")\n\n");

        // rise exception
        get_pymebed().exec("Hello\n");
        get_pymebed().exec("print(input('input:'))\n");

        auto result = get_pymebed().exec_file(
            L"script1.py",
            { L"-s=ooo", L"-s", L"-kj" });

        int a = boost::python::extract<int>(get_pymebed().global()["number"]);

    return 0;
}