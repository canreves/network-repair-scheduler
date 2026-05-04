#include "NetworkRepair.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

RepairRequest::RepairRequest()
    : id(""), u(0), v(0), urgency(0), cost(0) {}

RepairRequest::RepairRequest(const std::string& id_, int u_, int v_, int urgency_, int cost_)
    : id(id_), u(u_), v(v_), urgency(urgency_), cost(cost_) {}

bool higherPriority(const RepairRequest& a, const RepairRequest& b) {
    if (a.urgency != b.urgency) return a.urgency > b.urgency;
    if (a.cost != b.cost)       return a.cost    < b.cost;
    return a.id < b.id;
}


DisjointSetUnion::DisjointSetUnion() : componentCount(0) {}

DisjointSetUnion::DisjointSetUnion(int n) {
    init(n);
}

void DisjointSetUnion::init(int n) {
    parent.assign(n + 1, 0);
    sz.assign(n + 1, 1);
    for (int i = 0; i <= n; ++i) parent[i] = i;
    componentCount = n;
}

int DisjointSetUnion::find(int x) {
    int root = x;
    while (parent[root] != root) root = parent[root];
    while (parent[x] != root) {
        int nxt = parent[x];
        parent[x] = root;
        x = nxt;
    }
    return root;
}

bool DisjointSetUnion::unite(int a, int b) {
    int ra = find(a);
    int rb = find(b);
    if (ra == rb) return false;
    if (sz[ra] < sz[rb]) std::swap(ra, rb);
    parent[rb] = ra;
    sz[ra] += sz[rb];
    componentCount--;
    return true;
}

bool DisjointSetUnion::connected(int a, int b) {
    return find(a) == find(b);
}

int DisjointSetUnion::componentSize(int x) {
    return sz[find(x)];
}

int DisjointSetUnion::getComponentCount() const {
    return componentCount;
}


BinaryHeap::BinaryHeap() {}

bool BinaryHeap::empty() const { return data.empty(); }
int  BinaryHeap::size()  const { return (int)data.size(); }

bool BinaryHeap::contains(const std::string& id) const {
    return indexOf.find(id) != indexOf.end();
}

void BinaryHeap::swapAt(int i, int j) {
    std::swap(data[i], data[j]);
    indexOf[data[i].id] = i;
    indexOf[data[j].id] = j;
}

void BinaryHeap::siftUp(int i) {
    while (i > 0) {
        int parentIdx = (i - 1) / 2;
        if (higherPriority(data[i], data[parentIdx])) {
            swapAt(i, parentIdx);
            i = parentIdx;
        } else break;
    }
}

void BinaryHeap::siftDown(int i) {
    int n = (int)data.size();
    while (true) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        int best = i;
        if (l < n && higherPriority(data[l], data[best])) best = l;
        if (r < n && higherPriority(data[r], data[best])) best = r;
        if (best != i) {
            swapAt(i, best);
            i = best;
        } else break;
    }
}

void BinaryHeap::insert(const RepairRequest& r) {
    data.push_back(r);
    int i = (int)data.size() - 1;
    indexOf[r.id] = i;
    siftUp(i);
}

RepairRequest BinaryHeap::extractTop() {
    RepairRequest top = data[0];
    indexOf.erase(top.id);
    int n = (int)data.size();
    if (n == 1) {
        data.pop_back();
        return top;
    }
    data[0] = data.back();
    indexOf[data[0].id] = 0;
    data.pop_back();
    siftDown(0);
    return top;
}

void BinaryHeap::updatePriority(const std::string& id, int newUrgency, int newCost) {
    auto it = indexOf.find(id);
    if (it == indexOf.end()) return;
    int i = it->second;
    data[i].urgency = newUrgency;
    data[i].cost    = newCost;
    siftUp(i);
    siftDown(indexOf[id]);
}

std::vector<RepairRequest> BinaryHeap::snapshot() const {
    return data;
}


NetworkRepair::NetworkRepair() : totalCost(0) {}


void NetworkRepair::sortRequests(std::vector<RepairRequest>& v) {
    int n = (int)v.size();
    if (n < 2) return;

    std::vector<std::pair<int,int>> stack;
    stack.push_back({0, n - 1});

    while (!stack.empty()) {
        auto range = stack.back();
        stack.pop_back();
        int lo = range.first;
        int hi = range.second;
        if (lo >= hi) continue;

        int mid = lo + (hi - lo) / 2;
        if (higherPriority(v[mid], v[lo])) std::swap(v[mid], v[lo]);
        if (higherPriority(v[hi],  v[lo])) std::swap(v[hi],  v[lo]);
        if (higherPriority(v[mid], v[hi])) std::swap(v[mid], v[hi]);
        RepairRequest pivot = v[hi];

        int i = lo - 1;
        for (int j = lo; j < hi; ++j) {
            if (higherPriority(v[j], pivot)) {
                ++i;
                std::swap(v[i], v[j]);
            }
        }
        std::swap(v[i + 1], v[hi]);
        int p = i + 1;

        stack.push_back({lo, p - 1});
        stack.push_back({p + 1, hi});
    }
}

void NetworkRepair::loadNetwork(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return;

    int n = 0, m = 0;
    in >> n >> m;
    dsu.init(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        in >> u >> v;
        dsu.unite(u, v);
    }
}

void NetworkRepair::runOperations(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string op;
        iss >> op;
        if (op.empty()) continue;

        if (op == "ADD_REPAIR") {
            std::string id; int u, v, urg, cost;
            iss >> id >> u >> v >> urg >> cost;
            opAddRepair(id, u, v, urg, cost);
        } else if (op == "UPDATE_REPAIR") {
            std::string id; int urg, cost;
            iss >> id >> urg >> cost;
            opUpdateRepair(id, urg, cost);
        } else if (op == "PRINT_PENDING") {
            opPrintPending();
        } else if (op == "REPAIR_NEXT") {
            opRepairNext();
        } else if (op == "CONNECTED") {
            int u, v; iss >> u >> v;
            opConnected(u, v);
        } else if (op == "COMPONENT_SIZE") {
            int u; iss >> u;
            opComponentSize(u);
        } else if (op == "COMPONENT_COUNT") {
            opComponentCount();
        } else if (op == "TOTAL_COST") {
            opTotalCost();
        }
    }
}

void NetworkRepair::opAddRepair(const std::string& id, int u, int v, int urgency, int cost) {
    if (seenIds.find(id) != seenIds.end()) return;
    seenIds[id] = true;
    heap.insert(RepairRequest(id, u, v, urgency, cost));
}

void NetworkRepair::opUpdateRepair(const std::string& id, int newUrgency, int newCost) {
    if (!heap.contains(id)) return;
    heap.updatePriority(id, newUrgency, newCost);
}

void NetworkRepair::opPrintPending() {
    std::cout << "Pending repairs:" << "\n";
    if (heap.empty()) {
        std::cout << "<Empty>" << "\n";
        return;
    }
    std::vector<RepairRequest> snap = heap.snapshot();
    sortRequests(snap);
    for (const auto& r : snap) {
        std::cout << r.id << " " << r.u << " " << r.v
                  << " urgency=" << r.urgency
                  << " cost=" << r.cost << "\n";
    }
}

void NetworkRepair::opRepairNext() {
    if (heap.empty()) {
        std::cout << "No repairs pending." << "\n";
        return;
    }
    RepairRequest top = heap.extractTop();
    if (dsu.connected(top.u, top.v)) {
        std::cout << "Skipped: " << top.id << "\n";
        std::cout << "Stations: " << top.u << "-" << top.v << "\n";
        std::cout << "Components: " << dsu.getComponentCount() << "\n";
    } else {
        dsu.unite(top.u, top.v);
        totalCost += top.cost;
        std::cout << "Repaired: " << top.id << "\n";
        std::cout << "Stations: " << top.u << "-" << top.v << "\n";
        std::cout << "Components: " << dsu.getComponentCount() << "\n";
    }
}

void NetworkRepair::opConnected(int u, int v) {
    bool c = dsu.connected(u, v);
    std::cout << "Connected " << u << " " << v << ": " << (c ? "YES" : "NO") << "\n";
}

void NetworkRepair::opComponentSize(int u) {
    std::cout << "Component size of " << u << ": " << dsu.componentSize(u) << "\n";
}

void NetworkRepair::opComponentCount() {
    std::cout << "Component count: " << dsu.getComponentCount() << "\n";
}

void NetworkRepair::opTotalCost() {
    std::cout << "Total cost: " << totalCost << "\n";
}