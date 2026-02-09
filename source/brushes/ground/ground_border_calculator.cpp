//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/ground/ground_border_calculator.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/ground/auto_border.h"
#include "map/basemap.h"
#include "map/tile.h"
#include "game/item.h"
#include "game/items.h"
#include <array>
#include <algorithm>

void GroundBorderCalculator::calculate(BaseMap* map, Tile* tile) {
	static const auto extractGroundBrushFromTile = [](BaseMap* map, int x, int y, int z) -> GroundBrush* {
		Tile* tile = map->getTile(x, y, z);
		if (tile) {
			return tile->getGroundBrush();
		}
		return nullptr;
	};

	ASSERT(tile);

	GroundBrush* borderBrush;
	if (tile->ground) {
		borderBrush = tile->ground->getGroundBrush();
	} else {
		borderBrush = nullptr;
	}

	const Position& position = tile->getPosition();
	int x = position.x;
	int y = position.y;
	int z = position.z;

	// Pair of visited / what border type
	std::pair<bool, GroundBrush*> neighbours[8] = {
		{ false, nullptr }, { false, nullptr }, { false, nullptr }, { false, nullptr }, { false, nullptr }, { false, nullptr }, { false, nullptr }, { false, nullptr }
	};

	static constexpr std::array<std::pair<int32_t, int32_t>, 8> offsets = { { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } } };

	for (size_t i = 0; i < offsets.size(); ++i) {
		const auto& [dx, dy] = offsets[i];

		int nx = x + dx;
		int ny = y + dy;

		neighbours[i] = { false, extractGroundBrushFromTile(map, nx, ny, z) };
	}

	static std::vector<const GroundBrush::BorderBlock*> specificList;
	specificList.clear();

	std::vector<GroundBrush::BorderCluster> borderList;
	for (int32_t i = 0; i < 8; ++i) {
		auto& [visited, other] = neighbours[i];
		if (visited) {
			continue;
		}

		if (borderBrush) {
			if (other) {
				if (other->getID() == borderBrush->getID()) {
					continue;
				}

				if (other->hasOuterBorder() || borderBrush->hasInnerBorder()) {
					bool only_mountain = false;
					if (/*!borderBrush->hasInnerBorder() && */ (other->friendOf(borderBrush) || borderBrush->friendOf(other))) {
						if (!other->hasOptionalBorder()) {
							continue;
						}
						only_mountain = true;
					}

					uint32_t tiledata = 0;
					for (int32_t j = i; j < 8; ++j) {
						auto& [other_visited, other_brush] = neighbours[j];
						if (!other_visited && other_brush && other_brush->getID() == other->getID()) {
							other_visited = true;
							tiledata |= 1 << j;
						}
					}

					if (tiledata != 0) {
						// Add mountain if appropriate!
						if (other->hasOptionalBorder() && tile->hasOptionalBorder()) {
							GroundBrush::BorderCluster borderCluster;
							borderCluster.alignment = tiledata;
							borderCluster.z = 0x7FFFFFFF; // Above all other borders
							borderCluster.border = other->optional_border;

							borderList.push_back(borderCluster);
							if (other->useSoloOptionalBorder()) {
								only_mountain = true;
							}
						}

						if (!only_mountain) {
							const GroundBrush::BorderBlock* borderBlock = GroundBrush::getBrushTo(borderBrush, other);
							if (borderBlock) {
								bool found = false;
								for (GroundBrush::BorderCluster& borderCluster : borderList) {
									if (borderCluster.border == borderBlock->autoborder) {
										borderCluster.alignment |= tiledata;
										if (borderCluster.z < other->getZ()) {
											borderCluster.z = other->getZ();
										}

										if (!borderBlock->specific_cases.empty()) {
											specificList.push_back(borderBlock);
										}

										found = true;
										break;
									}
								}

								if (!found) {
									GroundBrush::BorderCluster borderCluster;
									borderCluster.alignment = tiledata;
									borderCluster.z = other->getZ();
									borderCluster.border = borderBlock->autoborder;

									borderList.push_back(borderCluster);
									if (!borderBlock->specific_cases.empty()) {
										specificList.push_back(borderBlock);
									}
								}
							}
						}
					}
				}
				// Border against nothing (or undefined tile)
				uint32_t tiledata = 0;
				for (int32_t j = i; j < 8; ++j) {
					auto& [other_visited, other_brush] = neighbours[j];
					if (!other_visited && !other_brush) {
						other_visited = true;
						tiledata |= 1 << j;
					}
				}

				if (tiledata != 0) {
					const GroundBrush::BorderBlock* borderBlock = GroundBrush::getBrushTo(borderBrush, nullptr);
					if (borderBlock) {
						if (borderBlock->autoborder) {
							bool found = false;
							for (GroundBrush::BorderCluster& borderCluster : borderList) {
								if (borderCluster.border == borderBlock->autoborder) {
									borderCluster.alignment |= tiledata;
									borderCluster.z = -1000;
									found = true;
									break;
								}
							}

							if (!found) {
								GroundBrush::BorderCluster borderCluster;
								borderCluster.alignment = tiledata;
								borderCluster.z = -1000;
								borderCluster.border = borderBlock->autoborder;
								borderList.push_back(borderCluster);
							}
						}

						if (!borderBlock->specific_cases.empty()) {
							specificList.push_back(borderBlock);
						}
					}
				}
				continue;
			} else {
				// Border against nothing (or undefined tile)
				uint32_t tiledata = 0;
				for (int32_t j = i; j < 8; ++j) {
					auto& [other_visited, other_brush] = neighbours[j];
					if (!other_visited && !other_brush) {
						other_visited = true;
						tiledata |= 1 << j;
					}
				}

				if (tiledata != 0) {
					const GroundBrush::BorderBlock* borderBlock = GroundBrush::getBrushTo(borderBrush, nullptr);
					if (borderBlock) {
						if (borderBlock->autoborder) {
							bool found = false;
							for (GroundBrush::BorderCluster& borderCluster : borderList) {
								if (borderCluster.border == borderBlock->autoborder) {
									borderCluster.alignment |= tiledata;
									borderCluster.z = -1000;
									found = true;
									break;
								}
							}

							if (!found) {
								GroundBrush::BorderCluster borderCluster;
								borderCluster.alignment = tiledata;
								borderCluster.z = -1000;
								borderCluster.border = borderBlock->autoborder;
								borderList.push_back(borderCluster);
							}
						}

						if (!borderBlock->specific_cases.empty()) {
							specificList.push_back(borderBlock);
						}
					}
				}
				continue;
			}
		} else if (other && other->hasOuterZilchBorder()) {
			uint32_t tiledata = 0;
			for (int32_t j = i; j < 8; ++j) {
				auto& [other_visited, other_brush] = neighbours[j];
				if (!other_visited && other_brush && other_brush->getID() == other->getID()) {
					other_visited = true;
					tiledata |= 1 << j;
				}
			}

			if (tiledata != 0) {
				const GroundBrush::BorderBlock* borderBlock = GroundBrush::getBrushTo(nullptr, other);
				if (borderBlock) {
					if (borderBlock->autoborder) {
						bool found = false;
						for (GroundBrush::BorderCluster& borderCluster : borderList) {
							if (borderCluster.border == borderBlock->autoborder) {
								borderCluster.alignment |= tiledata;
								if (borderCluster.z < other->getZ()) {
									borderCluster.z = other->getZ();
								}
								found = true;
								break;
							}
						}

						if (!found) {
							GroundBrush::BorderCluster borderCluster;
							borderCluster.alignment = tiledata;
							borderCluster.z = other->getZ();
							borderCluster.border = borderBlock->autoborder;
							borderList.push_back(borderCluster);
						}
					}

					if (!borderBlock->specific_cases.empty()) {
						specificList.push_back(borderBlock);
					}
				}

				// Add mountain if appropriate!
				if (other->hasOptionalBorder() && tile->hasOptionalBorder()) {
					GroundBrush::BorderCluster borderCluster;
					borderCluster.alignment = tiledata;
					borderCluster.z = 0x7FFFFFFF; // Above all other borders
					borderCluster.border = other->optional_border;

					borderList.push_back(borderCluster);
				} else {
					tile->setOptionalBorder(false);
				}
			}
		}
		// Check tile as done
		visited = true;
	}

	// Clean current borders
	std::erase_if(tile->items, [](Item* item) {
		if (item->isBorder()) {
			delete item;
			return true;
		}
		return false;
	});

	// Sort borders based on z-order
	std::ranges::sort(borderList, [](const GroundBrush::BorderCluster& a, const GroundBrush::BorderCluster& b) {
		return a.z < b.z;
	});

	std::ranges::sort(specificList);
	auto [first, last] = std::ranges::unique(specificList);
	specificList.erase(first, last);

	tile->cleanBorders();

	while (!borderList.empty()) {
		GroundBrush::BorderCluster& borderCluster = borderList.back();
		if (!borderCluster.border) {
			borderList.pop_back();
			continue;
		}

		uint32_t border_alignment = borderCluster.alignment;

		BorderType directions[4] = {
			static_cast<BorderType>((GroundBrush::border_types[border_alignment] & 0x000000FF) >> 0),
			static_cast<BorderType>((GroundBrush::border_types[border_alignment] & 0x0000FF00) >> 8),
			static_cast<BorderType>((GroundBrush::border_types[border_alignment] & 0x00FF0000) >> 16),
			static_cast<BorderType>((GroundBrush::border_types[border_alignment] & 0xFF000000) >> 24)
		};

		for (int32_t i = 0; i < 4; ++i) {
			BorderType direction = directions[i];
			if (direction == BORDER_NONE) {
				break;
			}

			if (borderCluster.border->tiles[direction] != 0) {
				tile->addBorderItem(Item::Create(borderCluster.border->tiles[direction]));
			} else {
				if (direction == NORTHWEST_DIAGONAL) {
					if (borderCluster.border->tiles[WEST_HORIZONTAL] != 0 && borderCluster.border->tiles[NORTH_HORIZONTAL] != 0) {
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
					}
				} else if (direction == NORTHEAST_DIAGONAL) {
					if (borderCluster.border->tiles[EAST_HORIZONTAL] != 0 && borderCluster.border->tiles[NORTH_HORIZONTAL] != 0) {
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
					}
				} else if (direction == SOUTHWEST_DIAGONAL) {
					if (borderCluster.border->tiles[SOUTH_HORIZONTAL] != 0 && borderCluster.border->tiles[WEST_HORIZONTAL] != 0) {
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
					}
				} else if (direction == SOUTHEAST_DIAGONAL) {
					if (borderCluster.border->tiles[SOUTH_HORIZONTAL] != 0 && borderCluster.border->tiles[EAST_HORIZONTAL] != 0) {
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
						tile->addBorderItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
					}
				}
			}
		}

		borderList.pop_back();
	}

	for (const GroundBrush::BorderBlock* borderBlock : specificList) {
		for (const GroundBrush::SpecificCaseBlock* specificCaseBlock : borderBlock->specific_cases) {
			uint32_t matches = 0;
			for (Item* item : tile->items) {
				if (!item->isBorder()) {
					break;
				}

				if (specificCaseBlock->match_group > 0) {
					if (item->getBorderGroup() == specificCaseBlock->match_group && item->getBorderAlignment() == specificCaseBlock->group_match_alignment) {
						++matches;
						continue;
					}
				}

				for (uint16_t matchId : specificCaseBlock->items_to_match) {
					if (item->getID() == matchId) {
						++matches;
					}
				}
			}

			if (matches >= specificCaseBlock->items_to_match.size()) {
				auto& tileItems = tile->items;
				auto it = tileItems.begin();

				// if delete_all mode, consider the border replaced
				bool replaced = specificCaseBlock->delete_all;

				while (it != tileItems.end()) {
					Item* item = *it;
					if (!item->isBorder()) {
						++it;
						continue;
					}

					bool inc = true;
					for (uint16_t matchId : specificCaseBlock->items_to_match) {
						if (item->getID() == matchId) {
							if (!replaced && item->getID() == specificCaseBlock->to_replace_id) {
								// replace the matching border, delete everything else
								item->setID(specificCaseBlock->with_id);
								replaced = true;
							} else {
								if (specificCaseBlock->delete_all || !specificCaseBlock->keepBorder) {
									delete item;
									it = tileItems.erase(it);
									inc = false;
									break;
								}
							}
						}
					}

					if (inc) {
						++it;
					}
				}
			}
		}
	}
}
