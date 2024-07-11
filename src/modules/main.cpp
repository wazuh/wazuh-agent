#include <iostream>
#include "pool.h"
#include "configuration.h"

using namespace std;

int main() {
    Pool pool;
    Configuration config;

    pool.start();
    pool.setup(config);
    cout << endl;

    try {
        auto inventory = pool.getModule("inventory");
        inventory->command("Hello World!");
    } catch (const out_of_range & e) {
        cerr << "!  OOPS: Module not found." << endl;
    }

    cout << endl;
    pool.stop();

    return 0;
}
