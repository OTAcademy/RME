//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_AUTO_BORDER_H
#define RME_AUTO_BORDER_H

#include "app/main.h"
#include <array>
#include <string>
#include <vector>

class GroundBrush;
class wxArrayString;
namespace pugi {
	class xml_node;
}

/**
 * @brief Handles auto-bordering logic for brushes.
 *
 * The AutoBorder class defines a set of rules and tiles used to automatically
 * create borders around ground tiles. It maps specific edge configurations
 * (north, south, corners, etc.) to item IDs.
 */
class AutoBorder {
public:
	/**
	 * @brief Constructs a new AutoBorder object with a given ID.
	 *
	 * @param id The unique identifier for this border configuration.
	 */
	explicit AutoBorder(int id);

	/**
	 * @brief Destroys the AutoBorder object.
	 */
	~AutoBorder() = default;

	/**
	 * @brief Converts a string representation of an edge to its integer ID.
	 *
	 * @param edgename The name of the edge (e.g., "n", "cnw").
	 * @return int The corresponding integer ID for the edge, or BORDER_NONE if invalid.
	 */
	static int edgeNameToID(std::string_view edgename);

	/**
	 * @brief Loads the auto-border configuration from an XML node.
	 *
	 * @param node The XML node containing the border configuration.
	 * @param warnings A list to append any warnings encountered during loading.
	 * @param owner The GroundBrush that owns this border (optional).
	 * @param ground_equivalent The ID of the ground item this border is equivalent to (optional).
	 * @return true if loading was successful, false otherwise.
	 */
	bool load(pugi::xml_node node, wxArrayString& warnings, GroundBrush* owner = nullptr, uint16_t ground_equivalent = 0);

	/**
	 * @brief Array of tile IDs for each border direction.
	 * Indices correspond to the direction mapping (e.g., NORTH_HORIZONTAL).
	 */
	std::array<uint32_t, 13> tiles;

	/**
	 * @brief The unique ID of this auto-border.
	 */
	uint32_t id;

	/**
	 * @brief Group ID for this border, used for matching specific cases.
	 */
	uint16_t group;

	/**
	 * @brief Flag indicating if this is a ground border.
	 */
	bool ground;
};

#endif // RME_AUTO_BORDER_H
