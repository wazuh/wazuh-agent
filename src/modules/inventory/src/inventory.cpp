#include "inventory.h"
#include <iostream>

using namespace std;

void Inventory::run() {
    cout << "+ [Inventory] is running" << endl;
}

int Inventory::setup(const Configuration & config) {
    return 0;
}

void Inventory::stop() {
    cout << "- [Inventory] stopped" << endl;
}

string Inventory::command(const string & query) {
    cout << "  [Inventory] query: " << query << endl;
    return "OK";
}

string Inventory::name() const {
    return "inventory";
}
