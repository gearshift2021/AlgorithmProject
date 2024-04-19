#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <climits>
#include <sstream>


using namespace std;

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
    cout<< "Height of b is " << b.height << " Height of a is " << a.height << endl;
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
    ss >> m >> comma >> n;
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
// Utility function to get all neighbors of a station that are within grid bounds
vector<Station> getNeighbors(const Station &s, const vector<vector<Station>> &grid)
{
    vector<Station> neighbors;
    vector<pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    for (auto &dir : directions)
    {
        int newX = s.x + dir.first;
        int newY = s.y + dir.second;
        if (newX >= 0 && newY >= 0 && newX < grid.size() && newY < grid[0].size())
        {
            cout <<"IM at (" << s.x << ", " << s.y << ") Neighbor at (" << newX << ", " << newY << ")" << endl;
            neighbors.push_back(grid[newX][newY]);
        }
    }
    return neighbors;
}



// Bellman-Ford-Moore algorithm to compute shortest paths from a source to all other stations
void bellmanFordMoore(const vector<vector<Station>> &grid, Station &source, vector<vector<int>> &minDistances)
{
    int m = grid.size(), n = grid[0].size();
    minDistances.assign(m, vector<int>(n, INT_MAX));
    minDistances[source.x][source.y] = 0;
    bool updated;

    // Run the algorithm m*n times to account for all stations
    for (int i = 0; i < m * n; ++i)
    {
        updated = false;
        for (int x = 0; x < m; ++x)
        {
            for (int y = 0; y < n; ++y)
            {
                if (minDistances[x][y] == INT_MAX)
                    continue;
                for (Station &neighbor : getNeighbors(grid[x][y], grid))
                {
                    int altDistance = minDistances[x][y] + timeToMove(grid[x][y], neighbor);
                    if (altDistance < minDistances[neighbor.x][neighbor.y])
                    {
                        minDistances[neighbor.x][neighbor.y] = altDistance;
                        updated = true;
                    }
                }
            }
        }
        if (!updated)
            break; // No updates mean we can exit early
    }
}

// Backtracking function to try all permutations of bath stations and compute the minimum path cost
int findMinimumCost(Station &source, vector<Station> &baths, const vector<vector<int>> &minDistances)
{
    sort(baths.begin(), baths.end(), [](const Station &a, const Station &b)
         { return make_pair(a.x, a.y) < make_pair(b.x, b.y); });

    int minCost = INT_MAX;
    do
    {
        int cost = 0;
        Station current = source;
        for (Station &bath : baths)
        {
            cost += minDistances[current.x][current.y] + timeToMove(current, bath);
            current = bath;
            if (cost >= minCost)
                break; // Prune the search if cost exceeds current minimum
        }
        minCost = min(minCost, cost);
    } while (next_permutation(baths.begin(), baths.end()));

    return minCost;
}

// Function to print the grid
void printGrid(const vector<vector<Station>>& grid) {
    for (const auto& row : grid) {
        for (const auto& station : row) {
            cout << station.height << " ";
        }
        cout << endl;
    }
}


int main()
{
    vector<vector<Station>> grid;
    Station source;
    vector<Station> baths;

    loadGrid("grid.txt", grid, source, baths);

        // Print the grid
    cout << "Initial Grid State:" << endl;
    printGrid(grid);

    // Compute the minimum distances from every station to every other station
    vector<vector<int>> minDistances;
    bellmanFordMoore(grid, source, minDistances);

    // Find the minimum cost using backtracking
    int minCost = findMinimumCost(source, baths, minDistances);

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
