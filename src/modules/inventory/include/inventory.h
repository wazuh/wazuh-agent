#ifndef INVENTORY_H
#define INVENTORY_H

#include <iostream>
#include <string>
#include "configuration.h"

using namespace std;

struct Inventory {
    void run();
    int setup(const Configuration& config);
    void stop();
    string command(const string& query);
    string name() const;
};

#endif // INVENTORY_H
