#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <boost/algorithm/string.hpp>
using namespace std;

int main()
{
    using namespace boost::lambda;
    typedef std::istream_iterator<int> in;

    // std::for_each(
    //     in(std::cin), in(), std::cout << (_1 * 3) << " ");

    std::istream_iterator<int> input(std::cin), end;
    std::for_each(input, end, [](int x)
                  { std::cout << (x * 3) << " "; });

    std::string text = "hello, world!";
    boost::to_upper(text);
    std::cout << text << std::endl; // Should output "HELLO, WORLD!"
    return 0;
}
