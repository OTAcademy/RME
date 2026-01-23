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

#ifndef RME_LUA_API_IMAGE_H
#define RME_LUA_API_IMAGE_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <wx/image.h>
#include <wx/bitmap.h>

namespace LuaAPI {

	// LuaImage class for Lua scripting
	// Supports loading external images and game item sprites
	class LuaImage {
	public:
		// Default constructor (empty image)
		LuaImage();

		// Constructor from file path
		explicit LuaImage(const std::string& path);

		// Constructor from item sprite ID
		LuaImage(int spriteId, bool isItemSprite);

		// Copy constructor
		LuaImage(const LuaImage& other);

		// Copy assignment operator
		LuaImage& operator=(const LuaImage& other);

		~LuaImage();

		// Static factory methods for Lua
		static LuaImage loadFromFile(const std::string& path);
		static LuaImage loadFromItemSprite(int itemId);
		static LuaImage loadFromSprite(int spriteId);

		// Properties
		int getWidth() const;
		int getHeight() const;
		bool isValid() const;
		std::string getPath() const {
			return filePath;
		}
		int getSpriteId() const {
			return spriteId;
		}
		bool isSpriteSource() const {
			return spriteSource;
		}

		// Operations
		LuaImage resize(int width, int height, bool smooth = true) const;
		LuaImage scale(double factor, bool smooth = true) const;

		// Get the underlying wxImage (for internal use)
		const wxImage& getWxImage() const {
			return image;
		}
		wxBitmap getBitmap() const;
		wxBitmap getBitmap(int width, int height, bool smooth = true) const;

		// Equality comparison
		bool operator==(const LuaImage& other) const;

	private:
		wxImage image;
		std::string filePath;
		int spriteId = 0;
		bool spriteSource = false;

		// Load image from game sprite
		void loadFromSpriteId(int id);
	};

	// Register the Image usertype with Lua
	void registerImage(sol::state& lua);

} // namespace LuaAPI

#endif // RME_LUA_API_IMAGE_H
