#include <boost/coroutine2/all.hpp>
#include <iostream>
#include <vector>
#include <functional>

class Graph {
public:
    explicit Graph(int n) : adj_(n) {}

    void add_edge(int u, int v) {
        adj_[u].push_back(v);
    }

    int size() const {
        return static_cast<int>(adj_.size());
    }

    const std::vector<int>& neighbors(int v) const {
        return adj_[v];
    }

private:
    std::vector<std::vector<int>> adj_;
};

using coro_t = boost::coroutines2::coroutine<int>;

// DFS как кооперативная задача:
// после посещения вершины передаем управление наружу через sink(v)
void dfs_coroutine(const Graph& g, int start, coro_t::push_type& sink) {
    std::vector<bool> visited(g.size(), false);

    std::function<void(int)> dfs = [&](int v) {
        visited[v] = true;

        // "Шаг" кооперативной многозадачности:
        // отдаем наружу текущую вершину и добровольно уступаем управление
        sink(v);

        for (int to : g.neighbors(v)) {
            if (!visited[to]) {
                dfs(to);
            }
        }
    };

    if (start >= 0 && start < g.size()) {
        dfs(start);
    }
}

int main() {
    // Тестовый граф, заданный в коде
    // 0 -> 1, 2
    // 1 -> 3, 4
    // 2 -> 5
    // 4 -> 6
    Graph g(7);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(1, 4);
    g.add_edge(2, 5);
    g.add_edge(4, 6);

    int start_vertex = 0;

    // pull_type получает значения из coroutine
    coro_t::pull_type source(
        [&](coro_t::push_type& sink) {
            dfs_coroutine(g, start_vertex, sink);
        }
    );

    std::cout << "Пошаговый обход графа в глубину:\n\n";

    int step = 1;
    for (int v : source) {
        std::cout << "[Диспетчер] Шаг " << step
                  << ": получена вершина " << v << "\n";

        // Имитация другой работы между возобновлениями DFS
        std::cout << "[Диспетчер] Выполняется другая работа...\n";
        std::cout << "[Диспетчер] Возобновляем DFS\n\n";

        ++step;
    }

    std::cout << "Обход завершен.\n";
    return 0;
} 