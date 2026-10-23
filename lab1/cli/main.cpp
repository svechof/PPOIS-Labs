#include "cli.h"

#include <iostream>

int main() {
    cli::Application application(std::cin, std::cout);
    application.run();
    return 0;
}
