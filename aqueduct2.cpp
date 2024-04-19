#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <cmath>
#include <unordered_map>

const int MAX_HEIGHT = 1000; // An assumption for the max height to define "infinite" time cost

// Parses the input file and fills out the grid and the list of B stations
void parseInputFile(const std::string& filename, std::vector<std::vector<int>>& grid, std::vector<std::pair<int, int>>& B) {
    std::ifstream file(filename);
    std::string line;
    if (file.is_open()) {
        // Read grid dimensions
        int m, n;
        getline(file, line);
        std::stringstream ss(line);
        ss >> m >> n;
        
        // Resize grid based on dimensions
        grid.resize(m, std::vector<int>(n));
        
        // Read the heights into the grid
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                int height;
                file >> height;
                grid[i][j] = height;
            }
        }

        // Read the stations in B
        while (getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            int x, y;
            ss >> x >> y;
            B.emplace_back(x, y);
        }
    }
    file.close();
}

// Calculates the time to move water between two stations
int timeCost(const std::vector<std::vector<int>>& grid, const std::pair<int, int>& from, const std::pair<int, int>& to) {
    return std::max(-1, 1 + (grid[to.first][to.second] - grid[from.first][from.second]));
}

std::vector<int> bellmanFord(const std::vector<std::vector<int>>& grid, const std::pair<int, int>& source) {
    int m = grid.size();
    int n = grid[0].size();
    std::vector<int> distance(m * n, MAX_HEIGHT); // Initialize distances with a high value (like "infinity")

    // Lambda to convert 2D grid coordinates to a single index for simplicity
    auto coordToIndex = [n](const std::pair<int, int>& coord) {
        return coord.first * n + coord.second;
    };

    distance[coordToIndex(source)] = 0; // Distance to source is 0

    for (int i = 0; i < m * n - 1; ++i) {
        for (int u = 0; u < m; ++u) {
            for (int v = 0; v < n; ++v) {
                // Check all four possible moves (up, down, left, right)
                std::vector<std::pair<int, int>> moves = {{u - 1, v}, {u + 1, v}, {u, v - 1}, {u, v + 1}};
                for (const auto& move : moves) {
                    if (move.first >= 0 && move.first < m && move.second >= 0 && move.second < n) {
                        int fromIndex = coordToIndex({u, v});
                        int toIndex = coordToIndex(move);
                        int cost = timeCost(grid, {u, v}, move);
                        if (distance[fromIndex] != MAX_HEIGHT && distance[fromIndex] + cost < distance[toIndex]) {
                            distance[toIndex] = distance[fromIndex] + cost;
                        }
                    }
                }
            }
        }
    }
    return distance;
}



// A utility function to set a bit in the integer bitmask
int setBit(int mask, int bit, bool value) {
    if (value) {
        return mask | (1 << bit);
    } else {
        return mask & ~(1 << bit);
    }
}

// Checks if a bit is set in the integer bitmask
bool isBitSet(int mask, int bit) {
    return (mask & (1 << bit)) != 0;
}

int findOptimalPath(
    const std::vector<std::vector<int>>& grid,
    const std::vector<std::pair<int, int>>& B,
    std::unordered_map<int, int>& M,
    int s,
    int subsetMask
) {
    // Base case: if the subset only contains the source
    if (subsetMask == 1) {
        return 0; // No cost if we are just at the starting node
    }

    // Check if we have already computed the optimal path for this subset
    if (M.find(subsetMask * B.size() + s) != M.end()) {
        return M[subsetMask * B.size() + s];
    }

    int minPathCost = MAX_HEIGHT;
    // Recur for all stations in the subset to find the minimum cost path
    for (int i = 0; i < B.size(); ++i) {
        if (isBitSet(subsetMask, i) && i != s) {
            int nextSubsetMask = setBit(subsetMask, i, false); // Remove the current station from the next subset
            int costToNextStation = bellmanFord(grid, B[s])[B[i].first * grid[0].size() + B[i].second]; // Cost to next station
            int nextStationCost = findOptimalPath(grid, B, M, i, nextSubsetMask);
            if (costToNextStation != MAX_HEIGHT && nextStationCost != MAX_HEIGHT) {
                minPathCost = std::min(minPathCost, costToNextStation + nextStationCost);
            }
        }
    }

    // Save the result in the memoization table before returning
    M[subsetMask * B.size() + s] = minPathCost;
    return minPathCost;
}


int main() {
    std::vector<std::vector<int>> grid;
    std::vector<std::pair<int, int>> B;

    // Read the input from the file
    parseInputFile("grid.txt", grid, B);

    // Use a map to keep track of the computed optimal paths
    std::unordered_map<int, int> memoizationTable;

    // The starting node is always (0,0) - the source
    int source = 0; // Assuming that the source is always the first node
    int allStationsSubset = (1 << B.size()) - 1; // Binary representation of the subset with all stations

    // Compute the optimal path cost
    int optimalPathCost = findOptimalPath(grid, B, memoizationTable, source, allStationsSubset);

    // Output the path cost to a file
    std::ofstream outputFile("pathLength.txt");
    if (outputFile.is_open()) {
        outputFile << "The minimum cost of any supply path is: " << optimalPathCost << std::endl;
        outputFile.close();
    }

    return 0;
}
