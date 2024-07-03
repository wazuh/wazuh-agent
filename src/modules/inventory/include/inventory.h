#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include "configuration.h"

class Inventory {
public:
    void run();
    int setup(const Configuration & config);
    void stop();
    std::string command(const std::string & query);
    std::string name() const;
};

#endif // INVENTORY_H

