//////////////////////////////////////////////////////////////////////
// Test for Autoborder Preview Manager
// This test verifies the fix for the crash when AutoBorder is enabled with ground brush
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cassert>
#include <memory>
#include "map/basemap.h"
#include "map/tile.h"
#include "map/map_region.h"
#include "map/position.h"

// Test 1: Verify Tile::deepCopy uses destination map's TileLocation
void test_deepCopy_uses_destination_location() {
	std::cout << "Test 1: deepCopy uses destination map's TileLocation..." << std::endl;

	BaseMap source_map;
	BaseMap preview_map;

	// Create a tile in source map
	Tile* source_tile = source_map.createTile(100, 100, 7);
	assert(source_tile != nullptr);
	assert(source_tile->getLocation() != nullptr);

	TileLocation* source_loc = source_tile->getLocation();
	std::cout << "  Source tile location: " << source_loc << std::endl;

	// Deep copy to preview map
	std::unique_ptr<Tile> copied = source_tile->deepCopy(preview_map);

	// The copied tile should use the preview map's TileLocation, not source map's
	TileLocation* copy_loc = copied->getLocation();
	std::cout << "  Copied tile location: " << copy_loc << std::endl;
	std::cout << "  Preview map's TileLocation at (100,100,7): " << preview_map.getTileL(100, 100, 7) << std::endl;

	// Verify the copy's location is in the preview map, not source map
	assert(copy_loc != nullptr);
	assert(copy_loc != source_loc); // Should NOT be the source location
	assert(copy_loc == preview_map.getTileL(100, 100, 7)); // Should be preview map's location

	// Verify position is correct
	assert(copied->getX() == 100);
	assert(copied->getY() == 100);
	assert(copied->getZ() == 7);

	std::cout << "  Position: (" << copied->getX() << ", " << copied->getY() << ", " << copied->getZ() << ")" << std::endl;
	std::cout << "Test 1: PASSED - deepCopy correctly uses destination map's TileLocation" << std::endl;
}

// Test 2: Verify copied tile works correctly after being placed in preview map
void test_copied_tile_after_setTile() {
	std::cout << "\nTest 2: Copied tile works correctly after setTile..." << std::endl;

	BaseMap source_map;
	BaseMap preview_map;

	// Create a tile with some data in source map
	Tile* source_tile = source_map.createTile(200, 200, 7);
	assert(source_tile != nullptr);

	// Deep copy
	std::unique_ptr<Tile> copied = source_tile->deepCopy(preview_map);

	// Verify location is correct before setTile
	assert(copied->getLocation() == preview_map.getTileL(200, 200, 7));

	// Place in preview map
	preview_map.setTile(std::move(copied));

	// Get tile back
	Tile* preview_tile = preview_map.getTile(200, 200, 7);
	assert(preview_tile != nullptr);

	// Verify location is still correct after setTile
	assert(preview_tile->getLocation() != nullptr);
	assert(preview_tile->getLocation() == preview_map.getTileL(200, 200, 7));

	// Verify position accessors work (this is where the crash would happen)
	Position pos = preview_tile->getPosition();
	assert(pos.x == 200);
	assert(pos.y == 200);
	assert(pos.z == 7);

	std::cout << "  Position after setTile: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
	std::cout << "Test 2: PASSED - Copied tile works correctly after setTile" << std::endl;
}

// Test 3: Verify getPosition() works on copied tiles (the crash scenario)
void test_getPosition_on_copied_tile() {
	std::cout << "\nTest 3: getPosition() on copied tile (crash scenario)..." << std::endl;

	BaseMap source_map;
	BaseMap preview_map;

	// Create source tile
	Tile* source_tile = source_map.createTile(300, 300, 7);
	assert(source_tile != nullptr);

	// Copy and place in preview map
	std::unique_ptr<Tile> copied = source_tile->deepCopy(preview_map);
	preview_map.setTile(std::move(copied));

	Tile* preview_tile = preview_map.getTile(300, 300, 7);
	assert(preview_tile != nullptr);
	assert(preview_tile->getLocation() != nullptr);

	// This is what GroundBorderCalculator::calculate() does - call getPosition()
	// If location is null or wrong, this will crash or return wrong data
	const Position& position = preview_tile->getPosition();

	std::cout << "  getPosition() returned: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;

	assert(position.x == 300);
	assert(position.y == 300);
	assert(position.z == 7);

	std::cout << "Test 3: PASSED - getPosition() works correctly on copied tile" << std::endl;
}

// Test 4: Test edge case - position (0, 0, 0)
void test_edge_case_zero_position() {
	std::cout << "\nTest 4: Edge case - position (0, 0, 0)..." << std::endl;

	BaseMap source_map;
	BaseMap preview_map;

	// Create tile at (0, 0, 0)
	Tile* source_tile = source_map.createTile(0, 0, 0);
	assert(source_tile != nullptr);

	// Deep copy
	std::unique_ptr<Tile> copied = source_tile->deepCopy(preview_map);

	// Verify location is in preview map
	assert(copied->getLocation() != nullptr);
	assert(copied->getLocation() == preview_map.getTileL(0, 0, 0));

	preview_map.setTile(std::move(copied));

	Tile* preview_tile = preview_map.getTile(0, 0, 0);
	assert(preview_tile != nullptr);
	assert(preview_tile->getLocation() != nullptr);

	// Test getPosition at edge
	Position pos = preview_tile->getPosition();
	assert(pos.x == 0);
	assert(pos.y == 0);
	assert(pos.z == 0);

	std::cout << "  Position at (0,0,0): (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
	std::cout << "Test 4: PASSED" << std::endl;
}

// Test 5: Simulate the AutoborderPreviewManager::CopyMapArea scenario
void test_copyMapArea_scenario() {
	std::cout << "\nTest 5: Simulate CopyMapArea scenario..." << std::endl;

	BaseMap source_map;
	BaseMap preview_map;

	// Create some tiles in source map (simulating editor.map)
	for (int y = 90; y <= 110; ++y) {
		for (int x = 90; x <= 110; ++x) {
			source_map.createTile(x, y, 7);
		}
	}

	// Simulate CopyMapArea
	int range = 10;
	int center_x = 100;
	int center_y = 100;
	int z = 7;

	for (int y = center_y - range; y <= center_y + range; ++y) {
		for (int x = center_x - range; x <= center_x + range; ++x) {
			Tile* src_tile = source_map.getTile(x, y, z);
			if (src_tile) {
				std::unique_ptr<Tile> new_tile = src_tile->deepCopy(preview_map);

				// Verify the copy has correct location BEFORE setTile
				assert(new_tile->getLocation() != nullptr);
				assert(new_tile->getLocation() == preview_map.getTileL(x, y, z));

				preview_map.setTile(std::move(new_tile));

				// Verify tile is accessible and has correct location AFTER setTile
				Tile* copied_tile = preview_map.getTile(x, y, z);
				assert(copied_tile != nullptr);
				assert(copied_tile->getLocation() != nullptr);
				assert(copied_tile->getLocation() == preview_map.getTileL(x, y, z));

				// This is what would crash before the fix
				Position pos = copied_tile->getPosition();
				assert(pos.x == x);
				assert(pos.y == y);
				assert(pos.z == z);
			}
		}
	}

	std::cout << "  Copied " << (range * 2 + 1) * (range * 2 + 1) << " tiles successfully" << std::endl;
	std::cout << "Test 5: PASSED - CopyMapArea scenario works correctly" << std::endl;
}

int main() {
	std::cout << "========================================" << std::endl;
	std::cout << "Autoborder Preview Manager Test Suite" << std::endl;
	std::cout << "Testing fix for: Crash with AutoBorder + Ground Brush" << std::endl;
	std::cout << "========================================" << std::endl;

	try {
		test_deepCopy_uses_destination_location();
		test_copied_tile_after_setTile();
		test_getPosition_on_copied_tile();
		test_edge_case_zero_position();
		test_copyMapArea_scenario();

		std::cout << "\n========================================" << std::endl;
		std::cout << "ALL TESTS PASSED!" << std::endl;
		std::cout << "The fix correctly addresses the crash issue." << std::endl;
		std::cout << "========================================" << std::endl;

		return 0;
	} catch (const std::exception& e) {
		std::cerr << "\nTEST FAILED with exception: " << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "\nTEST FAILED with unknown exception" << std::endl;
		return 1;
	}
}
