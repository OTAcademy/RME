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
#include <algorithm>

void GroundBorderCalculator::calculate(BaseMap* map, Tile* tile) {
	static const auto extractGroundBrushFromTile = [](BaseMap* map, uint32_t x, uint32_t y, uint32_t z) -> GroundBrush* {
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
	uint32_t x = position.x;
	uint32_t y = position.y;
	uint32_t z = position.z;

	// Pair of visited / what border type
	std::pair<bool, GroundBrush*> neighbours[8];
	if (x == 0) {
		if (y == 0) {
			neighbours[0] = { false, nullptr };
			neighbours[1] = { false, nullptr };
			neighbours[2] = { false, nullptr };
			neighbours[3] = { false, nullptr };
			neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y, z) };
			neighbours[5] = { false, nullptr };
			neighbours[6] = { false, extractGroundBrushFromTile(map, x, y + 1, z) };
			neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
		} else {
			neighbours[0] = { false, nullptr };
			neighbours[1] = { false, extractGroundBrushFromTile(map, x, y - 1, z) };
			neighbours[2] = { false, extractGroundBrushFromTile(map, x + 1, y - 1, z) };
			neighbours[3] = { false, nullptr };
			neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y, z) };
			neighbours[5] = { false, nullptr };
			neighbours[6] = { false, extractGroundBrushFromTile(map, x, y + 1, z) };
			neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
		}
	} else if (y == 0) {
		neighbours[0] = { false, nullptr };
		neighbours[1] = { false, nullptr };
		neighbours[2] = { false, nullptr };
		neighbours[3] = { false, extractGroundBrushFromTile(map, x - 1, y, z) };
		neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y, z) };
		neighbours[5] = { false, extractGroundBrushFromTile(map, x - 1, y + 1, z) };
		neighbours[6] = { false, extractGroundBrushFromTile(map, x, y + 1, z) };
		neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
	} else {
		neighbours[0] = { false, extractGroundBrushFromTile(map, x - 1, y - 1, z) };
		neighbours[1] = { false, extractGroundBrushFromTile(map, x, y - 1, z) };
		neighbours[2] = { false, extractGroundBrushFromTile(map, x + 1, y - 1, z) };
		neighbours[3] = { false, extractGroundBrushFromTile(map, x - 1, y, z) };
		neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y, z) };
		neighbours[5] = { false, extractGroundBrushFromTile(map, x - 1, y + 1, z) };
		neighbours[6] = { false, extractGroundBrushFromTile(map, x, y + 1, z) };
		neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
	}

	static std::vector<const GroundBrush::BorderBlock*> specificList;
	specificList.clear();

	std::vector<GroundBrush::BorderCluster> borderList;
	for (int32_t i = 0; i < 8; ++i) {
		auto& neighbourPair = neighbours[i];
		if (neighbourPair.first) {
			continue;
		}

		GroundBrush* other = neighbourPair.second;
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
						auto& otherPair = neighbours[j];
						if (!otherPair.first && otherPair.second && otherPair.second->getID() == other->getID()) {
							otherPair.first = true;
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
			} else if (borderBrush->hasInnerZilchBorder()) {
				// Border against nothing (or undefined tile)
				uint32_t tiledata = 0;
				for (int32_t j = i; j < 8; ++j) {
					auto& otherPair = neighbours[j];
					if (!otherPair.first && !otherPair.second) {
						otherPair.first = true;
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
				auto& otherPair = neighbours[j];
				if (!otherPair.first && otherPair.second && otherPair.second->getID() == other->getID()) {
					otherPair.first = true;
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
		neighbourPair.first = true;
	}

	ItemVector& items = tile->items;
	// Clean current borders
	for (auto it = items.begin(); it != items.end();) {
		Item* item = *it;
		if (item->isBorder()) {
			delete item;
			it = items.erase(it);
		} else {
			++it;
		}
	}

	// Sort borders based on z-order
	std::sort(borderList.begin(), borderList.end());

	std::sort(specificList.begin(), specificList.end());
	auto last = std::unique(specificList.begin(), specificList.end());
	specificList.erase(last, specificList.end());

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

			if (borderCluster.border->tiles[direction]) {
				tile->addBorderItem(Item::Create(borderCluster.border->tiles[direction]));
			} else {
				if (direction == NORTHWEST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
				} else if (direction == NORTHEAST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
				} else if (direction == SOUTHWEST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
				} else if (direction == SOUTHEAST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
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
