#ifndef NETWORK_REPAIR_H
#define NETWORK_REPAIR_H
#include <string>
#include <vector>
#include <unordered_map>

struct RepairRequest {
    std::string id;
    int u;
    int v;
    int urgency;
    int cost;

    RepairRequest();
    RepairRequest(const std::string& id, int u, int v, int urgency, int cost);
};


bool higherPriority(const RepairRequest& a, const RepairRequest& b);

class DisjointSetUnion {
private:
    std::vector<int> parent;
    std::vector<int> sz;
    int componentCount;

public:
    DisjointSetUnion();
    explicit DisjointSetUnion(int n);

    void init(int n);
    int find(int x);
    bool unite(int a, int b);
    bool connected(int a, int b);
    int componentSize(int x);
    int getComponentCount() const;
};


class BinaryHeap {
private:
    std::vector<RepairRequest> data;
    std::unordered_map<std::string, int> indexOf;

    void siftUp(int i);
    void siftDown(int i);
    void swapAt(int i, int j);

public:
    BinaryHeap();

    bool empty() const;
    int size() const;
    bool contains(const std::string& id) const;

    void insert(const RepairRequest& r);
    RepairRequest extractTop();
    void updatePriority(const std::string& id, int newUrgency, int newCost);

    std::vector<RepairRequest> snapshot() const;
};


class NetworkRepair {
private:
    DisjointSetUnion dsu;
    BinaryHeap heap;
    long long totalCost;

    std::unordered_map<std::string, bool> seenIds;

public:
    NetworkRepair();
    void loadNetwork(const std::string& filename);
    void runOperations(const std::string& filename);
    void opAddRepair(const std::string& id, int u, int v, int urgency, int cost);
    void opUpdateRepair(const std::string& id, int newUrgency, int newCost);
    void opPrintPending();
    void opRepairNext();
    void opConnected(int u, int v);
    void opComponentSize(int u);
    void opComponentCount();
    void opTotalCost();
    static void sortRequests(std::vector<RepairRequest>& v);
};

#endif