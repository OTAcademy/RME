//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "lua_api_algo.h"

#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <queue>
#include <set>

namespace LuaAPI {

	// Helper: convert Lua table to 2D grid
	static std::vector<std::vector<int>> tableToGrid(const sol::table& tbl, int width, int height) {
		std::vector<std::vector<int>> grid(height, std::vector<int>(width, 0));

		for (int y = 1; y <= height; ++y) {
			if (tbl[y].valid() && tbl[y].get_type() == sol::type::table) {
				sol::table row = tbl[y];
				for (int x = 1; x <= width; ++x) {
					if (row[x].valid()) {
						grid[y - 1][x - 1] = row[x].get<int>();
					}
				}
			}
		}
		return grid;
	}

	// Helper: convert 2D grid to Lua table
	static sol::table gridToTable(const std::vector<std::vector<int>>& grid, sol::state_view& lua) {
		sol::table result = lua.create_table();
		for (size_t y = 0; y < grid.size(); ++y) {
			sol::table row = lua.create_table();
			for (size_t x = 0; x < grid[y].size(); ++x) {
				row[x + 1] = grid[y][x];
			}
			result[y + 1] = row;
		}
		return result;
	}

	// Helper: convert Lua table to 2D float grid
	static std::vector<std::vector<float>> tableToFloatGrid(const sol::table& tbl, int width, int height) {
		std::vector<std::vector<float>> grid(height, std::vector<float>(width, 0.0f));

		for (int y = 1; y <= height; ++y) {
			if (tbl[y].valid() && tbl[y].get_type() == sol::type::table) {
				sol::table row = tbl[y];
				for (int x = 1; x <= width; ++x) {
					if (row[x].valid()) {
						grid[y - 1][x - 1] = row[x].get<float>();
					}
				}
			}
		}
		return grid;
	}

	// Helper: convert 2D float grid to Lua table
	static sol::table floatGridToTable(const std::vector<std::vector<float>>& grid, sol::state_view& lua) {
		sol::table result = lua.create_table();
		for (size_t y = 0; y < grid.size(); ++y) {
			sol::table row = lua.create_table();
			for (size_t x = 0; x < grid[y].size(); ++x) {
				row[x + 1] = grid[y][x];
			}
			result[y + 1] = row;
		}
		return result;
	}

	void registerAlgo(sol::state& lua) {
		sol::table algoTable = lua.create_table();

		// ========================================
		// CELLULAR AUTOMATA
		// ========================================

		// algo.cellularAutomata(grid, options) -> grid
		// Run cellular automata simulation (useful for caves, organic shapes)
		// grid: 2D table where 1 = wall, 0 = floor
		// options: { iterations, birthLimit, deathLimit, width, height }
		algoTable.set_function("cellularAutomata", [](sol::table inputGrid, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			int iterations = 4;
			int birthLimit = 4; // Become wall if neighbors >= birthLimit
			int deathLimit = 3; // Stay wall if neighbors >= deathLimit
			int width = 0;
			int height = 0;

			// Get dimensions from grid
			if (inputGrid[1].valid()) {
				height = static_cast<int>(inputGrid.size());
				if (inputGrid[1].get_type() == sol::type::table) {
					sol::table firstRow = inputGrid[1];
					width = static_cast<int>(firstRow.size());
				}
			}

			if (options) {
				sol::table opts = *options;
				iterations = opts.get_or(std::string("iterations"), 4);
				birthLimit = opts.get_or(std::string("birthLimit"), 4);
				deathLimit = opts.get_or(std::string("deathLimit"), 3);
				width = opts.get_or(std::string("width"), width);
				height = opts.get_or(std::string("height"), height);
			}

			if (width <= 0 || height <= 0) {
				return inputGrid; // Return unchanged if invalid dimensions
			}

			auto grid = tableToGrid(inputGrid, width, height);

			// Run iterations
			for (int iter = 0; iter < iterations; ++iter) {
				std::vector<std::vector<int>> newGrid = grid;

				for (int y = 0; y < height; ++y) {
					for (int x = 0; x < width; ++x) {
						// Count neighbors (8-directional)
						int neighbors = 0;
						for (int dy = -1; dy <= 1; ++dy) {
							for (int dx = -1; dx <= 1; ++dx) {
								if (dx == 0 && dy == 0) {
									continue;
								}
								int nx = x + dx;
								int ny = y + dy;
								// Treat edges as walls
								if (nx < 0 || nx >= width || ny < 0 || ny >= height) {
									neighbors++;
								} else if (grid[ny][nx] == 1) {
									neighbors++;
								}
							}
						}

						// Apply rules
						if (grid[y][x] == 1) {
							// Wall survives if enough neighbors
							newGrid[y][x] = (neighbors >= deathLimit) ? 1 : 0;
						} else {
							// Floor becomes wall if too many neighbors
							newGrid[y][x] = (neighbors >= birthLimit) ? 1 : 0;
						}
					}
				}

				grid = newGrid;
			}

			return gridToTable(grid, lua);
		});

		// algo.generateCave(width, height, options) -> grid
		// Generate a cave map using cellular automata
		// options: { fillProbability, iterations, birthLimit, deathLimit, seed }
		algoTable.set_function("generateCave", [](int width, int height, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			float fillProbability = 0.45f;
			int iterations = 4;
			int birthLimit = 4;
			int deathLimit = 3;
			int seed = static_cast<int>(time(nullptr));

			if (options) {
				sol::table opts = *options;
				fillProbability = opts.get_or(std::string("fillProbability"), 0.45f);
				iterations = opts.get_or(std::string("iterations"), 4);
				birthLimit = opts.get_or(std::string("birthLimit"), 4);
				deathLimit = opts.get_or(std::string("deathLimit"), 3);
				seed = opts.get_or(std::string("seed"), seed);
			}

			std::mt19937 rng(seed);
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);

			// Initialize random grid
			std::vector<std::vector<int>> grid(height, std::vector<int>(width, 0));
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					// Edges are always walls
					if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
						grid[y][x] = 1;
					} else {
						grid[y][x] = (dist(rng) < fillProbability) ? 1 : 0;
					}
				}
			}

			// Run cellular automata
			for (int iter = 0; iter < iterations; ++iter) {
				std::vector<std::vector<int>> newGrid = grid;

				for (int y = 1; y < height - 1; ++y) {
					for (int x = 1; x < width - 1; ++x) {
						int neighbors = 0;
						for (int dy = -1; dy <= 1; ++dy) {
							for (int dx = -1; dx <= 1; ++dx) {
								if (dx == 0 && dy == 0) {
									continue;
								}
								if (grid[y + dy][x + dx] == 1) {
									neighbors++;
								}
							}
						}

						if (grid[y][x] == 1) {
							newGrid[y][x] = (neighbors >= deathLimit) ? 1 : 0;
						} else {
							newGrid[y][x] = (neighbors >= birthLimit) ? 1 : 0;
						}
					}
				}

				grid = newGrid;
			}

			return gridToTable(grid, lua);
		});

		// ========================================
		// EROSION ALGORITHMS
		// ========================================

		// algo.erode(heightmap, options) -> heightmap
		// Hydraulic erosion simulation for terrain
		// heightmap: 2D table of float values [0, 1]
		// options: { iterations, erosionRadius, inertia, sedimentCapacity, minSlope, erosionSpeed, depositSpeed, evaporateSpeed, gravity }
		algoTable.set_function("erode", [](sol::table inputHeightmap, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			// Get dimensions
			int height = 0;
			int width = 0;
			if (inputHeightmap[1].valid()) {
				height = static_cast<int>(inputHeightmap.size());
				if (inputHeightmap[1].get_type() == sol::type::table) {
					sol::table firstRow = inputHeightmap[1];
					width = static_cast<int>(firstRow.size());
				}
			}

			if (width <= 2 || height <= 2) {
				return inputHeightmap;
			}

			// Erosion parameters
			int iterations = 50000;
			int erosionRadius = 3;
			float inertia = 0.05f;
			float sedimentCapacity = 4.0f;
			float minSlope = 0.01f;
			float erosionSpeed = 0.3f;
			float depositSpeed = 0.3f;
			float evaporateSpeed = 0.01f;
			float gravity = 4.0f;
			int seed = static_cast<int>(time(nullptr));
			int maxDropletLifetime = 30;

			if (options) {
				sol::table opts = *options;
				iterations = opts.get_or(std::string("iterations"), 50000);
				erosionRadius = opts.get_or(std::string("erosionRadius"), 3);
				inertia = opts.get_or(std::string("inertia"), 0.05f);
				sedimentCapacity = opts.get_or(std::string("sedimentCapacity"), 4.0f);
				minSlope = opts.get_or(std::string("minSlope"), 0.01f);
				erosionSpeed = opts.get_or(std::string("erosionSpeed"), 0.3f);
				depositSpeed = opts.get_or(std::string("depositSpeed"), 0.3f);
				evaporateSpeed = opts.get_or(std::string("evaporateSpeed"), 0.01f);
				gravity = opts.get_or(std::string("gravity"), 4.0f);
				seed = opts.get_or(std::string("seed"), seed);
				maxDropletLifetime = opts.get_or(std::string("maxDropletLifetime"), 30);
			}

			auto heightmap = tableToFloatGrid(inputHeightmap, width, height);

			std::mt19937 rng(seed);
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);

			// Precompute erosion brush weights
			std::vector<std::vector<std::pair<int, int>>> brushIndices(erosionRadius * 2 + 1);
			std::vector<std::vector<float>> brushWeights(erosionRadius * 2 + 1);

			for (int radius = 0; radius <= erosionRadius; ++radius) {
				for (int y = -radius; y <= radius; ++y) {
					for (int x = -radius; x <= radius; ++x) {
						float sqrDst = (float)(x * x + y * y);
						if (sqrDst <= radius * radius) {
							brushIndices[radius].push_back({ x, y });
							float weight = 1.0f - std::sqrt(sqrDst) / (float)radius;
							brushWeights[radius].push_back(weight);
						}
					}
				}
			}

			// Helper to get interpolated height
			auto getHeight = [&](float x, float y) -> float {
				int xi = (int)x;
				int yi = (int)y;
				float fx = x - xi;
				float fy = y - yi;

				xi = std::max(0, std::min(xi, width - 2));
				yi = std::max(0, std::min(yi, height - 2));

				float h00 = heightmap[yi][xi];
				float h10 = heightmap[yi][xi + 1];
				float h01 = heightmap[yi + 1][xi];
				float h11 = heightmap[yi + 1][xi + 1];

				return h00 * (1 - fx) * (1 - fy) + h10 * fx * (1 - fy) + h01 * (1 - fx) * fy + h11 * fx * fy;
			};

			// Helper to get gradient
			auto getGradient = [&](float x, float y) -> std::pair<float, float> {
				int xi = (int)x;
				int yi = (int)y;

				xi = std::max(1, std::min(xi, width - 2));
				yi = std::max(1, std::min(yi, height - 2));

				float gx = (heightmap[yi][xi + 1] - heightmap[yi][xi - 1]) * 0.5f;
				float gy = (heightmap[yi + 1][xi] - heightmap[yi - 1][xi]) * 0.5f;

				return { gx, gy };
			};

			// Simulate droplets
			for (int i = 0; i < iterations; ++i) {
				// Random starting position
				float posX = dist(rng) * (width - 2) + 1;
				float posY = dist(rng) * (height - 2) + 1;
				float dirX = 0, dirY = 0;
				float speed = 1;
				float water = 1;
				float sediment = 0;

				for (int lifetime = 0; lifetime < maxDropletLifetime; ++lifetime) {
					int nodeX = (int)posX;
					int nodeY = (int)posY;

					// Get gradient
					auto [gx, gy] = getGradient(posX, posY);

					// Update direction with inertia
					dirX = dirX * inertia - gx * (1 - inertia);
					dirY = dirY * inertia - gy * (1 - inertia);

					// Normalize direction
					float len = std::sqrt(dirX * dirX + dirY * dirY);
					if (len > 0) {
						dirX /= len;
						dirY /= len;
					}

					// New position
					float newPosX = posX + dirX;
					float newPosY = posY + dirY;

					// Stop if out of bounds
					if (newPosX < 1 || newPosX >= width - 1 || newPosY < 1 || newPosY >= height - 1) {
						break;
					}

					// Height difference
					float newHeight = getHeight(newPosX, newPosY);
					float oldHeight = getHeight(posX, posY);
					float deltaHeight = newHeight - oldHeight;

					// Calculate sediment capacity
					float capacity = std::max(-deltaHeight, minSlope) * speed * water * sedimentCapacity;

					// Deposit or erode
					if (sediment > capacity || deltaHeight > 0) {
						// Deposit sediment
						float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, sediment) : (sediment - capacity) * depositSpeed;
						sediment -= amountToDeposit;

						// Deposit at current position
						int hx = std::min(std::max(nodeX, 0), width - 1);
						int hy = std::min(std::max(nodeY, 0), height - 1);
						heightmap[hy][hx] += amountToDeposit;
					} else {
						// Erode terrain
						float amountToErode = std::min((capacity - sediment) * erosionSpeed, -deltaHeight);

						// Erode in radius
						for (size_t j = 0; j < brushIndices[erosionRadius].size(); ++j) {
							int ex = nodeX + brushIndices[erosionRadius][j].first;
							int ey = nodeY + brushIndices[erosionRadius][j].second;

							if (ex >= 0 && ex < width && ey >= 0 && ey < height) {
								float weightedErode = amountToErode * brushWeights[erosionRadius][j];
								heightmap[ey][ex] -= weightedErode;
								sediment += weightedErode;
							}
						}
					}

					// Update position and speed
					posX = newPosX;
					posY = newPosY;
					speed = std::sqrt(speed * speed + deltaHeight * gravity);
					water *= (1 - evaporateSpeed);
				}
			}

			return floatGridToTable(heightmap, lua);
		});

		// algo.thermalErode(heightmap, options) -> heightmap
		// Thermal erosion (talus/slope erosion)
		algoTable.set_function("thermalErode", [](sol::table inputHeightmap, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			int height = 0;
			int width = 0;
			if (inputHeightmap[1].valid()) {
				height = static_cast<int>(inputHeightmap.size());
				if (inputHeightmap[1].get_type() == sol::type::table) {
					sol::table firstRow = inputHeightmap[1];
					width = static_cast<int>(firstRow.size());
				}
			}

			if (width <= 2 || height <= 2) {
				return inputHeightmap;
			}

			int iterations = 50;
			float talusAngle = 0.5f; // Maximum slope before erosion
			float erosionAmount = 0.5f;

			if (options) {
				sol::table opts = *options;
				iterations = opts.get_or(std::string("iterations"), 50);
				talusAngle = opts.get_or(std::string("talusAngle"), 0.5f);
				erosionAmount = opts.get_or(std::string("erosionAmount"), 0.5f);
			}

			auto heightmap = tableToFloatGrid(inputHeightmap, width, height);

			// 4-directional neighbors
			const int dx[] = { 0, 1, 0, -1 };
			const int dy[] = { -1, 0, 1, 0 };

			for (int iter = 0; iter < iterations; ++iter) {
				auto newHeightmap = heightmap;

				for (int y = 1; y < height - 1; ++y) {
					for (int x = 1; x < width - 1; ++x) {
						float currentHeight = heightmap[y][x];

						// Find maximum difference
						float maxDiff = 0;
						int maxIdx = -1;

						for (int i = 0; i < 4; ++i) {
							int nx = x + dx[i];
							int ny = y + dy[i];
							float diff = currentHeight - heightmap[ny][nx];
							if (diff > maxDiff) {
								maxDiff = diff;
								maxIdx = i;
							}
						}

						// Erode if slope exceeds talus angle
						if (maxDiff > talusAngle && maxIdx >= 0) {
							float transfer = (maxDiff - talusAngle) * erosionAmount * 0.5f;
							newHeightmap[y][x] -= transfer;
							newHeightmap[y + dy[maxIdx]][x + dx[maxIdx]] += transfer;
						}
					}
				}

				heightmap = newHeightmap;
			}

			return floatGridToTable(heightmap, lua);
		});

		// ========================================
		// SMOOTHING ALGORITHMS
		// ========================================

		// algo.smooth(grid, options) -> grid
		// Gaussian-like smoothing for grids
		algoTable.set_function("smooth", [](sol::table inputGrid, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			int height = 0;
			int width = 0;
			if (inputGrid[1].valid()) {
				height = static_cast<int>(inputGrid.size());
				if (inputGrid[1].get_type() == sol::type::table) {
					sol::table firstRow = inputGrid[1];
					width = static_cast<int>(firstRow.size());
				}
			}

			if (width <= 2 || height <= 2) {
				return inputGrid;
			}

			int iterations = 1;
			int kernelSize = 3;

			if (options) {
				sol::table opts = *options;
				iterations = opts.get_or(std::string("iterations"), 1);
				kernelSize = opts.get_or(std::string("kernelSize"), 3);
			}

			auto grid = tableToFloatGrid(inputGrid, width, height);

			int radius = kernelSize / 2;

			for (int iter = 0; iter < iterations; ++iter) {
				auto newGrid = grid;

				for (int y = radius; y < height - radius; ++y) {
					for (int x = radius; x < width - radius; ++x) {
						float sum = 0;
						int count = 0;

						for (int dy = -radius; dy <= radius; ++dy) {
							for (int dx = -radius; dx <= radius; ++dx) {
								sum += grid[y + dy][x + dx];
								count++;
							}
						}

						newGrid[y][x] = sum / count;
					}
				}

				grid = newGrid;
			}

			return floatGridToTable(grid, lua);
		});

		// ========================================
		// VORONOI DIAGRAM
		// ========================================

		// algo.voronoi(width, height, points) -> grid of region indices
		// Generate Voronoi diagram from seed points
		algoTable.set_function("voronoi", [](int width, int height, sol::table points, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			// Parse points
			std::vector<std::pair<int, int>> seedPoints;
			for (auto& kv : points) {
				if (kv.second.get_type() == sol::type::table) {
					sol::table pt = kv.second;
					int x = pt.get_or(std::string("x"), pt.get_or(1, 0));
					int y = pt.get_or(std::string("y"), pt.get_or(2, 0));
					seedPoints.push_back({ x, y });
				}
			}

			if (seedPoints.empty()) {
				return lua.create_table();
			}

			std::vector<std::vector<int>> grid(height, std::vector<int>(width, 0));

			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					float minDist = std::numeric_limits<float>::max();
					int closestRegion = 0;

					for (size_t i = 0; i < seedPoints.size(); ++i) {
						float dx = (float)(x - seedPoints[i].first);
						float dy = (float)(y - seedPoints[i].second);
						float dist = dx * dx + dy * dy; // Squared distance for speed

						if (dist < minDist) {
							minDist = dist;
							closestRegion = static_cast<int>(i + 1); // 1-indexed for Lua
						}
					}

					grid[y][x] = closestRegion;
				}
			}

			return gridToTable(grid, lua);
		});

		// algo.generateRandomPoints(width, height, count, seed) -> table of points
		// Generate random points for Voronoi, etc.
		algoTable.set_function("generateRandomPoints", [](int width, int height, int count, sol::optional<int> seed, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			int sd = seed.value_or(static_cast<int>(time(nullptr)));
			std::mt19937 rng(sd);
			std::uniform_int_distribution<int> distX(0, width - 1);
			std::uniform_int_distribution<int> distY(0, height - 1);

			sol::table result = lua.create_table();

			for (int i = 0; i < count; ++i) {
				sol::table point = lua.create_table();
				point["x"] = distX(rng);
				point["y"] = distY(rng);
				result[i + 1] = point;
			}

			return result;
		});

		// ========================================
		// MAZE GENERATION
		// ========================================

		// algo.generateMaze(width, height, options) -> grid
		// Generate a maze using recursive backtracking
		algoTable.set_function("generateMaze", [](int width, int height, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			int seed = static_cast<int>(time(nullptr));

			if (options) {
				sol::table opts = *options;
				seed = opts.get_or(std::string("seed"), seed);
			}

			std::mt19937 rng(seed);

			// Make dimensions odd for proper maze
			if (width % 2 == 0) {
				width++;
			}
			if (height % 2 == 0) {
				height++;
			}

			// Initialize grid with walls
			std::vector<std::vector<int>> grid(height, std::vector<int>(width, 1));

			// Recursive backtracking
			std::function<void(int, int)> carve = [&](int x, int y) {
				grid[y][x] = 0;

				// Shuffle directions
				std::vector<int> dirs = { 0, 1, 2, 3 }; // N, E, S, W
				std::shuffle(dirs.begin(), dirs.end(), rng);

				const int dx[] = { 0, 1, 0, -1 };
				const int dy[] = { -1, 0, 1, 0 };

				for (int dir : dirs) {
					int nx = x + dx[dir] * 2;
					int ny = y + dy[dir] * 2;

					if (nx > 0 && nx < width - 1 && ny > 0 && ny < height - 1 && grid[ny][nx] == 1) {
						// Carve through wall
						grid[y + dy[dir]][x + dx[dir]] = 0;
						carve(nx, ny);
					}
				}
			};

			// Start from (1, 1)
			carve(1, 1);

			return gridToTable(grid, lua);
		});

		// ========================================
		// BSP (Binary Space Partitioning) - Dungeon Generation
		// ========================================

		// algo.generateDungeon(width, height, options) -> { grid, rooms }
		// Generate a dungeon using BSP
		algoTable.set_function("generateDungeon", [](int width, int height, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
			sol::state_view lua(s);

			int minRoomSize = 5;
			int maxRoomSize = 15;
			int seed = static_cast<int>(time(nullptr));
			int maxDepth = 4;

			if (options) {
				sol::table opts = *options;
				minRoomSize = opts.get_or(std::string("minRoomSize"), 5);
				maxRoomSize = opts.get_or(std::string("maxRoomSize"), 15);
				seed = opts.get_or(std::string("seed"), seed);
				maxDepth = opts.get_or(std::string("maxDepth"), 4);
			}

			std::mt19937 rng(seed);

			// Initialize grid with walls
			std::vector<std::vector<int>> grid(height, std::vector<int>(width, 1));

			// Store rooms
			std::vector<std::tuple<int, int, int, int>> rooms; // x, y, w, h

			// BSP Node
			struct BSPNode {
				int x, y, w, h;
				BSPNode* left = nullptr;
				BSPNode* right = nullptr;
				int roomX, roomY, roomW, roomH;
				bool hasRoom = false;
			};

			std::function<BSPNode*(int, int, int, int, int)> split;
			split = [&](int x, int y, int w, int h, int depth) -> BSPNode* {
				BSPNode* node = new BSPNode { x, y, w, h };

				if (depth >= maxDepth || w < minRoomSize * 2 || h < minRoomSize * 2) {
					// Check if space is sufficient for a room
					if (w < minRoomSize + 2 || h < minRoomSize + 2) {
						return node;
					}

					// Create room
					std::uniform_int_distribution<int> roomW(minRoomSize, std::min(maxRoomSize, w - 2));
					std::uniform_int_distribution<int> roomH(minRoomSize, std::min(maxRoomSize, h - 2));

					int rw = roomW(rng);
					int rh = roomH(rng);

					std::uniform_int_distribution<int> roomX(x + 1, x + w - rw - 1);
					std::uniform_int_distribution<int> roomY(y + 1, y + h - rh - 1);

					node->roomX = roomX(rng);
					node->roomY = roomY(rng);
					node->roomW = rw;
					node->roomH = rh;
					node->hasRoom = true;

					rooms.push_back({ node->roomX, node->roomY, rw, rh });

					return node;
				}

				// Split
				std::uniform_real_distribution<float> splitDist(0.3f, 0.7f);
				float splitRatio = splitDist(rng);

				bool splitHorizontal = (w < h) || (w == h && rng() % 2 == 0);

				if (splitHorizontal) {
					int splitY = y + static_cast<int>(h * splitRatio);
					node->left = split(x, y, w, splitY - y, depth + 1);
					node->right = split(x, splitY, w, y + h - splitY, depth + 1);
				} else {
					int splitX = x + static_cast<int>(w * splitRatio);
					node->left = split(x, y, splitX - x, h, depth + 1);
					node->right = split(splitX, y, x + w - splitX, h, depth + 1);
				}

				return node;
			};

			BSPNode* root = split(0, 0, width, height, 0);

			// Carve rooms
			for (const auto& room : rooms) {
				int rx = std::get<0>(room);
				int ry = std::get<1>(room);
				int rw = std::get<2>(room);
				int rh = std::get<3>(room);

				for (int py = ry; py < ry + rh && py < height; ++py) {
					for (int px = rx; px < rx + rw && px < width; ++px) {
						grid[py][px] = 0;
					}
				}
			}

			// Connect rooms with corridors
			for (size_t i = 1; i < rooms.size(); ++i) {
				int x1 = std::get<0>(rooms[i - 1]) + std::get<2>(rooms[i - 1]) / 2;
				int y1 = std::get<1>(rooms[i - 1]) + std::get<3>(rooms[i - 1]) / 2;
				int x2 = std::get<0>(rooms[i]) + std::get<2>(rooms[i]) / 2;
				int y2 = std::get<1>(rooms[i]) + std::get<3>(rooms[i]) / 2;

				// L-shaped corridor
				if (rng() % 2 == 0) {
					// Horizontal first
					for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
						if (y1 >= 0 && y1 < height && x >= 0 && x < width) {
							grid[y1][x] = 0;
						}
					}
					for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
						if (y >= 0 && y < height && x2 >= 0 && x2 < width) {
							grid[y][x2] = 0;
						}
					}
				} else {
					// Vertical first
					for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
						if (y >= 0 && y < height && x1 >= 0 && x1 < width) {
							grid[y][x1] = 0;
						}
					}
					for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
						if (y2 >= 0 && y2 < height && x >= 0 && x < width) {
							grid[y2][x] = 0;
						}
					}
				}
			}

			// Cleanup BSP
			std::function<void(BSPNode*)> deleteBSP = [&](BSPNode* node) {
				if (node) {
					deleteBSP(node->left);
					deleteBSP(node->right);
					delete node;
				}
			};
			deleteBSP(root);

			// Return result
			sol::table result = lua.create_table();
			result["grid"] = gridToTable(grid, lua);

			sol::table roomsTable = lua.create_table();
			for (size_t i = 0; i < rooms.size(); ++i) {
				sol::table room = lua.create_table();
				room["x"] = std::get<0>(rooms[i]);
				room["y"] = std::get<1>(rooms[i]);
				room["width"] = std::get<2>(rooms[i]);
				room["height"] = std::get<3>(rooms[i]);
				roomsTable[i + 1] = room;
			}
			result["rooms"] = roomsTable;

			return result;
		});

		lua["algo"] = algoTable;
	}

} // namespace LuaAPI
