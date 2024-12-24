#include <iostream>
#include <string>

#include "OSLogWrapper.h"

int main()
{
    std::cout << "Reading logs with OSLog..." << std::endl;
    std::cout << getLog() << std::endl;
    return 0;
}
