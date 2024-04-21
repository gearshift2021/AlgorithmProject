#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <climits>
#include <sstream>
#include <map>

using namespace std;

// Memoization map
map<pair<int, unsigned int>, int> memo;

// Data structure to represent a station
struct Station {
    int height;
    int x;
    int y;

    // Define the less-than operator for Station
    bool operator<(const Station& other) const {
        if (x == other.x) {
            return y < other.y;
        }
        return x < other.x;
    }
};

// Function to calculate time taken to move from one station to another
int timeToMove(const Station &a, const Station &b)
{
    int minCost =max(-1, 1 + (b.height - a.height));
    cout << "Time to move from (" << a.x << ", " << a.y << ") to (" << b.x << ", " << b.y << ") is " << minCost << endl;
    return minCost;
}

// Function to load the grid and bath stations from a file
void loadGrid(const string& filename, vector<vector<Station>>& grid, Station& source, vector<Station>& baths) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    int m, n;
    char comma; // to read the commas
    string line;
    getline(file, line);
    istringstream ss(line);
    ss >> n>> comma >> m;
    cout << "Grid size: " << m << "x" << n << endl;
    grid.resize(m, vector<Station>(n));

    // Read the heights and coordinates of the stations
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            getline(file, line);
            istringstream ss(line);
            ss >> grid[j][i].height >> comma >> grid[j][i].x >> comma >> grid[j][i].y;
            cout << "Station at (" << grid[i][j].x << ", " << grid[i][j].y << ") has height " << grid[i][j].height << endl;
        }
    }

    // Read the source station coordinates
    getline(file, line);
    istringstream ssSource(line);
    ssSource >> source.x >> comma >> source.y;
    source.height = grid[source.x][source.y].height;
    cout << "Source station at (" << source.x << ", " << source.y << ") has height " << source.height << endl;

    // Read the bath station coordinates
    Station bath;
    while (getline(file, line)) {
        istringstream ssBath(line);
        ssBath >> bath.x >> comma >> bath.y;
        if(ssBath) { // ensure we read a valid line
            bath.height = grid[bath.x][bath.y].height;
            cout << "Bath station at (" << bath.x << ", " << bath.y << ") has height " << bath.height << endl;
            baths.push_back(bath);
        }
    }

    file.close();
}

// Bellman-Ford algorithm to compute shortest paths from a source to all other stations
void bellmanFord(const Station& source, const vector<vector<Station>>& grid, vector<vector<int>>& distances) {
    int m = grid.size(), n = grid[0].size();
    distances.assign(m, vector<int>(n, INT_MAX));
    distances[source.x][source.y] = 0;
    for (int step = 0; step < m * n - 1; step++) {
        for (int x = 0; x < m; x++) {
            for (int y = 0; y < n; y++) {
                if (distances[x][y] != INT_MAX) {
                    vector<pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                    for (auto& dir : directions) {
                        int newX = x + dir.first;
                        int newY = y + dir.second;
                        if (newX >= 0 && newX < m && newY >= 0 && newY < n) {
                            int newCost = distances[x][y] + timeToMove(grid[x][y], grid[newX][newY]);
                            if (newCost < distances[newX][newY]) {
                                distances[newX][newY] = newCost;
                            }
                        }
                    }
                }
            }
        }
    }
}

int findMinCostRecursively(const Station& current, const vector<Station>& baths, unsigned int bathMask, const vector<vector<Station>>& grid, map<pair<int, unsigned int>, int>& memo) {
    // Memoization check
    auto key = make_pair(current.x * 100 + current.y, bathMask);
    if (memo.find(key) != memo.end()) return memo[key];

    if (bathMask == (1 << baths.size()) - 1) return 0; // All baths visited

    int minCost = INT_MAX;
    vector<vector<int>> distances;

    // Run Bellman-Ford from the current station
    bellmanFord(current, grid, distances);

    for (int i = 0; i < baths.size(); ++i) {
        if (!(bathMask & (1 << i))) {
            int nextBathMask = bathMask | (1 << i);
            int costToNextBath = distances[baths[i].x][baths[i].y];
            int remainingCost = findMinCostRecursively(baths[i], baths, nextBathMask, grid, memo);
            minCost = min(minCost, costToNextBath + remainingCost);
        }
    }

    memo[key] = minCost;
    return minCost;
}


int main()
{
    vector<vector<Station>> grid;
    Station source;
    vector<Station> baths;
      vector<vector<int>> minDistances;

    loadGrid("grid.txt", grid, source, baths);

    // Find the minimum cost
    int minCost = findMinCostRecursively(source, baths, 0, grid, memo);



    // Write the minimum cost to pathLength.txt
    ofstream outFile("pathLength.txt");
    if (!outFile.is_open())
    {
        cerr << "Error opening pathLength.txt for writing." << endl;
        return 1;
    }
    outFile << minCost << endl;
    outFile.close();

    cout << "The minimum cost of any supply path is: " << minCost << endl;
    return 0;
}
