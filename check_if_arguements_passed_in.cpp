// This code is C++ code to check if any arguements were passed in on the command prompt

#include <iostream>

int main(int argc, char *argv[])
{
    // argc is the number of arguments passed in - this includes the file name itself
    std::cout << "Number of arguments passed: " << argc - 1 << std::endl;
    std::cout << std::endl
              << std::endl;

    int arguements_passed_in = argc - 1;

    if (arguements_passed_in == 0)
    {
        std::cout << "No arguments passed in." << std::endl;
    }
    else if (arguements_passed_in == 2)
    {
        std::cout << "TWO argument passed in." << std::endl;
    }
    else
    {
        std::cout << "We're going into the loop now... arguments passed in:" << std::endl;
        for (int i = 1; i < argc; i++)
        {
            std::cout << "Argument " << i << ": " << argv[i] << std::endl;
        }
    }

    return 0;
}
