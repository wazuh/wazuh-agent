#include <iostream>
#include "configuration.hpp"
#include "inventory.hpp"
#include "moduleManager.hpp"

using namespace std;

int modulesExec() {
    ModuleManager moduleManager;
    Configuration config;
    bool running{true};

    moduleManager.addModule(Inventory::instance());
    moduleManager.setup(config);
    moduleManager.start();

    while(running) {
        sleep(1);
    }

    moduleManager.stop();

    return 0;
}
