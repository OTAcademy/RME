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

#ifndef RME_LUA_SCRIPT_H
#define RME_LUA_SCRIPT_H

#include <string>

// Represents a single Lua script (file or directory with manifest.json)
class LuaScript {
public:
	// Constructor for single .lua file
	LuaScript(const std::string& filepath);

	// Constructor for directory with manifest.json
	LuaScript(const std::string& directory, bool isDirectory);

	~LuaScript() = default;

	// Script info
	const std::string& getFilePath() const {
		return filepath;
	}
	const std::string& getDirectory() const {
		return directory;
	}
	const std::string& getFileName() const {
		return filename;
	}
	const std::string& getDisplayName() const {
		return displayName;
	}
	const std::string& getDescription() const {
		return description;
	}
	const std::string& getAuthor() const {
		return author;
	}
	const std::string& getVersion() const {
		return version;
	}
	const std::string& getShortcut() const {
		return shortcut;
	}
	bool shouldAutoRun() const {
		return autorun;
	}

	// Is this a directory-based script with manifest?
	bool isPackage() const {
		return isPackageScript;
	}

	// State
	bool isEnabled() const {
		return enabled;
	}
	void setEnabled(bool value) {
		enabled = value;
	}

	// Parse metadata from comments or manifest.json
	void parseMetadata();

private:
	void parseMetadataFromComments();
	void parseMetadataFromManifest();

	std::string filepath; // Full path to the main .lua file
	std::string directory; // Directory containing the script (for packages)
	std::string filename; // Just the filename
	std::string displayName; // Display name (from metadata or filename)
	std::string description; // Description
	std::string author; // Author
	std::string version; // Version
	std::string shortcut; // Keyboard shortcut
	bool enabled;
	bool autorun;
	bool isPackageScript; // True if loaded from directory with manifest.json
};

#endif // RME_LUA_SCRIPT_H
