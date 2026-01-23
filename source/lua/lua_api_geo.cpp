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
#include "lua_api_geo.h"
#include <random>

#include <vector>
#include <queue>
#include <set>
#include <cmath>
#include <algorithm>
#include <functional>

namespace LuaAPI {

void registerGeo(sol::state& lua) {
	sol::table geoTable = lua.create_table();

	// ========================================
	// BRESENHAM'S LINE ALGORITHM
	// ========================================

	// geo.bresenhamLine(x1, y1, x2, y2) -> table of points
	// Returns all points on a line between two points
	geoTable.set_function("bresenhamLine", [](int x1, int y1, int x2, int y2, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		int dx = std::abs(x2 - x1);
		int dy = std::abs(y2 - y1);
		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;
		int err = dx - dy;

		int index = 1;
		int x = x1, y = y1;

		while (true) {
			sol::table point = lua.create_table();
			point["x"] = x;
			point["y"] = y;
			result[index++] = point;

			if (x == x2 && y == y2) break;

			int e2 = 2 * err;
			if (e2 > -dy) {
				err -= dy;
				x += sx;
			}
			if (e2 < dx) {
				err += dx;
				y += sy;
			}
		}

		return result;
	});

	// geo.bresenhamLine3d(x1, y1, z1, x2, y2, z2) -> table of 3D points
	geoTable.set_function("bresenhamLine3d", [](int x1, int y1, int z1, int x2, int y2, int z2, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		int dx = std::abs(x2 - x1);
		int dy = std::abs(y2 - y1);
		int dz = std::abs(z2 - z1);

		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;
		int sz = (z1 < z2) ? 1 : -1;

		// Driving axis is the one with greatest delta
		int dm = std::max({dx, dy, dz});

		int i = dm;
		int x = x1, y = y1, z = z1;

		int xErr = dm / 2;
		int yErr = dm / 2;
		int zErr = dm / 2;

		int index = 1;

		for (int step = 0; step <= dm; ++step) {
			sol::table point = lua.create_table();
			point["x"] = x;
			point["y"] = y;
			point["z"] = z;
			result[index++] = point;

			xErr -= dx;
			yErr -= dy;
			zErr -= dz;

			if (xErr < 0) {
				xErr += dm;
				x += sx;
			}
			if (yErr < 0) {
				yErr += dm;
				y += sy;
			}
			if (zErr < 0) {
				zErr += dm;
				z += sz;
			}
		}

		return result;
	});

	// ========================================
	// BEZIER CURVES
	// ========================================

	// geo.bezierCurve(points, steps) -> table of points
	// Quadratic/Cubic Bezier curve through control points
	geoTable.set_function("bezierCurve", [](sol::table controlPoints, sol::optional<int> steps, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		int numSteps = steps.value_or(20);

		// Parse control points
		std::vector<std::pair<float, float>> points;
		for (auto& kv : controlPoints) {
			if (kv.second.get_type() == sol::type::table) {
				sol::table pt = kv.second;
				float x = pt.get_or("x", pt.get_or(1, 0.0f));
				float y = pt.get_or("y", pt.get_or(2, 0.0f));
				points.push_back({x, y});
			}
		}

		if (points.size() < 2) {
			return result;
		}

		int index = 1;

		// De Casteljau's algorithm for any number of control points
		auto deCasteljau = [&](float t) -> std::pair<float, float> {
			std::vector<std::pair<float, float>> temp = points;

			while (temp.size() > 1) {
				std::vector<std::pair<float, float>> newTemp;
				for (size_t i = 0; i < temp.size() - 1; ++i) {
					float x = temp[i].first + t * (temp[i + 1].first - temp[i].first);
					float y = temp[i].second + t * (temp[i + 1].second - temp[i].second);
					newTemp.push_back({x, y});
				}
				temp = newTemp;
			}

			return temp[0];
		};

		for (int i = 0; i <= numSteps; ++i) {
			float t = (float)i / (float)numSteps;
			auto [x, y] = deCasteljau(t);

			sol::table point = lua.create_table();
			point["x"] = std::round(x);
			point["y"] = std::round(y);
			result[index++] = point;
		}

		return result;
	});

	// geo.bezierCurve3d(points, steps) -> table of 3D points
	geoTable.set_function("bezierCurve3d", [](sol::table controlPoints, sol::optional<int> steps, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		int numSteps = steps.value_or(20);

		// Parse control points
		std::vector<std::tuple<float, float, float>> points;
		for (auto& kv : controlPoints) {
			if (kv.second.get_type() == sol::type::table) {
				sol::table pt = kv.second;
				float x = pt.get_or("x", 0.0f);
				float y = pt.get_or("y", 0.0f);
				float z = pt.get_or("z", 0.0f);
				points.push_back({x, y, z});
			}
		}

		if (points.size() < 2) {
			return result;
		}

		int index = 1;

		auto deCasteljau3d = [&](float t) -> std::tuple<float, float, float> {
			std::vector<std::tuple<float, float, float>> temp = points;

			while (temp.size() > 1) {
				std::vector<std::tuple<float, float, float>> newTemp;
				for (size_t i = 0; i < temp.size() - 1; ++i) {
					float x = std::get<0>(temp[i]) + t * (std::get<0>(temp[i + 1]) - std::get<0>(temp[i]));
					float y = std::get<1>(temp[i]) + t * (std::get<1>(temp[i + 1]) - std::get<1>(temp[i]));
					float z = std::get<2>(temp[i]) + t * (std::get<2>(temp[i + 1]) - std::get<2>(temp[i]));
					newTemp.push_back({x, y, z});
				}
				temp = newTemp;
			}

			return temp[0];
		};

		for (int i = 0; i <= numSteps; ++i) {
			float t = (float)i / (float)numSteps;
			auto [x, y, z] = deCasteljau3d(t);

			sol::table point = lua.create_table();
			point["x"] = std::round(x);
			point["y"] = std::round(y);
			point["z"] = std::round(z);
			result[index++] = point;
		}

		return result;
	});

	// ========================================
	// FLOOD FILL
	// ========================================

	// geo.floodFill(grid, startX, startY, newValue, options) -> grid
	// Flood fill algorithm (4-connected or 8-connected)
	geoTable.set_function("floodFill", [](sol::table inputGrid, int startX, int startY, int newValue, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);

		// Get dimensions
		int height = static_cast<int>(inputGrid.size());
		int width = 0;
		if (inputGrid[1].valid() && inputGrid[1].get_type() == sol::type::table) {
			sol::table firstRow = inputGrid[1];
			width = static_cast<int>(firstRow.size());
		}

		if (width <= 0 || height <= 0) return inputGrid;

		bool eightConnected = false;
		if (options) {
			sol::table opts = *options;
			eightConnected = opts.get_or("eightConnected", false);
		}

		// Convert to grid
		std::vector<std::vector<int>> grid(height, std::vector<int>(width, 0));
		for (int y = 1; y <= height; ++y) {
			if (inputGrid[y].valid() && inputGrid[y].get_type() == sol::type::table) {
				sol::table row = inputGrid[y];
				for (int x = 1; x <= width; ++x) {
					if (row[x].valid()) {
						grid[y - 1][x - 1] = row[x].get<int>();
					}
				}
			}
		}

		// Adjust for 1-indexed Lua
		int sx = startX - 1;
		int sy = startY - 1;

		if (sx < 0 || sx >= width || sy < 0 || sy >= height) {
			return inputGrid;
		}

		int oldValue = grid[sy][sx];
		if (oldValue == newValue) {
			return inputGrid;
		}

		// BFS flood fill
		std::queue<std::pair<int, int>> queue;
		queue.push({sx, sy});

		// Direction arrays
		const int dx4[] = {0, 1, 0, -1};
		const int dy4[] = {-1, 0, 1, 0};
		const int dx8[] = {0, 1, 1, 1, 0, -1, -1, -1};
		const int dy8[] = {-1, -1, 0, 1, 1, 1, 0, -1};

		const int* dx = eightConnected ? dx8 : dx4;
		const int* dy = eightConnected ? dy8 : dy4;
		int numDirs = eightConnected ? 8 : 4;

		while (!queue.empty()) {
			auto [cx, cy] = queue.front();
			queue.pop();

			if (cx < 0 || cx >= width || cy < 0 || cy >= height) continue;
			if (grid[cy][cx] != oldValue) continue;

			grid[cy][cx] = newValue;

			for (int i = 0; i < numDirs; ++i) {
				int nx = cx + dx[i];
				int ny = cy + dy[i];
				if (nx >= 0 && nx < width && ny >= 0 && ny < height && grid[ny][nx] == oldValue) {
					queue.push({nx, ny});
				}
			}
		}

		// Convert back to Lua table
		sol::table result = lua.create_table();
		for (int y = 0; y < height; ++y) {
			sol::table row = lua.create_table();
			for (int x = 0; x < width; ++x) {
				row[x + 1] = grid[y][x];
			}
			result[y + 1] = row;
		}

		return result;
	});

	// geo.getFloodFillPositions(grid, startX, startY, options) -> table of positions
	// Returns all positions that would be filled without modifying the grid
	geoTable.set_function("getFloodFillPositions", [](sol::table inputGrid, int startX, int startY, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		int height = static_cast<int>(inputGrid.size());
		int width = 0;
		if (inputGrid[1].valid() && inputGrid[1].get_type() == sol::type::table) {
			sol::table firstRow = inputGrid[1];
			width = static_cast<int>(firstRow.size());
		}

		if (width <= 0 || height <= 0) return result;

		bool eightConnected = false;
		if (options) {
			sol::table opts = *options;
			eightConnected = opts.get_or("eightConnected", false);
		}

		// Convert to grid
		std::vector<std::vector<int>> grid(height, std::vector<int>(width, 0));
		for (int y = 1; y <= height; ++y) {
			if (inputGrid[y].valid() && inputGrid[y].get_type() == sol::type::table) {
				sol::table row = inputGrid[y];
				for (int x = 1; x <= width; ++x) {
					if (row[x].valid()) {
						grid[y - 1][x - 1] = row[x].get<int>();
					}
				}
			}
		}

		int sx = startX - 1;
		int sy = startY - 1;

		if (sx < 0 || sx >= width || sy < 0 || sy >= height) {
			return result;
		}

		int targetValue = grid[sy][sx];

		std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
		std::queue<std::pair<int, int>> queue;
		queue.push({sx, sy});
		visited[sy][sx] = true;

		const int dx4[] = {0, 1, 0, -1};
		const int dy4[] = {-1, 0, 1, 0};
		const int dx8[] = {0, 1, 1, 1, 0, -1, -1, -1};
		const int dy8[] = {-1, -1, 0, 1, 1, 1, 0, -1};

		const int* dx = eightConnected ? dx8 : dx4;
		const int* dy = eightConnected ? dy8 : dy4;
		int numDirs = eightConnected ? 8 : 4;

		int index = 1;

		while (!queue.empty()) {
			auto [cx, cy] = queue.front();
			queue.pop();

			sol::table point = lua.create_table();
			point["x"] = cx + 1; // Back to 1-indexed
			point["y"] = cy + 1;
			result[index++] = point;

			for (int i = 0; i < numDirs; ++i) {
				int nx = cx + dx[i];
				int ny = cy + dy[i];
				if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
					!visited[ny][nx] && grid[ny][nx] == targetValue) {
					visited[ny][nx] = true;
					queue.push({nx, ny});
				}
			}
		}

		return result;
	});

	// ========================================
	// CIRCLE / ELLIPSE
	// ========================================

	// geo.circle(centerX, centerY, radius, options) -> table of points
	// Generate points on a circle (outline or filled)
	geoTable.set_function("circle", [](int centerX, int centerY, int radius, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		bool filled = false;
		if (options) {
			sol::table opts = *options;
			filled = opts.get_or("filled", false);
		}

		int index = 1;

		if (filled) {
			// Filled circle
			for (int y = -radius; y <= radius; ++y) {
				for (int x = -radius; x <= radius; ++x) {
					if (x * x + y * y <= radius * radius) {
						sol::table point = lua.create_table();
						point["x"] = centerX + x;
						point["y"] = centerY + y;
						result[index++] = point;
					}
				}
			}
		} else {
			// Midpoint circle algorithm (outline)
			int x = radius;
			int y = 0;
			int err = 0;

			auto addPoints = [&](int cx, int cy, int px, int py) {
				sol::table p1 = lua.create_table(); p1["x"] = cx + px; p1["y"] = cy + py; result[index++] = p1;
				sol::table p2 = lua.create_table(); p2["x"] = cx + py; p2["y"] = cy + px; result[index++] = p2;
				sol::table p3 = lua.create_table(); p3["x"] = cx - py; p3["y"] = cy + px; result[index++] = p3;
				sol::table p4 = lua.create_table(); p4["x"] = cx - px; p4["y"] = cy + py; result[index++] = p4;
				sol::table p5 = lua.create_table(); p5["x"] = cx - px; p5["y"] = cy - py; result[index++] = p5;
				sol::table p6 = lua.create_table(); p6["x"] = cx - py; p6["y"] = cy - px; result[index++] = p6;
				sol::table p7 = lua.create_table(); p7["x"] = cx + py; p7["y"] = cy - px; result[index++] = p7;
				sol::table p8 = lua.create_table(); p8["x"] = cx + px; p8["y"] = cy - py; result[index++] = p8;
			};

			while (x >= y) {
				addPoints(centerX, centerY, x, y);
				y++;

				if (err <= 0) {
					err += 2 * y + 1;
				} else {
					x--;
					err += 2 * (y - x) + 1;
				}
			}
		}

		return result;
	});

	// geo.ellipse(centerX, centerY, radiusX, radiusY, options) -> table of points
	geoTable.set_function("ellipse", [](int centerX, int centerY, int radiusX, int radiusY, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		bool filled = false;
		if (options) {
			sol::table opts = *options;
			filled = opts.get_or("filled", false);
		}

		int index = 1;

		if (filled) {
			for (int y = -radiusY; y <= radiusY; ++y) {
				for (int x = -radiusX; x <= radiusX; ++x) {
					float dx = (float)x / radiusX;
					float dy = (float)y / radiusY;
					if (dx * dx + dy * dy <= 1.0f) {
						sol::table point = lua.create_table();
						point["x"] = centerX + x;
						point["y"] = centerY + y;
						result[index++] = point;
					}
				}
			}
		} else {
			// Midpoint ellipse algorithm
			int rx2 = radiusX * radiusX;
			int ry2 = radiusY * radiusY;
			int twoRx2 = 2 * rx2;
			int twoRy2 = 2 * ry2;

			int x = 0;
			int y = radiusY;
			int px = 0;
			int py = twoRx2 * y;

			auto addEllipsePoints = [&](int cx, int cy, int ex, int ey) {
				sol::table p1 = lua.create_table(); p1["x"] = cx + ex; p1["y"] = cy + ey; result[index++] = p1;
				sol::table p2 = lua.create_table(); p2["x"] = cx - ex; p2["y"] = cy + ey; result[index++] = p2;
				sol::table p3 = lua.create_table(); p3["x"] = cx + ex; p3["y"] = cy - ey; result[index++] = p3;
				sol::table p4 = lua.create_table(); p4["x"] = cx - ex; p4["y"] = cy - ey; result[index++] = p4;
			};

			// Region 1
			int p = (int)(ry2 - rx2 * radiusY + 0.25 * rx2);
			while (px < py) {
				addEllipsePoints(centerX, centerY, x, y);
				x++;
				px += twoRy2;
				if (p < 0) {
					p += ry2 + px;
				} else {
					y--;
					py -= twoRx2;
					p += ry2 + px - py;
				}
			}

			// Region 2
			p = (int)(ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2);
			while (y >= 0) {
				addEllipsePoints(centerX, centerY, x, y);
				y--;
				py -= twoRx2;
				if (p > 0) {
					p += rx2 - py;
				} else {
					x++;
					px += twoRy2;
					p += rx2 - py + px;
				}
			}
		}

		return result;
	});

	// ========================================
	// RECTANGLE
	// ========================================

	// geo.rectangle(x1, y1, x2, y2, options) -> table of points
	geoTable.set_function("rectangle", [](int x1, int y1, int x2, int y2, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		bool filled = false;
		if (options) {
			sol::table opts = *options;
			filled = opts.get_or("filled", false);
		}

		int minX = std::min(x1, x2);
		int maxX = std::max(x1, x2);
		int minY = std::min(y1, y2);
		int maxY = std::max(y1, y2);

		int index = 1;

		if (filled) {
			for (int y = minY; y <= maxY; ++y) {
				for (int x = minX; x <= maxX; ++x) {
					sol::table point = lua.create_table();
					point["x"] = x;
					point["y"] = y;
					result[index++] = point;
				}
			}
		} else {
			// Outline only
			for (int x = minX; x <= maxX; ++x) {
				sol::table p1 = lua.create_table(); p1["x"] = x; p1["y"] = minY; result[index++] = p1;
				sol::table p2 = lua.create_table(); p2["x"] = x; p2["y"] = maxY; result[index++] = p2;
			}
			for (int y = minY + 1; y < maxY; ++y) {
				sol::table p1 = lua.create_table(); p1["x"] = minX; p1["y"] = y; result[index++] = p1;
				sol::table p2 = lua.create_table(); p2["x"] = maxX; p2["y"] = y; result[index++] = p2;
			}
		}

		return result;
	});

	// ========================================
	// POLYGON
	// ========================================

	// geo.polygon(vertices, options) -> table of points (outline using Bresenham)
	geoTable.set_function("polygon", [](sol::table vertices, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		// Parse vertices
		std::vector<std::pair<int, int>> verts;
		for (auto& kv : vertices) {
			if (kv.second.get_type() == sol::type::table) {
				sol::table pt = kv.second;
				int x = pt.get_or("x", pt.get_or(1, 0));
				int y = pt.get_or("y", pt.get_or(2, 0));
				verts.push_back({x, y});
			}
		}

		if (verts.size() < 3) {
			return result;
		}

		int index = 1;

		// Draw lines between consecutive vertices
		for (size_t i = 0; i < verts.size(); ++i) {
			int x1 = verts[i].first;
			int y1 = verts[i].second;
			int x2 = verts[(i + 1) % verts.size()].first;
			int y2 = verts[(i + 1) % verts.size()].second;

			// Bresenham
			int dx = std::abs(x2 - x1);
			int dy = std::abs(y2 - y1);
			int sx = (x1 < x2) ? 1 : -1;
			int sy = (y1 < y2) ? 1 : -1;
			int err = dx - dy;

			int x = x1, y = y1;

			while (true) {
				sol::table point = lua.create_table();
				point["x"] = x;
				point["y"] = y;
				result[index++] = point;

				if (x == x2 && y == y2) break;

				int e2 = 2 * err;
				if (e2 > -dy) {
					err -= dy;
					x += sx;
				}
				if (e2 < dx) {
					err += dx;
					y += sy;
				}
			}
		}

		return result;
	});

	// ========================================
	// DISTANCE FUNCTIONS
	// ========================================

	// geo.distance(x1, y1, x2, y2) -> number (Euclidean distance)
	geoTable.set_function("distance", [](float x1, float y1, float x2, float y2) -> float {
		float dx = x2 - x1;
		float dy = y2 - y1;
		return std::sqrt(dx * dx + dy * dy);
	});

	// geo.distanceSq(x1, y1, x2, y2) -> number (Squared distance, faster)
	geoTable.set_function("distanceSq", [](float x1, float y1, float x2, float y2) -> float {
		float dx = x2 - x1;
		float dy = y2 - y1;
		return dx * dx + dy * dy;
	});

	// geo.distanceManhattan(x1, y1, x2, y2) -> number
	geoTable.set_function("distanceManhattan", [](int x1, int y1, int x2, int y2) -> int {
		return std::abs(x2 - x1) + std::abs(y2 - y1);
	});

	// geo.distanceChebyshev(x1, y1, x2, y2) -> number (King's move distance)
	geoTable.set_function("distanceChebyshev", [](int x1, int y1, int x2, int y2) -> int {
		return std::max(std::abs(x2 - x1), std::abs(y2 - y1));
	});

	// ========================================
	// POINT IN SHAPE TESTS
	// ========================================

	// geo.pointInCircle(px, py, cx, cy, radius) -> boolean
	geoTable.set_function("pointInCircle", [](float px, float py, float cx, float cy, float radius) -> bool {
		float dx = px - cx;
		float dy = py - cy;
		return dx * dx + dy * dy <= radius * radius;
	});

	// geo.pointInRectangle(px, py, x1, y1, x2, y2) -> boolean
	geoTable.set_function("pointInRectangle", [](float px, float py, float x1, float y1, float x2, float y2) -> bool {
		float minX = std::min(x1, x2);
		float maxX = std::max(x1, x2);
		float minY = std::min(y1, y2);
		float maxY = std::max(y1, y2);
		return px >= minX && px <= maxX && py >= minY && py <= maxY;
	});

	// geo.pointInPolygon(px, py, vertices) -> boolean (Ray casting algorithm)
	geoTable.set_function("pointInPolygon", [](float px, float py, sol::table vertices) -> bool {
		std::vector<std::pair<float, float>> verts;
		for (auto& kv : vertices) {
			if (kv.second.get_type() == sol::type::table) {
				sol::table pt = kv.second;
				float x = pt.get_or("x", pt.get_or(1, 0.0f));
				float y = pt.get_or("y", pt.get_or(2, 0.0f));
				verts.push_back({x, y});
			}
		}

		if (verts.size() < 3) return false;

		bool inside = false;
		size_t n = verts.size();
		for (size_t i = 0, j = n - 1; i < n; j = i++) {
			float xi = verts[i].first, yi = verts[i].second;
			float xj = verts[j].first, yj = verts[j].second;

			if (((yi > py) != (yj > py)) &&
				(px < (xj - xi) * (py - yi) / (yj - yi) + xi)) {
				inside = !inside;
			}
		}

		return inside;
	});

	// ========================================
	// RANDOM SCATTER
	// ========================================

	// geo.randomScatter(x1, y1, x2, y2, count, options) -> table of points
	// Scatter random points in a region (useful for doodad placement)
	geoTable.set_function("randomScatter", [](int x1, int y1, int x2, int y2, int count, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		int seed = static_cast<int>(time(nullptr));
		int minDistance = 0;

		if (options) {
			sol::table opts = *options;
			seed = opts.get_or("seed", seed);
			minDistance = opts.get_or("minDistance", 0);
		}

		std::mt19937 rng(seed);
		std::uniform_int_distribution<int> distX(std::min(x1, x2), std::max(x1, x2));
		std::uniform_int_distribution<int> distY(std::min(y1, y2), std::max(y1, y2));

		std::vector<std::pair<int, int>> points;

		int attempts = 0;
		int maxAttempts = count * 100;

		while (points.size() < static_cast<size_t>(count) && attempts < maxAttempts) {
			attempts++;
			int x = distX(rng);
			int y = distY(rng);

			// Check minimum distance
			bool valid = true;
			if (minDistance > 0) {
				for (const auto& p : points) {
					int dx = x - p.first;
					int dy = y - p.second;
					if (dx * dx + dy * dy < minDistance * minDistance) {
						valid = false;
						break;
					}
				}
			}

			if (valid) {
				points.push_back({x, y});
			}
		}

		for (size_t i = 0; i < points.size(); ++i) {
			sol::table point = lua.create_table();
			point["x"] = points[i].first;
			point["y"] = points[i].second;
			result[i + 1] = point;
		}

		return result;
	});

	// geo.poissonDiskSampling(x1, y1, x2, y2, minDistance, options) -> table of points
	// Blue noise distribution (evenly spaced random points)
	geoTable.set_function("poissonDiskSampling", [](int x1, int y1, int x2, int y2, float minDistance, sol::optional<sol::table> options, sol::this_state s) -> sol::table {
		sol::state_view lua(s);
		sol::table result = lua.create_table();

		int seed = static_cast<int>(time(nullptr));
		int maxAttempts = 30;

		if (options) {
			sol::table opts = *options;
			seed = opts.get_or("seed", seed);
			maxAttempts = opts.get_or("maxAttempts", 30);
		}

		std::mt19937 rng(seed);

		int minX = std::min(x1, x2);
		int maxX = std::max(x1, x2);
		int minY = std::min(y1, y2);
		int maxY = std::max(y1, y2);

		float width = static_cast<float>(maxX - minX);
		float height = static_cast<float>(maxY - minY);

		float cellSize = minDistance / std::sqrt(2.0f);
		int gridWidth = static_cast<int>(std::ceil(width / cellSize));
		int gridHeight = static_cast<int>(std::ceil(height / cellSize));

		std::vector<std::vector<int>> grid(gridHeight, std::vector<int>(gridWidth, -1));
		std::vector<std::pair<float, float>> points;
		std::vector<size_t> activeList;

		// Start with first point
		std::uniform_real_distribution<float> distW(0, width);
		std::uniform_real_distribution<float> distH(0, height);

		float firstX = distW(rng);
		float firstY = distH(rng);
		points.push_back({firstX, firstY});
		activeList.push_back(0);

		int gx = static_cast<int>(firstX / cellSize);
		int gy = static_cast<int>(firstY / cellSize);
		if (gx >= 0 && gx < gridWidth && gy >= 0 && gy < gridHeight) {
			grid[gy][gx] = 0;
		}

		std::uniform_real_distribution<float> dist01(0, 1);

		while (!activeList.empty()) {
			size_t randIndex = rng() % activeList.size();
			size_t pointIndex = activeList[randIndex];
			auto& point = points[pointIndex];

			bool found = false;

			for (int k = 0; k < maxAttempts; ++k) {
				float angle = dist01(rng) * 2 * 3.14159265f;
				float r = minDistance + dist01(rng) * minDistance;

				float newX = point.first + r * std::cos(angle);
				float newY = point.second + r * std::sin(angle);

				if (newX < 0 || newX >= width || newY < 0 || newY >= height) continue;

				int ngx = static_cast<int>(newX / cellSize);
				int ngy = static_cast<int>(newY / cellSize);

				bool valid = true;

				// Check neighbors
				for (int dy = -2; dy <= 2 && valid; ++dy) {
					for (int dx = -2; dx <= 2 && valid; ++dx) {
						int cx = ngx + dx;
						int cy = ngy + dy;
						if (cx >= 0 && cx < gridWidth && cy >= 0 && cy < gridHeight) {
							int neighborIdx = grid[cy][cx];
							if (neighborIdx >= 0) {
								auto& neighbor = points[neighborIdx];
								float ddx = newX - neighbor.first;
								float ddy = newY - neighbor.second;
								if (ddx * ddx + ddy * ddy < minDistance * minDistance) {
									valid = false;
								}
							}
						}
					}
				}

				if (valid) {
					size_t newIdx = points.size();
					points.push_back({newX, newY});
					activeList.push_back(newIdx);
					if (ngx >= 0 && ngx < gridWidth && ngy >= 0 && ngy < gridHeight) {
						grid[ngy][ngx] = static_cast<int>(newIdx);
					}
					found = true;
					break;
				}
			}

			if (!found) {
				activeList.erase(activeList.begin() + randIndex);
			}
		}

		for (size_t i = 0; i < points.size(); ++i) {
			sol::table point = lua.create_table();
			point["x"] = static_cast<int>(points[i].first) + minX;
			point["y"] = static_cast<int>(points[i].second) + minY;
			result[i + 1] = point;
		}

		return result;
	});

	lua["geo"] = geoTable;
}

} // namespace LuaAPI
