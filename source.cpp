#include "NetworkRepair.h"

int main() {
    NetworkRepair app;
    app.loadNetwork("network.txt");
    app.runOperations("operations.txt");
    return 0;
}