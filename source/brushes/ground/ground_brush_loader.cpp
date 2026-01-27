//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/ground/ground_brush_loader.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/ground/auto_border.h"
#include "brushes/brush.h"
#include "game/items.h"
#include "ext/pugixml.hpp"
#include <wx/string.h>

extern Brushes g_brushes;

// Helper for C++20 case-insensitive comparison (zero allocation)
static const auto iequal = [](char a, char b) {
	return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
};

bool GroundBrushLoader::load(GroundBrush& brush, pugi::xml_node node, wxArrayString& warnings) {
	pugi::xml_attribute attribute;
	if ((attribute = node.attribute("lookid"))) {
		brush.look_id = attribute.as_ushort();
	}

	if ((attribute = node.attribute("server_lookid"))) {
		brush.look_id = g_items[attribute.as_ushort()].clientID;
	}

	if ((attribute = node.attribute("z-order"))) {
		brush.z_order = attribute.as_int();
	}

	if ((attribute = node.attribute("solo_optional"))) {
		brush.use_only_optional = attribute.as_bool();
	}

	if ((attribute = node.attribute("randomize"))) {
		brush.randomize = attribute.as_bool();
	}

	for (pugi::xml_node childNode : node.children()) {
		std::string_view childName = childNode.name();
		if (std::ranges::equal(childName, std::string_view("item"), iequal)) {
			uint16_t itemId = childNode.attribute("id").as_ushort();
			int32_t chance = 1;
			if (auto attribute = childNode.attribute("chance")) {
				chance = attribute.as_int();
			}

			if (chance < 0) {
				warnings.push_back("\nChance for ground item " + std::to_string(itemId) + " is negative, defaulting to 0.");
				chance = 0;
			}

			ItemType& it = g_items[itemId];
			if (it.id == 0) {
				warnings.push_back("\nInvalid item id " + std::to_string(itemId));
				return false;
			}

			if (!it.isGroundTile()) {
				warnings.push_back("\nItem " + std::to_string(itemId) + " is not ground item.");
				return false;
			}

			if (it.brush && it.brush != &brush) {
				warnings.push_back("\nItem " + std::to_string(itemId) + " can not be member of two brushes");
				return false;
			}

			it.brush = &brush;
			brush.total_chance += chance;

			GroundBrush::ItemChanceBlock ci;
			ci.id = itemId;
			ci.chance = brush.total_chance;
			brush.border_items.push_back(ci);
		} else if (std::ranges::equal(childName, std::string_view("optional"), iequal)) {
			// Mountain border!
			if (brush.optional_border) {
				warnings.push_back("\nDuplicate optional borders!");
				continue;
			}

			if ((attribute = childNode.attribute("ground_equivalent"))) {
				uint16_t ground_equivalent = attribute.as_ushort();

				// Load from inline definition
				ItemType& it = g_items[ground_equivalent];
				if (it.id == 0) {
					warnings.push_back("Invalid id of ground dependency equivalent item.\n");
					continue;
				} else if (!it.isGroundTile()) {
					warnings.push_back("Ground dependency equivalent is not a ground item.\n");
					continue;
				} else if (it.brush && it.brush != &brush) {
					warnings.push_back("Ground dependency equivalent does not use the same brush as ground border.\n");
					continue;
				}

				AutoBorder* autoBorder = newd AutoBorder(0); // Empty id basically
				autoBorder->ground = true;
				autoBorder->load(childNode, warnings, &brush, ground_equivalent);
				brush.optional_border = autoBorder;
			} else {
				// Load from ID
				if (!(attribute = childNode.attribute("id"))) {
					warnings.push_back("\nMissing tag id for border node");
					continue;
				}

				uint16_t id = attribute.as_ushort();
				auto it = g_brushes.borders.find(id);
				if (it == g_brushes.borders.end() || !it->second) {
					warnings.push_back("\nCould not find border id " + std::to_string(id));
					continue;
				}

				brush.optional_border = it->second.get();
			}
		} else if (std::ranges::equal(childName, std::string_view("border"), iequal)) {
			AutoBorder* autoBorder;
			if (!(attribute = childNode.attribute("id"))) {
				if (!(attribute = childNode.attribute("ground_equivalent"))) {
					continue;
				}

				uint16_t ground_equivalent = attribute.as_ushort();
				ItemType& it = g_items[ground_equivalent];
				bool valid = true;
				if (it.id == 0) {
					warnings.push_back("Invalid id of ground dependency equivalent item.\n");
					valid = false;
				} else if (!it.isGroundTile()) { // Changed to else if to avoid duplicate warnings
					warnings.push_back("Ground dependency equivalent is not a ground item.\n");
					valid = false;
				} else if (it.brush && it.brush != &brush) {
					warnings.push_back("Ground dependency equivalent does not use the same brush as ground border.\n");
					valid = false;
				}

				if (valid) {
					autoBorder = newd AutoBorder(0); // Empty id basically
					autoBorder->ground = true;
					autoBorder->load(childNode, warnings, &brush, ground_equivalent);
				} else {
					continue;
				}
			} else {
				int32_t id = attribute.as_int();
				if (id == 0) {
					autoBorder = nullptr;
				} else {
					auto it = g_brushes.borders.find(id);
					if (it == g_brushes.borders.end() || !it->second) {
						warnings.push_back("\nCould not find border id " + std::to_string(id));
						continue;
					}
					autoBorder = it->second.get();
				}
			}

			auto borderBlock = std::make_unique<GroundBrush::BorderBlock>();
			borderBlock->super = false;
			borderBlock->outer = true;
			borderBlock->autoborder = autoBorder;

			if ((attribute = childNode.attribute("to"))) {
				const std::string_view value = attribute.as_string();
				if (value == "all") {
					borderBlock->to = 0xFFFFFFFF;
				} else if (value == "none") {
					borderBlock->to = 0;
				} else {
					Brush* tobrush = g_brushes.getBrush(value);
					if (!tobrush) {
						warnings.push_back("To brush " + wxstr(value) + " doesn't exist.");
						if (autoBorder && autoBorder->ground) {
							delete autoBorder;
						}
						continue;
					}
					borderBlock->to = tobrush->getID();
				}
			} else {
				borderBlock->to = 0xFFFFFFFF;
			}

			if ((attribute = childNode.attribute("super")) && attribute.as_bool()) {
				borderBlock->super = true;
			}

			if ((attribute = childNode.attribute("align"))) {
				const std::string_view value = attribute.as_string();
				if (value == "outer") {
					borderBlock->outer = true;
				} else if (value == "inner") {
					borderBlock->outer = false;
				} else {
					borderBlock->outer = true;
				}
			}

			if (borderBlock->outer) {
				if (borderBlock->to == 0) {
					brush.has_zilch_outer_border = true;
				} else {
					brush.has_outer_border = true;
				}
			} else {
				if (borderBlock->to == 0) {
					brush.has_zilch_inner_border = true;
				} else {
					brush.has_inner_border = true;
				}
			}

			for (pugi::xml_node subChildNode : childNode.children()) {
				if (!std::ranges::equal(std::string_view(subChildNode.name()), std::string_view("specific"), iequal)) {
					continue;
				}

				GroundBrush::SpecificCaseBlock* specificCaseBlock = nullptr;
				for (pugi::xml_node superChildNode : subChildNode.children()) {
					std::string_view superChildName = superChildNode.name();
					if (std::ranges::equal(superChildName, std::string_view("conditions"), iequal)) {
						for (pugi::xml_node conditionChild : superChildNode.children()) {
							std::string_view conditionName = conditionChild.name();
							if (std::ranges::equal(conditionName, std::string_view("match_border"), iequal)) {
								if (!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = attribute.as_int();
								if (!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								auto it = g_brushes.borders.find(border_id);
								if (it == g_brushes.borders.end()) {
									warnings.push_back("Unknown border id in specific case match block " + std::to_string(border_id));
									continue;
								}

								AutoBorder* autoBorder = it->second.get();
								ASSERT(autoBorder != nullptr);

								uint32_t match_itemid = autoBorder->tiles[edge_id];
								if (!specificCaseBlock) {
									specificCaseBlock = newd GroundBrush::SpecificCaseBlock();
								}
								specificCaseBlock->items_to_match.push_back(match_itemid);
							} else if (std::ranges::equal(conditionName, std::string_view("match_group"), iequal)) {
								if (!(attribute = conditionChild.attribute("group"))) {
									continue;
								}

								uint16_t group = attribute.as_ushort();
								if (!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if (!specificCaseBlock) {
									specificCaseBlock = newd GroundBrush::SpecificCaseBlock();
								}

								specificCaseBlock->match_group = group;
								specificCaseBlock->group_match_alignment = ::BorderType(edge_id);
								specificCaseBlock->items_to_match.push_back(group);
							} else if (std::ranges::equal(conditionName, std::string_view("match_item"), iequal)) {
								if (!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t match_itemid = attribute.as_int();
								if (!specificCaseBlock) {
									specificCaseBlock = newd GroundBrush::SpecificCaseBlock();
								}

								specificCaseBlock->match_group = 0;
								specificCaseBlock->items_to_match.push_back(match_itemid);
							}
						}
					} else if (std::ranges::equal(superChildName, std::string_view("actions"), iequal)) {
						for (pugi::xml_node actionChild : superChildNode.children()) {
							std::string_view actionName = actionChild.name();
							if (std::ranges::equal(actionName, std::string_view("replace_border"), iequal)) {
								if (!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = attribute.as_int();
								if (!(attribute = actionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if (!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = attribute.as_int();
								auto itt = g_brushes.borders.find(border_id);
								if (itt == g_brushes.borders.end()) {
									warnings.push_back("Unknown border id in specific case match block " + std::to_string(border_id));
									continue;
								}

								AutoBorder* autoBorder = itt->second.get();
								ASSERT(autoBorder != nullptr);

								ItemType& it = g_items[with_id];
								if (it.id == 0) {
									return false;
								}

								it.isBorder = true;
								if (!specificCaseBlock) {
									specificCaseBlock = newd GroundBrush::SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = autoBorder->tiles[edge_id];
								specificCaseBlock->with_id = with_id;
							} else if (std::ranges::equal(actionName, std::string_view("replace_item"), iequal)) {
								if (!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t to_replace_id = attribute.as_int();
								if (!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = attribute.as_int();
								ItemType& it = g_items[with_id];
								if (it.id == 0) {
									return false;
								}

								it.isBorder = true;
								if (!specificCaseBlock) {
									specificCaseBlock = newd GroundBrush::SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = to_replace_id;
								specificCaseBlock->with_id = with_id;
							} else if (std::ranges::equal(actionName, std::string_view("delete_borders"), iequal)) {
								if (!specificCaseBlock) {
									specificCaseBlock = newd GroundBrush::SpecificCaseBlock();
								}
								specificCaseBlock->delete_all = true;
							}
						}
					}
				}
				if (specificCaseBlock) {
					if ((attribute = subChildNode.attribute("keep_border"))) {
						specificCaseBlock->keepBorder = attribute.as_bool();
					}

					borderBlock->specific_cases.push_back(specificCaseBlock);
				}
			}
			brush.borders.push_back(borderBlock.release());
		} else if (std::ranges::equal(childName, std::string_view("friend"), iequal)) {
			const std::string_view name = childNode.attribute("name").as_string();
			if (!name.empty()) {
				if (name == "all") {
					brush.friends.push_back(0xFFFFFFFF);
				} else {
					Brush* otherBrush = g_brushes.getBrush(name);
					if (otherBrush) {
						brush.friends.push_back(otherBrush->getID());
					} else {
						warnings.push_back("Brush '" + wxstr(name) + "' is not defined.");
					}
				}
			}
			brush.hate_friends = false;
		} else if (std::ranges::equal(childName, std::string_view("enemy"), iequal)) {
			const std::string_view name = childNode.attribute("name").as_string();
			if (!name.empty()) {
				if (name == "all") {
					brush.friends.push_back(0xFFFFFFFF);
				} else {
					Brush* otherBrush = g_brushes.getBrush(name);
					if (otherBrush) {
						brush.friends.push_back(otherBrush->getID());
					} else {
						warnings.push_back("Brush '" + wxstr(name) + "' is not defined.");
					}
				}
			}
			brush.hate_friends = true;
		} else if (std::ranges::equal(childName, std::string_view("clear_borders"), iequal)) {
			for (GroundBrush::BorderBlock* bb : brush.borders) {
				if (bb->autoborder) {
					for (GroundBrush::SpecificCaseBlock* specificCaseBlock : bb->specific_cases) {
						delete specificCaseBlock;
					}
					if (bb->autoborder->ground) {
						delete bb->autoborder;
					}
				}
				delete bb;
			}
			brush.borders.clear();
		} else if (std::ranges::equal(childName, std::string_view("clear_friends"), iequal)) {
			brush.friends.clear();
			brush.hate_friends = false;
		}
	}

	if (brush.total_chance == 0) {
		brush.randomize = false;
	}

	return true;
}
