#include <iostream>
#include <csignal>
#include "pool.hpp"
#include "configuration.hpp"

using namespace std;

static Pool* global_pool = nullptr;

static void signalHandler(int signal) {

    switch (signal) {
        case SIGHUP:
        case SIGINT:
        case SIGTERM:
            cout << endl << "!  Signal received: " << signal << ". Stopping modules..." << endl;
            if (global_pool) {
                global_pool->stop();
            }
            exit(signal);
        default:
            cerr << "unknown signal (" << signal << ")" << endl;
    }
}

int modulesExec(){
    Pool pool;
    global_pool = &pool;
    Configuration config;

    signal(SIGHUP, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    pool.setup(config);
    pool.start();

    try {
        auto inventory = pool.getModule("Inventory");
        inventory->command("Hello World!");
    } catch (const out_of_range & e) {
        cerr << "!  OOPS: Module not found." << endl;
    }

    pause();

    return 0;
}
