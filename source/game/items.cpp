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

#include "app/main.h"
#include <string_view>
#include <map>
#include <unordered_map>
#include <array>
#include <format>

#include "game/materials.h"
#include "app/managers/version_manager.h"
#include "ui/gui.h"
#include <string.h> // memcpy

#include "game/items.h"
#include "game/item.h"

ItemDatabase g_items;

ItemType::ItemType() :
	sprite(nullptr),
	id(0),
	clientID(0),
	brush(nullptr),
	doodad_brush(nullptr),
	collection_brush(nullptr),
	raw_brush(nullptr),
	is_metaitem(false),
	has_raw(false),
	in_other_tileset(false),
	group(ITEM_GROUP_NONE),
	type(ITEM_TYPE_NONE),
	volume(0),
	maxTextLen(0),
	// writeOnceItemID(0),
	ground_equivalent(0),
	border_group(0),
	has_equivalent(false),
	wall_hate_me(false),
	name(""),
	description(""),
	weight(0.0f),
	attack(0),
	defense(0),
	armor(0),
	slot_position(SLOTP_HAND),
	weapon_type(WEAPON_NONE),
	charges(0),
	client_chargeable(false),
	extra_chargeable(false),
	ignoreLook(false),

	isHangable(false),
	hookEast(false),
	hookSouth(false),
	canReadText(false),
	canWriteText(false),
	replaceable(true),
	decays(false),
	stackable(false),
	moveable(true),
	alwaysOnBottom(false),
	pickupable(false),
	rotable(false),
	isBorder(false),
	isOptionalBorder(false),
	isWall(false),
	isBrushDoor(false),
	isOpen(false),
	isTable(false),
	isCarpet(false),

	floorChangeDown(false),
	floorChangeNorth(false),
	floorChangeSouth(false),
	floorChangeEast(false),
	floorChangeWest(false),
	floorChange(false),

	unpassable(false),
	blockPickupable(false),
	blockMissiles(false),
	blockPathfinder(false),
	hasElevation(false),

	alwaysOnTopOrder(0),
	rotateTo(0),
	way_speed(100),
	border_alignment(BORDER_NONE) {
	////
}

ItemType::~ItemType() {
	////
}

bool ItemType::isFloorChange() const {
	return floorChange || floorChangeDown || floorChangeNorth || floorChangeSouth || floorChangeEast || floorChangeWest;
}

ItemDatabase::ItemDatabase() :
	// Version information
	MajorVersion(0),
	MinorVersion(0),
	BuildNumber(0),

	// Count of GameSprite types
	item_count(0),
	effect_count(0),
	monster_count(0),
	distance_count(0),

	minclientID(0),
	maxclientID(0),

	max_item_id(0) {
	////
}

void ItemDatabase::clear() {
	items.clear();
}

bool ItemDatabase::loadFromOtbVer1(BinaryNode* itemNode, wxString& error, std::vector<std::string>& warnings) {
	return loadFromOtbGeneric(itemNode, OtbFileFormatVersion::V1, error, warnings);
}

bool ItemDatabase::loadFromOtbVer2(BinaryNode* itemNode, wxString& error, std::vector<std::string>& warnings) {
	return loadFromOtbGeneric(itemNode, OtbFileFormatVersion::V2, error, warnings);
}

bool ItemDatabase::loadFromOtbVer3(BinaryNode* itemNode, wxString& error, std::vector<std::string>& warnings) {
	return loadFromOtbGeneric(itemNode, OtbFileFormatVersion::V3, error, warnings);
}

bool ItemDatabase::loadFromOtbGeneric(BinaryNode* itemNode, OtbFileFormatVersion version, wxString& error, std::vector<std::string>& warnings) {
	uint8_t u8;

	for (; itemNode != nullptr; itemNode = itemNode->advance()) {
		if (!itemNode->getU8(u8)) {
			// Invalid!
			warnings.push_back("Invalid item type encountered...");
			continue;
		}

		if (ItemGroup_t(u8) == ITEM_GROUP_DEPRECATED) {
			continue;
		}

		auto owned_t = std::make_unique<ItemType>();
		ItemType* t = owned_t.get();
		t->group = ItemGroup_t(u8);

		static const auto group_handlers = [] {
			std::array<void (*)(ItemType&, OtbFileFormatVersion), ITEM_GROUP_LAST + 1> h {};
			h.fill([](ItemType&, OtbFileFormatVersion) { });
			h[ITEM_GROUP_DOOR] = [](ItemType& i, [[maybe_unused]] OtbFileFormatVersion v) { i.type = ITEM_TYPE_DOOR; };
			h[ITEM_GROUP_CONTAINER] = [](ItemType& i, [[maybe_unused]] OtbFileFormatVersion v) { i.type = ITEM_TYPE_CONTAINER; };
			h[ITEM_GROUP_RUNE] = [](ItemType& i, [[maybe_unused]] OtbFileFormatVersion v) { i.client_chargeable = true; };
			h[ITEM_GROUP_TELEPORT] = [](ItemType& i, [[maybe_unused]] OtbFileFormatVersion v) { i.type = ITEM_TYPE_TELEPORT; };
			h[ITEM_GROUP_MAGICFIELD] = [](ItemType& i, [[maybe_unused]] OtbFileFormatVersion v) { i.type = ITEM_TYPE_MAGICFIELD; };
			h[ITEM_GROUP_PODIUM] = [](ItemType& i, OtbFileFormatVersion v) {
				if (v >= OtbFileFormatVersion::V3) {
					i.type = ITEM_TYPE_PODIUM;
				}
			};
			return h;
		}();

		if (t->group <= ITEM_GROUP_LAST) {
			group_handlers[t->group](*t, version);
		} else {
			warnings.push_back("Unknown item group declaration");
		}

		uint32_t flags;
		if (itemNode->getU32(flags)) {
			t->unpassable = ((flags & FLAG_UNPASSABLE) == FLAG_UNPASSABLE);
			t->blockMissiles = ((flags & FLAG_BLOCK_MISSILES) == FLAG_BLOCK_MISSILES);
			t->blockPathfinder = ((flags & FLAG_BLOCK_PATHFINDER) == FLAG_BLOCK_PATHFINDER);
			t->hasElevation = ((flags & FLAG_HAS_ELEVATION) == FLAG_HAS_ELEVATION);
			t->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
			t->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
			t->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
			t->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
			t->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
			t->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
			t->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
			t->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
			t->floorChange = t->floorChangeDown || t->floorChangeNorth || t->floorChangeEast || t->floorChangeSouth || t->floorChangeWest;

			// The OTB `FLAG_ALWAYSONTOP` is mapped to the editor's `alwaysOnBottom` property.
			t->alwaysOnBottom = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
			t->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
			t->hookEast = ((flags & FLAG_HOOK_EAST) == FLAG_HOOK_EAST);
			t->hookSouth = ((flags & FLAG_HOOK_SOUTH) == FLAG_HOOK_SOUTH);
			t->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
			t->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
			t->canReadText = ((flags & FLAG_READABLE) == FLAG_READABLE);

			if (version >= OtbFileFormatVersion::V3) {
				t->client_chargeable = ((flags & FLAG_CLIENTCHARGES) == FLAG_CLIENTCHARGES);
				t->ignoreLook = ((flags & FLAG_IGNORE_LOOK) == FLAG_IGNORE_LOOK);
			}
		}

		using AttributeHandler = bool (*)(ItemType&, BinaryNode*, uint16_t, wxString&, std::vector<std::string>&);
		static const auto handlers = [] {
			std::array<AttributeHandler, 256> h {};
			h.fill([](ItemType&, BinaryNode* node, uint16_t len, wxString&, std::vector<std::string>&) {
				node->skip(len);
				return true;
			});

			h[ITEM_ATTR_SERVERID] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString& err, std::vector<std::string>& warnings) {
				if (len != sizeof(uint16_t)) {
					err = std::format("items.otb: Unexpected data length of server id block (Should be {} bytes)", sizeof(uint16_t));
					return false;
				}
				if (!node->getU16(it.id)) {
					warnings.push_back("Invalid item type property (serverID)");
					return true;
				}
				if (g_items.max_item_id < it.id) {
					g_items.max_item_id = it.id;
				}
				return true;
			};

			h[ITEM_ATTR_CLIENTID] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString& err, std::vector<std::string>& warnings) {
				if (len != sizeof(uint16_t)) {
					err = std::format("items.otb: Unexpected data length of client id block (Should be {} bytes)", sizeof(uint16_t));
					return false;
				}
				if (!node->getU16(it.clientID)) {
					warnings.push_back("Invalid item type property (clientID)");
				}
				it.sprite = static_cast<GameSprite*>(g_gui.gfx.getSprite(it.clientID));
				return true;
			};

			h[ITEM_ATTR_SPEED] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString& err, std::vector<std::string>& warnings) {
				if (len != sizeof(uint16_t)) {
					err = std::format("items.otb: Unexpected data length of speed block (Should be {} bytes)", sizeof(uint16_t));
					return false;
				}
				uint16_t speed = 0;
				if (!node->getU16(speed)) {
					warnings.push_back("Invalid item type property (speed)");
				}
				it.way_speed = speed;
				return true;
			};

			h[ITEM_ATTR_LIGHT2] = [](ItemType&, BinaryNode* node, uint16_t len, wxString&, std::vector<std::string>& warnings) {
				const size_t expected_len = 4; // sizeof(lightBlock2)
				if (len != expected_len) {
					warnings.push_back(std::format("items.otb: Unexpected data length of item light (2) block (Should be {} bytes)", expected_len));
					return true;
				}
				if (!node->skip(4)) {
					warnings.push_back("Invalid item type property (light2)");
				}
				return true;
			};

			h[ITEM_ATTR_TOPORDER] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString&, std::vector<std::string>& warnings) {
				if (len != sizeof(uint8_t)) {
					warnings.push_back("items.otb: Unexpected data length of item toporder block (Should be 1 byte)");
					return true;
				}
				uint8_t u8_top = 0;
				if (!node->getU8(u8_top)) {
					warnings.push_back("Invalid item type property (topOrder)");
				}
				it.alwaysOnTopOrder = u8_top;
				return true;
			};

			h[ITEM_ATTR_NAME] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString&, std::vector<std::string>& warnings) {
				if (len >= 128) {
					warnings.push_back("items.otb: Unexpected data length of item name block (Should be < 128 bytes)");
					return true;
				}
				uint8_t name[128] = { 0 };
				if (!node->getRAW(name, len)) {
					warnings.push_back("Invalid item type property (name)");
				}
				it.name = reinterpret_cast<const char*>(name);
				return true;
			};

			h[ITEM_ATTR_DESCR] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString&, std::vector<std::string>& warnings) {
				if (len >= 128) {
					warnings.push_back("items.otb: Unexpected data length of item descr block (Should be < 128 bytes)");
					return true;
				}
				uint8_t description[128] = { 0 };
				if (!node->getRAW(description, len)) {
					warnings.push_back("Invalid item type property (description)");
				}
				it.description = reinterpret_cast<const char*>(description);
				return true;
			};

			h[ITEM_ATTR_MAXITEMS] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString&, std::vector<std::string>& warnings) {
				if (len != sizeof(uint16_t)) {
					warnings.push_back("items.otb: Unexpected data length of item volume block (Should be 2 bytes)");
					return true;
				}
				if (!node->getU16(it.volume)) {
					warnings.push_back("Invalid item type property (volume)");
				}
				return true;
			};

			h[ITEM_ATTR_WEIGHT] = [](ItemType& it, BinaryNode* node, uint16_t len, wxString&, std::vector<std::string>& warnings) {
				if (len != sizeof(double)) {
					warnings.push_back("items.otb: Unexpected data length of item weight block (Should be 8 bytes)");
					return true;
				}
				double wi;
				if (!node->getRAW(reinterpret_cast<uint8_t*>(&wi), sizeof(double))) {
					warnings.push_back("Invalid item type property (weight)");
					return true;
				}
				it.weight = wi;
				return true;
			};

			h[ITEM_ATTR_ROTATETO] = [](ItemType& it, BinaryNode* node, uint16_t len, [[maybe_unused]] wxString&, std::vector<std::string>& warnings) {
				if (len != sizeof(uint16_t)) {
					warnings.push_back("items.otb: Unexpected data length of item rotateTo block (Should be 2 bytes)");
					return true;
				}
				uint16_t rotate = 0;
				if (!node->getU16(rotate)) {
					warnings.push_back("Invalid item type property (rotateTo)");
				}
				it.rotateTo = rotate;
				return true;
			};

			h[ITEM_ATTR_WRITEABLE3] = [](ItemType& it, BinaryNode* node, uint16_t len, [[maybe_unused]] wxString&, std::vector<std::string>& warnings) {
				const size_t expected_len = 4; // sizeof(writeableBlock3)
				if (len != expected_len) {
					warnings.push_back(std::format("items.otb: Unexpected data length of item writeable (3) block (Should be {} bytes)", expected_len));
					return true;
				}
				uint16_t readOnlyID = 0;
				uint16_t maxTextLen = 0;
				if (!node->getU16(readOnlyID)) {
					warnings.push_back("Invalid item type property (writeable3_readOnlyID)");
				}
				if (!node->getU16(maxTextLen)) {
					warnings.push_back("Invalid item type property (writeable3_maxTextLen)");
				}
				it.maxTextLen = maxTextLen;
				return true;
			};

			h[ITEM_ATTR_CLASSIFICATION] = [](ItemType& it, BinaryNode* node, uint16_t len, [[maybe_unused]] wxString&, std::vector<std::string>& warnings) {
				if (len != sizeof(uint8_t)) {
					warnings.push_back("items.otb: Unexpected data length of item classification block (Should be 1 byte)");
					return true;
				}
				uint8_t cls = 0;
				if (!node->getU8(cls)) {
					warnings.push_back("Invalid item type property (classification)");
				}
				it.classification = cls;
				return true;
			};

			return h;
		}();

		uint8_t attribute;
		while (itemNode->getU8(attribute)) {
			uint16_t datalen;
			if (!itemNode->getU16(datalen)) {
				warnings.push_back("Invalid item type property (premature end)");
				break;
			}

			if (!handlers[attribute](*t, itemNode, datalen, error, warnings)) {
				return false;
			}
		}

		if (t) {
			if (t->id < items.size() && items[t->id]) {
				warnings.push_back("items.otb: Duplicate items");
			}

			if (t->id >= items.size()) {
				items.resize(t->id + 1);
			}
			items[t->id] = std::move(owned_t);
		}
	}
	return true;
}

bool ItemDatabase::loadFromOtb(const FileName& datafile, wxString& error, std::vector<std::string>& warnings) {
	std::string filename = datafile.GetFullPath().ToStdString();
	DiskNodeFileReadHandle f(filename, StringVector(1, "OTBI"));

	if (!f.isOk()) {
		error = std::format("Couldn't open file \"{}\": {}", filename, f.getErrorMessage());
		return false;
	}

	BinaryNode* root = f.getRootNode();

#define safe_get(node, func, ...)               \
	do {                                        \
		if (!node->get##func(__VA_ARGS__)) {    \
			error = wxstr(f.getErrorMessage()); \
			return false;                       \
		}                                       \
	} while (false)

	// Read root flags
	root->skip(1); // Type info
	// uint32_t flags =

	root->skip(4); // Unused?

	uint8_t attr;
	safe_get(root, U8, attr);
	if (attr == ROOT_ATTR_VERSION) {
		uint16_t datalen;
		if (!root->getU16(datalen) || datalen != 4 + 4 + 4 + 1 * 128) {
			error = "items.otb: Size of version header is invalid, updated .otb version?";
			return false;
		}
		safe_get(root, U32, MajorVersion); // items otb format file version
		safe_get(root, U32, MinorVersion); // client version
		safe_get(root, U32, BuildNumber); // revision
		std::string csd;
		csd.resize(128);

		if (!root->getRAW((uint8_t*)csd.data(), 128)) { // CSDVersion ??
			error = wxstr(f.getErrorMessage());
			return false;
		}
	} else {
		error = "Expected ROOT_ATTR_VERSION as first node of items.otb!";
	}

	if (g_settings.getInteger(Config::CHECK_SIGNATURES)) {
		if (g_version.GetCurrentVersion().getOTBVersion().format_version != MajorVersion) {
			error = std::format("Unsupported items.otb version (version {})", MajorVersion);
			return false;
		}
	}

	using OtbLoaderFunc = bool (ItemDatabase::*)(BinaryNode*, wxString&, std::vector<std::string>&);
	static const std::map<uint32_t, OtbLoaderFunc> loaders = {
		{ 1, &ItemDatabase::loadFromOtbVer1 },
		{ 2, &ItemDatabase::loadFromOtbVer2 },
		{ 3, &ItemDatabase::loadFromOtbVer3 }
	};

	BinaryNode* itemNode = root->getChild();
	if (auto it_loader = loaders.find(MajorVersion); it_loader != loaders.end()) {
		return (this->*(it_loader->second))(itemNode, error, warnings);
	}

	error = std::format("items.otb: Unsupported version ({})", MajorVersion);
	return false;
}

void ItemDatabase::parseItemTypeAttribute(ItemType& it, std::string_view value) {
	static const std::unordered_map<std::string_view, void (*)(ItemType&)> parsers = {
		{ "depot", [](ItemType& i) { i.type = ITEM_TYPE_DEPOT; } },
		{ "mailbox", [](ItemType& i) { i.type = ITEM_TYPE_MAILBOX; } },
		{ "trashholder", [](ItemType& i) { i.type = ITEM_TYPE_TRASHHOLDER; } },
		{ "container", [](ItemType& i) { i.type = ITEM_TYPE_CONTAINER; } },
		{ "door", [](ItemType& i) { i.type = ITEM_TYPE_DOOR; } },
		{ "magicfield", [](ItemType& i) { i.group = ITEM_GROUP_MAGICFIELD; i.type = ITEM_TYPE_MAGICFIELD; } },
		{ "teleport", [](ItemType& i) { i.type = ITEM_TYPE_TELEPORT; } },
		{ "bed", [](ItemType& i) { i.type = ITEM_TYPE_BED; } },
		{ "key", [](ItemType& i) { i.type = ITEM_TYPE_KEY; } },
		{ "podium", [](ItemType& i) { i.type = ITEM_TYPE_PODIUM; } },
	};

	if (auto it_parser = parsers.find(value); it_parser != parsers.end()) {
		it_parser->second(it);
	}
}

void ItemDatabase::parseSlotTypeAttribute(ItemType& it, std::string_view value) {
	static const std::unordered_map<std::string_view, void (*)(ItemType&)> parsers = {
		{ "head", [](ItemType& i) { i.slot_position |= SLOTP_HEAD; } },
		{ "body", [](ItemType& i) { i.slot_position |= SLOTP_ARMOR; } },
		{ "legs", [](ItemType& i) { i.slot_position |= SLOTP_LEGS; } },
		{ "feet", [](ItemType& i) { i.slot_position |= SLOTP_FEET; } },
		{ "backpack", [](ItemType& i) { i.slot_position |= SLOTP_BACKPACK; } },
		{ "two-handed", [](ItemType& i) { i.slot_position |= SLOTP_TWO_HAND; } },
		{ "right-hand", [](ItemType& i) { i.slot_position &= ~SLOTP_LEFT; } },
		{ "left-hand", [](ItemType& i) { i.slot_position &= ~SLOTP_RIGHT; } },
		{ "necklace", [](ItemType& i) { i.slot_position |= SLOTP_NECKLACE; } },
		{ "ring", [](ItemType& i) { i.slot_position |= SLOTP_RING; } },
		{ "ammo", [](ItemType& i) { i.slot_position |= SLOTP_AMMO; } },
		{ "hand", [](ItemType& i) { i.slot_position |= SLOTP_HAND; } },
	};

	if (auto it_parser = parsers.find(value); it_parser != parsers.end()) {
		it_parser->second(it);
	}
}

void ItemDatabase::parseWeaponTypeAttribute(ItemType& it, std::string_view value) {
	static const std::unordered_map<std::string_view, void (*)(ItemType&)> parsers = {
		{ "sword", [](ItemType& i) { i.weapon_type = WEAPON_SWORD; } },
		{ "club", [](ItemType& i) { i.weapon_type = WEAPON_CLUB; } },
		{ "axe", [](ItemType& i) { i.weapon_type = WEAPON_AXE; } },
		{ "shield", [](ItemType& i) { i.weapon_type = WEAPON_SHIELD; } },
		{ "distance", [](ItemType& i) { i.weapon_type = WEAPON_DISTANCE; } },
		{ "wand", [](ItemType& i) { i.weapon_type = WEAPON_WAND; } },
		{ "ammunition", [](ItemType& i) { i.weapon_type = WEAPON_AMMO; } },
	};

	if (auto it_parser = parsers.find(value); it_parser != parsers.end()) {
		it_parser->second(it);
	}
}

void ItemDatabase::parseFloorChangeAttribute(ItemType& it, std::string_view value) {
	static const std::unordered_map<std::string_view, void (*)(ItemType&)> parsers = {
		{ "down", [](ItemType& i) { i.floorChangeDown = true; i.floorChange = true; } },
		{ "north", [](ItemType& i) { i.floorChangeNorth = true; i.floorChange = true; } },
		{ "south", [](ItemType& i) { i.floorChangeSouth = true; i.floorChange = true; } },
		{ "west", [](ItemType& i) { i.floorChangeWest = true; i.floorChange = true; } },
		{ "east", [](ItemType& i) { i.floorChangeEast = true; i.floorChange = true; } },
		{ "northex", [](ItemType& i) { i.floorChange = true; } },
		{ "southex", [](ItemType& i) { i.floorChange = true; } },
		{ "westex", [](ItemType& i) { i.floorChange = true; } },
		{ "eastex", [](ItemType& i) { i.floorChange = true; } },
		{ "southalt", [](ItemType& i) { i.floorChange = true; } },
		{ "eastalt", [](ItemType& i) { i.floorChange = true; } },
	};

	if (auto it_parser = parsers.find(value); it_parser != parsers.end()) {
		it_parser->second(it);
	}
}

bool ItemDatabase::loadItemFromGameXml(pugi::xml_node itemNode, int id) {
	ClientVersionID clientVersion = g_version.GetCurrentVersionID();
	if (clientVersion < CLIENT_VERSION_980 && id > 20000 && id < 20100) {
		itemNode = itemNode.next_sibling();
		return true;
	} else if (id > 30000 && id < 30100) {
		itemNode = itemNode.next_sibling();
		return true;
	}

	ItemType& it = getItemType(id);

	it.name = itemNode.attribute("name").as_string();
	it.editorsuffix = itemNode.attribute("editorsuffix").as_string();

	using ParserFunc = void (*)(ItemType&, pugi::xml_node);
	static const std::unordered_map<std::string_view, ParserFunc> parsers = {
		{ "type", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 std::string val = attr.as_string();
				 to_lower_str(val);
				 ItemDatabase::parseItemTypeAttribute(it, val);
			 }
		 } },
		{ "name", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.name = attr.as_string();
			 }
		 } },
		{ "description", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.description = attr.as_string();
			 }
		 } },
		{ "speed", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.way_speed = attr.as_uint();
			 }
		 } },
		{ "weight", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.weight = attr.as_int() / 100.f;
			 }
		 } },
		{ "armor", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.armor = attr.as_int();
			 }
		 } },
		{ "defense", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.defense = attr.as_int();
			 }
		 } },
		{ "slottype", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 std::string val = attr.as_string();
				 to_lower_str(val);
				 ItemDatabase::parseSlotTypeAttribute(it, val);
			 }
		 } },
		{ "weapontype", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 std::string val = attr.as_string();
				 to_lower_str(val);
				 ItemDatabase::parseWeaponTypeAttribute(it, val);
			 }
		 } },
		{ "rotateto", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.rotateTo = attr.as_ushort();
			 }
		 } },
		{ "containersize", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.volume = attr.as_ushort();
			 }
		 } },
		{ "readable", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.canReadText = attr.as_bool();
			 }
		 } },
		{ "writeable", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.canWriteText = it.canReadText = attr.as_bool();
			 }
		 } },
		{ "decayto", [](ItemType& it, [[maybe_unused]] pugi::xml_node node) {
			 it.decays = true;
		 } },
		{ "maxtextlen", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.maxTextLen = attr.as_ushort();
				 it.canReadText = it.maxTextLen > 0;
			 }
		 } },
		{ "maxtextlength", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.maxTextLen = attr.as_ushort();
				 it.canReadText = it.maxTextLen > 0;
			 }
		 } },
		{ "allowdistread", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.allowDistRead = attr.as_bool();
			 }
		 } },
		{ "charges", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 it.charges = attr.as_uint();
				 it.extra_chargeable = true;
			 }
		 } },
		{ "floorchange", [](ItemType& it, pugi::xml_node node) {
			 if (auto attr = node.attribute("value")) {
				 std::string val = attr.as_string();
				 to_lower_str(val);
				 ItemDatabase::parseFloorChangeAttribute(it, val);
			 }
		 } }
	};

	for (auto attributeNode : itemNode.children()) {
		if (auto attr = attributeNode.attribute("key")) {
			std::string key = attr.as_string();
			to_lower_str(key);
			if (auto it_parser = parsers.find(key); it_parser != parsers.end()) {
				it_parser->second(it, attributeNode);
			}
		}
	}
	return true;
}

bool ItemDatabase::loadFromGameXml(const FileName& identifier, wxString& error, std::vector<std::string>& warnings) {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(identifier.GetFullPath().mb_str());
	if (!result) {
		error = "Could not load items.xml (Syntax error?)";
		return false;
	}

	pugi::xml_node node = doc.child("items");
	if (!node) {
		error = "items.xml, invalid root node.";
		return false;
	}

	for (pugi::xml_node itemNode = node.first_child(); itemNode; itemNode = itemNode.next_sibling()) {
		if (as_lower_str(itemNode.name()) != "item") {
			continue;
		}

		uint16_t fromId = 0;
		uint16_t toId = 0;
		if (const pugi::xml_attribute attribute = itemNode.attribute("id")) {
			fromId = toId = attribute.as_ushort();
		} else {
			fromId = itemNode.attribute("fromid").as_ushort();
			toId = itemNode.attribute("toid").as_ushort();
		}

		if (fromId == 0 || toId == 0) {
			error = "Could not read item id from item node.";
			return false;
		}

		for (uint16_t id = fromId; id <= toId; ++id) {
			if (!loadItemFromGameXml(itemNode, id)) {
				return false;
			}
		}
	}
	return true;
}

bool ItemDatabase::loadMetaItem(pugi::xml_node node) {
	if (const pugi::xml_attribute attribute = node.attribute("id")) {
		const uint16_t id = attribute.as_ushort();
		if (id == 0 || (id < items.size() && items[id])) {
			return false;
		}

		if (id >= items.size()) {
			items.resize(id + 1);
		}
		items[id] = std::make_unique<ItemType>();
		items[id]->is_metaitem = true;
		items[id]->id = id;
		return true;
	}
	return false;
}

ItemType& ItemDatabase::getItemType(int id) {
	if (static_cast<size_t>(id) < items.size()) {
		if (auto& it = items[id]) {
			return *it;
		}
	}
	static ItemType dummyItemType; // use this for invalid ids
	return dummyItemType;
}

bool ItemDatabase::typeExists(int id) const {
	return static_cast<size_t>(id) < items.size() && items[id] != nullptr;
}
