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

ItemDatabase::~ItemDatabase() {
	clear();
}

void ItemDatabase::clear() {
	for (uint32_t i = 0; i < items.size(); i++) {
		delete items[i];
		items.set(i, nullptr);
	}
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

		ItemType* t = newd ItemType();
		t->group = ItemGroup_t(u8);

		switch (t->group) {
			case ITEM_GROUP_NONE:
			case ITEM_GROUP_GROUND:
			case ITEM_GROUP_SPLASH:
			case ITEM_GROUP_FLUID:
			case ITEM_GROUP_WEAPON:
			case ITEM_GROUP_AMMUNITION:
			case ITEM_GROUP_ARMOR:
			case ITEM_GROUP_WRITEABLE:
			case ITEM_GROUP_KEY:
				break;
			case ITEM_GROUP_DOOR:
				t->type = ITEM_TYPE_DOOR;
				break;
			case ITEM_GROUP_CONTAINER:
				t->type = ITEM_TYPE_CONTAINER;
				break;
			case ITEM_GROUP_RUNE:
				t->client_chargeable = true;
				break;
			case ITEM_GROUP_TELEPORT:
				t->type = ITEM_TYPE_TELEPORT;
				break;
			case ITEM_GROUP_MAGICFIELD:
				t->type = ITEM_TYPE_MAGICFIELD;
				break;
			case ITEM_GROUP_PODIUM:
				t->type = ITEM_TYPE_PODIUM;
				break;
			default:
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

		uint8_t attribute;
		while (itemNode->getU8(attribute)) {
			uint16_t datalen;
			if (!itemNode->getU16(datalen)) {
				warnings.push_back("Invalid item type property");
				break;
			}

			switch (attribute) {
				case ITEM_ATTR_SERVERID: {
					if (datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of server id block (Should be 2 bytes)";
						return false;
					}
					if (!itemNode->getU16(t->id)) {
						warnings.push_back("Invalid item type property (2)");
					}

					if (max_item_id < t->id) {
						max_item_id = t->id;
					}
					break;
				}

				case ITEM_ATTR_CLIENTID: {
					if (datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of client id block (Should be 2 bytes)";
						return false;
					}

					if (!itemNode->getU16(t->clientID)) {
						warnings.push_back("Invalid item type property (2)");
					}

					t->sprite = static_cast<GameSprite*>(g_gui.gfx.getSprite(t->clientID));
					break;
				}

				case ITEM_ATTR_SPEED: {
					if (datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of speed block (Should be 2 bytes)";
						return false;
					}

					uint16_t speed = 0;
					if (!itemNode->getU16(speed)) {
						warnings.push_back("Invalid item type property (3)");
					}
					t->way_speed = speed;
					break;
				}

				case ITEM_ATTR_LIGHT2: {
					if (datalen != sizeof(lightBlock2)) {
						warnings.push_back(("items.otb: Unexpected data length of item light (2) block (Should be " + i2ws(sizeof(lightBlock2)) + " bytes)").ToStdString());
						break;
					}

					if (!itemNode->skip(4)) { // Just skip two bytes, we don't need light
						warnings.push_back("Invalid item type property (4)");
					}
					break;
				}

				case ITEM_ATTR_TOPORDER: {
					if (datalen != sizeof(uint8_t)) {
						warnings.push_back("items.otb: Unexpected data length of item toporder block (Should be 1 byte)");
						break;
					}

					uint8_t u8_top = 0;
					if (!itemNode->getU8(u8_top)) {
						warnings.push_back("Invalid item type property (5)");
					}

					t->alwaysOnTopOrder = u8_top;
					break;
				}

				case ITEM_ATTR_NAME: {
					if (datalen >= 128) {
						warnings.push_back("items.otb: Unexpected data length of item name block (Should be 128 bytes)");
						break;
					}

					uint8_t name[128];
					memset(&name, 0, 128);

					if (!itemNode->getRAW(name, datalen)) {
						warnings.push_back("Invalid item type property (6)");
						break;
					}
					t->name = (char*)name;
					break;
				}

				case ITEM_ATTR_DESCR: {
					if (datalen >= 128) {
						warnings.push_back("items.otb: Unexpected data length of item descr block (Should be 128 bytes)");
						break;
					}

					uint8_t description[128];
					memset(&description, 0, 128);

					if (!itemNode->getRAW(description, datalen)) {
						warnings.push_back("Invalid item type property (7)");
						break;
					}

					t->description = (char*)description;
					break;
				}

				case ITEM_ATTR_MAXITEMS: {
					if (datalen != sizeof(unsigned short)) {
						warnings.push_back("items.otb: Unexpected data length of item volume block (Should be 2 bytes)");
						break;
					}

					if (!itemNode->getU16(t->volume)) {
						warnings.push_back("Invalid item type property (8)");
					}
					break;
				}

				case ITEM_ATTR_WEIGHT: {
					if (datalen != sizeof(double)) {
						warnings.push_back("items.otb: Unexpected data length of item weight block (Should be 8 bytes)");
						break;
					}
					uint8_t w[sizeof(double)];
					if (!itemNode->getRAW(w, sizeof(double))) {
						warnings.push_back("Invalid item type property (7)");
						break;
					}

					double wi = *reinterpret_cast<double*>(&w);
					t->weight = wi;
					break;
				}

				case ITEM_ATTR_ROTATETO: {
					if (datalen != sizeof(unsigned short)) {
						warnings.push_back("items.otb: Unexpected data length of item rotateTo block (Should be 2 bytes)");
						break;
					}

					uint16_t rotate;
					if (!itemNode->getU16(rotate)) {
						warnings.push_back("Invalid item type property (8)");
						break;
					}

					t->rotateTo = rotate;
					break;
				}

				case ITEM_ATTR_WRITEABLE3: {
					if (datalen != sizeof(writeableBlock3)) {
						warnings.push_back("items.otb: Unexpected data length of item toporder block (Should be 1 byte)");
						break;
					}

					uint16_t readOnlyID = 0;
					uint16_t maxTextLen = 0;

					if (!itemNode->getU16(readOnlyID)) {
						warnings.push_back("Invalid item type property (9)");
						break;
					}

					if (!itemNode->getU16(maxTextLen)) {
						warnings.push_back("Invalid item type property (10)");
						break;
					}

					// t->readOnlyId = wb3->readOnlyId;
					t->maxTextLen = maxTextLen;
					break;
				}

				case ITEM_ATTR_CLASSIFICATION: {
					if (datalen != sizeof(uint8_t)) {
						warnings.push_back("items.otb: Unexpected data length of item classification block (Should be 1 byte)");
						break;
					}

					uint8_t cls = 0;
					if (!itemNode->getU8(cls)) {
						warnings.push_back("Invalid item type property (5)");
					}

					t->classification = cls;
					break;
				}

				default: {
					// skip unknown attributes
					itemNode->skip(datalen);
					// warnings.push_back("items.otb: Skipped unknown attribute");
					break;
				}
			}
		}

		if (t) {
			if (items[t->id]) {
				warnings.push_back("items.otb: Duplicate items");
				delete items[t->id];
			}
			items.set(t->id, t);
		}
	}
	return true;
}

bool ItemDatabase::loadFromOtb(const FileName& datafile, wxString& error, std::vector<std::string>& warnings) {
	std::string filename = nstr((datafile.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + datafile.GetFullName()));
	DiskNodeFileReadHandle f(filename, StringVector(1, "OTBI"));

	if (!f.isOk()) {
		error = "Couldn't open file \"" + wxstr(filename) + "\":" + wxstr(f.getErrorMessage());
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
			error = "Unsupported items.otb version (version " + i2ws(MajorVersion) + ")";
			return false;
		}
	}

	BinaryNode* itemNode = root->getChild();
	switch (MajorVersion) {
		case 1:
			return loadFromOtbVer1(itemNode, error, warnings);
		case 2:
			return loadFromOtbVer2(itemNode, error, warnings);
		case 3:
			return loadFromOtbVer3(itemNode, error, warnings);
	}
	return true;
}

void ItemDatabase::parseItemTypeAttribute(ItemType& it, std::string_view value) {
	static const std::map<std::string_view, void (*)(ItemType&)> parsers = {
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
	static const std::map<std::string_view, void (*)(ItemType&)> parsers = {
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
	static const std::map<std::string_view, void (*)(ItemType&)> parsers = {
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
	static const std::map<std::string_view, void (*)(ItemType&)> parsers = {
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

	pugi::xml_attribute attribute;
	for (pugi::xml_node itemAttributesNode = itemNode.first_child(); itemAttributesNode; itemAttributesNode = itemAttributesNode.next_sibling()) {
		if (!(attribute = itemAttributesNode.attribute("key"))) {
			continue;
		}

		std::string key = attribute.as_string();
		to_lower_str(key);
		if (key == "type") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				std::string typeValue = attribute.as_string();
				to_lower_str(typeValue);
				parseItemTypeAttribute(it, typeValue);
			}
		} else if (key == "name") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.name = attribute.as_string();
			}
		} else if (key == "description") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.description = attribute.as_string();
			}
		} else if (key == "runespellName") {
			/*if((attribute = itemAttributesNode.attribute("value"))) {
				it.runeSpellName = attribute.as_string();
			}*/
		} else if (key == "speed") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.way_speed = attribute.as_uint();
			}
		} else if (key == "weight") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.weight = attribute.as_int() / 100.f;
			}
		} else if (key == "armor") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.armor = attribute.as_int();
			}
		} else if (key == "defense") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.defense = attribute.as_int();
			}
		} else if (key == "slottype") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				std::string typeValue = attribute.as_string();
				to_lower_str(typeValue);
				parseSlotTypeAttribute(it, typeValue);
			}
		} else if (key == "weapontype") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				std::string typeValue = attribute.as_string();
				to_lower_str(typeValue);
				parseWeaponTypeAttribute(it, typeValue);
			}
		} else if (key == "rotateto") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.rotateTo = attribute.as_ushort();
			}
		} else if (key == "containersize") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.volume = attribute.as_ushort();
			}
		} else if (key == "readable") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.canReadText = attribute.as_bool();
			}
		} else if (key == "writeable") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.canWriteText = it.canReadText = attribute.as_bool();
			}
		} else if (key == "decayto") {
			it.decays = true;
		} else if (key == "maxtextlen" || key == "maxtextlength") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.maxTextLen = attribute.as_ushort();
				it.canReadText = it.maxTextLen > 0;
			}
		} else if (key == "writeonceitemid") {
			/*if((attribute = itemAttributesNode.attribute("value"))) {
				it.writeOnceItemId = pugi::cast<int32_t>(attribute.value());
			}*/
		} else if (key == "allowdistread") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.allowDistRead = attribute.as_bool();
			}
		} else if (key == "charges") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				it.charges = attribute.as_uint();
				it.extra_chargeable = true;
			}
		} else if (key == "floorchange") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				std::string value = attribute.as_string();
				to_lower_str(value);
				parseFloorChangeAttribute(it, value);
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
		if (id == 0 || items[id]) {
			return false;
		}
		items.set(id, newd ItemType());
		items[id]->is_metaitem = true;
		items[id]->id = id;
		return true;
	}
	return false;
}

ItemType& ItemDatabase::getItemType(int id) {
	ItemType* it = items[id];
	if (it) {
		return *it;
	} else {
		static ItemType dummyItemType; // use this for invalid ids
		return dummyItemType;
	}
}

bool ItemDatabase::typeExists(int id) const {
	ItemType* it = items[id];
	return it != nullptr;
}
