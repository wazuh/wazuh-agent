#include <iostream>
#include <csignal>
<<<<<<< HEAD
<<<<<<<< HEAD:src/modules/modules.cpp
#include "pool.hpp"
#include "configuration.hpp"

using namespace std;

static Pool* global_pool = nullptr;
========
#include "moduleManager.h"
=======
>>>>>>> b1a318f55 (feat: Added module manager unit tests)
#include "configuration.h"
#include "inventory.h"
#include "moduleManager.h"

using namespace std;

ModuleManager* g_moduleManager = nullptr;
>>>>>>>> 1a0566c11 (refactor: renamed class Pool to ModulesManager):src/modules/src/main.cpp

static void signalHandler(int signal) {

    switch (signal) {
        case SIGHUP:
        case SIGINT:
        case SIGTERM:
            cout << endl << "Signal received: " << signal << ". Stopping modules..." << endl;
            if (g_moduleManager) {
                g_moduleManager->stop();
            }
            exit(signal);
        default:
            cerr << "unknown signal (" << signal << ")" << endl;
    }
}

<<<<<<<< HEAD:src/modules/modules.cpp
int modulesExec(){
    Pool pool;
    global_pool = &pool;
========
int main() {
    ModuleManager moduleManager;
    g_moduleManager = &moduleManager;
>>>>>>>> 1a0566c11 (refactor: renamed class Pool to ModulesManager):src/modules/src/main.cpp
    Configuration config;

    signal(SIGHUP, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    moduleManager.addModule(Inventory::instance());
    moduleManager.setup(config);
    moduleManager.start();

    try {
        auto inventory = moduleManager.getModule("Inventory");
        inventory->command("Hello World!");
    } catch (const out_of_range & e) {
        cerr << "!  OOPS: Module not found." << endl;
    }

    pause();

    return 0;
}
