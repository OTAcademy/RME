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

#include "game/item_attributes.h"
#include "io/filehandle.h"
#include <cstring>

ItemAttributes::ItemAttributes() :
	attributes(nullptr) {
	////
}

ItemAttributes::ItemAttributes(const ItemAttributes& o) {
	if (o.attributes) {
		attributes = newd ItemAttributeMap(*o.attributes);
	}
}

ItemAttributes::~ItemAttributes() {
	clearAllAttributes();
}

void ItemAttributes::createAttributes() {
	if (!attributes) {
		attributes = newd ItemAttributeMap;
	}
}

void ItemAttributes::clearAllAttributes() {
	if (attributes) {
		delete attributes;
	}
	attributes = nullptr;
}

ItemAttributeMap ItemAttributes::getAttributes() const {
	if (attributes) {
		return *attributes;
	}
	return ItemAttributeMap();
}

void ItemAttributes::setAttribute(const std::string& key, const ItemAttribute& value) {
	createAttributes();
	(*attributes)[key] = value;
}

void ItemAttributes::setAttribute(const std::string& key, const std::string& value) {
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, int32_t value) {
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, double value) {
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, bool value) {
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::eraseAttribute(const std::string& key) {
	if (!attributes) {
		return;
	}

	ItemAttributeMap::iterator iter = attributes->find(key);

	if (iter != attributes->end()) {
		attributes->erase(iter);
	}
}

const std::string* ItemAttributes::getStringAttribute(const std::string& key) const {
	if (!attributes) {
		return nullptr;
	}

	ItemAttributeMap::iterator iter = attributes->find(key);
	if (iter != attributes->end()) {
		return iter->second.getString();
	}
	return nullptr;
}

const int32_t* ItemAttributes::getIntegerAttribute(const std::string& key) const {
	if (!attributes) {
		return nullptr;
	}

	ItemAttributeMap::iterator iter = attributes->find(key);
	if (iter != attributes->end()) {
		return iter->second.getInteger();
	}
	return nullptr;
}

const double* ItemAttributes::getFloatAttribute(const std::string& key) const {
	if (!attributes) {
		return nullptr;
	}

	ItemAttributeMap::iterator iter = attributes->find(key);
	if (iter != attributes->end()) {
		return iter->second.getFloat();
	}
	return nullptr;
}

const bool* ItemAttributes::getBooleanAttribute(const std::string& key) const {
	if (!attributes) {
		return nullptr;
	}

	ItemAttributeMap::iterator iter = attributes->find(key);
	if (iter != attributes->end()) {
		return iter->second.getBoolean();
	}
	return nullptr;
}

bool ItemAttributes::hasStringAttribute(const std::string& key) const {
	return getStringAttribute(key) != nullptr;
}

bool ItemAttributes::hasIntegerAttribute(const std::string& key) const {
	return getIntegerAttribute(key) != nullptr;
}

bool ItemAttributes::hasFloatAttribute(const std::string& key) const {
	return getFloatAttribute(key) != nullptr;
}

bool ItemAttributes::hasBooleanAttribute(const std::string& key) const {
	return getBooleanAttribute(key) != nullptr;
}

// Attribute type
// Can hold either int, bool or std::string
// Without using newd to allocate them

ItemAttribute::ItemAttribute() :
	type(ItemAttribute::NONE) {
	////
}

ItemAttribute::ItemAttribute(const std::string& str) :
	type(ItemAttribute::STRING), m_value(str) {
}

ItemAttribute::ItemAttribute(int32_t i) :
	type(ItemAttribute::INTEGER), m_value(i) {
}

ItemAttribute::ItemAttribute(double f) :
	type(ItemAttribute::DOUBLE), m_value(f) {
}

ItemAttribute::ItemAttribute(bool b) :
	type(ItemAttribute::BOOLEAN), m_value(b) {
}

ItemAttribute::ItemAttribute(const ItemAttribute& o) :
	type(o.type), m_value(o.m_value) {
}

ItemAttribute& ItemAttribute::operator=(const ItemAttribute& o) {
	if (&o == this) {
		return *this;
	}
	type = o.type;
	m_value = o.m_value;
	return *this;
}

ItemAttribute::~ItemAttribute() {
}

void ItemAttribute::clear() {
	type = NONE;
	m_value = std::monostate{};
}

void ItemAttribute::set(const std::string& str) {
	type = STRING;
	m_value = str;
}

void ItemAttribute::set(int32_t i) {
	type = INTEGER;
	m_value = i;
}

void ItemAttribute::set(double y) {
	type = DOUBLE;
	m_value = y;
}

void ItemAttribute::set(bool b) {
	type = BOOLEAN;
	m_value = b;
}

const std::string* ItemAttribute::getString() const {
	return std::get_if<std::string>(&m_value);
}

const int32_t* ItemAttribute::getInteger() const {
	return std::get_if<int32_t>(&m_value);
}

const double* ItemAttribute::getFloat() const {
	return std::get_if<double>(&m_value);
}

const bool* ItemAttribute::getBoolean() const {
	return std::get_if<bool>(&m_value);
}

bool ItemAttributes::unserializeAttributeMap(const IOMap& maphandle, BinaryNode* stream) {
	uint16_t n;
	if (stream->getU16(n)) {
		createAttributes();

		std::string key;
		ItemAttribute attrib;

		while (n--) {
			if (!stream->getString(key)) {
				return false;
			}
			if (!attrib.unserialize(maphandle, stream)) {
				return false;
			}
			(*attributes)[key] = attrib;
		}
	}
	return true;
}

void ItemAttributes::serializeAttributeMap(const IOMap& maphandle, NodeFileWriteHandle& f) const {
	// Maximum of 65535 attributes per item
	f.addU16(std::min((size_t)0xFFFF, attributes->size()));

	ItemAttributeMap::const_iterator attribute = attributes->begin();
	int i = 0;
	while (attribute != attributes->end() && i <= 0xFFFF) {
		const std::string& key = attribute->first;
		if (key.size() > 0xFFFF) {
			f.addString(key.substr(0, 65535));
		} else {
			f.addString(key);
		}

		attribute->second.serialize(maphandle, f);
		++attribute, ++i;
	}
}

bool ItemAttribute::unserialize(const IOMap& maphandle, BinaryNode* stream) {
	// Read type
	uint8_t rtype;
	stream->getU8(rtype);

	// Read contents
	switch (rtype) {
		case STRING: {
			std::string str;
			if (!stream->getLongString(str)) {
				return false;
			}
			set(str);
			break;
		}
		case INTEGER: {
			uint32_t u32;
			if (!stream->getU32(u32)) {
				return false;
			}
			// Safe conversion from u32 to int32_t (2's complement)
			set(static_cast<int32_t>(u32));
			break;
		}
		case FLOAT: {
			uint32_t u32;
			if (!stream->getU32(u32)) {
				return false;
			}
			// Safe type punning
			float f;
			std::memcpy(&f, &u32, sizeof(float));
			set(static_cast<double>(f));
			break;
		}
		case DOUBLE: {
			uint64_t u64;
			if (!stream->getU64(u64)) {
				return false;
			}
			// Safe type punning
			double d;
			std::memcpy(&d, &u64, sizeof(double));
			set(d);
			break;
		}
		case BOOLEAN: {
			uint8_t b;
			if (!stream->getU8(b)) {
				return false;
			}
			set(b != 0);
		}
		default:
			break;
	}
	return true;
}

void ItemAttribute::serialize(const IOMap& maphandle, NodeFileWriteHandle& f) const {
	// Write type
	f.addU8((uint8_t)(type));

	// Write contents
	switch (type) {
		case STRING:
			f.addLongString(*getString());
			break;
		case INTEGER:
			f.addU32(static_cast<uint32_t>(*getInteger()));
			break;
		case DOUBLE: {
			double d = *getFloat();
			uint64_t u64;
			std::memcpy(&u64, &d, sizeof(double));
			f.addU64(u64);
			break;
		}
		case BOOLEAN:
			f.addU8(static_cast<uint8_t>(*getBoolean()));
		default:
			break;
	}
}
