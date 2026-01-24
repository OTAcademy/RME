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

#include "main.h"
#include "lua_script.h"

#include <fstream>
#include <sstream>
#include <algorithm>

// Constructor for single .lua file
LuaScript::LuaScript(const std::string& filepath) :
	filepath(filepath),
	enabled(true),
	autorun(false),
	isPackageScript(false) {

	// Extract filename from path
	size_t lastSlash = filepath.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		filename = filepath.substr(lastSlash + 1);
		directory = filepath.substr(0, lastSlash);
	} else {
		filename = filepath;
		directory = ".";
	}

	// Default display name is filename without extension
	size_t lastDot = filename.find_last_of('.');
	if (lastDot != std::string::npos) {
		displayName = filename.substr(0, lastDot);
	} else {
		displayName = filename;
	}

	// Replace underscores with spaces for display
	std::replace(displayName.begin(), displayName.end(), '_', ' ');

	// Parse metadata from script comments
	parseMetadata();
}

// Constructor for directory with manifest.lua
LuaScript::LuaScript(const std::string& dir, bool isDirectory) :
	directory(dir),
	enabled(true),
	autorun(false),
	isPackageScript(true) {

	// Extract folder name for default display name
	size_t lastSlash = dir.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		displayName = dir.substr(lastSlash + 1);
	} else {
		displayName = dir;
	}

	// Replace underscores with spaces for display
	std::replace(displayName.begin(), displayName.end(), '_', ' ');

	// Parse metadata from manifest.lua
	parseMetadata();
}

void LuaScript::parseMetadata() {
	if (isPackageScript) {
		parseMetadataFromManifest();
	} else {
		parseMetadataFromComments();
	}
}

void LuaScript::parseMetadataFromManifest() {
	// Parse manifest.lua which returns a table with metadata
	// Format:
	// return {
	//     name = "Script Name",
	//     description = "Description",
	//     author = "Author",
	//     version = "1.0.0",
	//     main = "main_script",
	//     shortcut = "Ctrl+Shift+H"
	// }

	std::string manifestPath = directory + "/manifest.lua";
	std::ifstream file(manifestPath);
	if (!file.is_open()) {
		// Try default main.lua
		filepath = directory + "/main.lua";
		filename = "main.lua";
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();

	// Simple Lua table parser for manifest
	auto getValue = [&content](const std::string& key) -> std::string {
		// Look for: key = "value" or key = 'value'
		std::string pattern1 = key + " = \"";
		std::string pattern2 = key + " = '";
		std::string pattern3 = key + "=\"";
		std::string pattern4 = key + "='";

		size_t pos = content.find(pattern1);
		char quote = '"';
		if (pos == std::string::npos) {
			pos = content.find(pattern2);
			quote = '\'';
		}
		if (pos == std::string::npos) {
			pos = content.find(pattern3);
			quote = '"';
		}
		if (pos == std::string::npos) {
			pos = content.find(pattern4);
			quote = '\'';
		}

		if (pos == std::string::npos) {
			return "";
		}

		size_t valueStart = content.find(quote, pos) + 1;
		size_t valueEnd = content.find(quote, valueStart);
		if (valueEnd == std::string::npos) {
			return "";
		}

		return content.substr(valueStart, valueEnd - valueStart);
	};

	std::string val;

	val = getValue("name");
	if (!val.empty()) {
		displayName = val;
	}

	val = getValue("description");
	if (!val.empty()) {
		description = val;
	}

	val = getValue("author");
	if (!val.empty()) {
		author = val;
	}

	val = getValue("version");
	if (!val.empty()) {
		version = val;
	}

	val = getValue("shortcut");
	if (!val.empty()) {
		shortcut = val;
	}

	val = getValue("autorun");
	if (!val.empty()) {
		if (val == "true") {
			autorun = true;
		}
	} else {
		// Also check boolean literal
		if (content.find("autorun = true") != std::string::npos || content.find("autorun=true") != std::string::npos) {
			autorun = true;
		}
	}

	val = getValue("main");
	if (!val.empty()) {
		// Add .lua extension if not present
		if (val.size() < 4 || val.substr(val.size() - 4) != ".lua") {
			val += ".lua";
		}
		filepath = directory + "/" + val;
		filename = val;
	} else {
		// Default to main.lua
		filepath = directory + "/main.lua";
		filename = "main.lua";
	}
}

void LuaScript::parseMetadataFromComments() {
	// Try to read script header comments for metadata
	// Format:
	// -- @Title: Script Name
	// -- @Description: Description...
	// -- @Author: Author Name
	// -- @Version: Version Number
	// -- @Shortcut: Ctrl+K
	// -- Or implicit: First line name, subsequent lines description

	std::ifstream file(filepath);
	if (!file.is_open()) {
		return;
	}

	std::string line;
	bool foundName = false;
	std::ostringstream descBuilder;
	int lineNum = 0;
	const int maxHeaderLines = 20; // Only check first 20 lines for metadata

	while (std::getline(file, line) && lineNum < maxHeaderLines) {
		lineNum++;

		// Trim whitespace
		size_t start = line.find_first_not_of(" \t");
		if (start == std::string::npos) {
			continue;
		}
		line = line.substr(start);

		// Check if it's a comment
		if (line.substr(0, 2) != "--") {
			// Stop at first non-comment line
			break;
		}

		// Remove comment prefix and trim
		std::string comment = line.substr(2);
		start = comment.find_first_not_of(" \t");
		if (start == std::string::npos) {
			continue;
		}
		comment = comment.substr(start);

		// Skip empty comments
		if (comment.empty()) {
			continue;
		}

		// Check for tags
		if (comment.size() > 7 && comment.substr(0, 7) == "@Title:") {
			displayName = comment.substr(7);
			size_t s = displayName.find_first_not_of(" \t");
			if (s != std::string::npos) {
				displayName = displayName.substr(s);
			}
			foundName = true;
			continue;
		} else if (comment.size() > 13 && comment.substr(0, 13) == "@Description:") {
			std::string descPart = comment.substr(13);
			size_t s = descPart.find_first_not_of(" \t");
			if (s != std::string::npos) {
				descPart = descPart.substr(s);
			}

			if (descBuilder.tellp() > 0) {
				descBuilder << " ";
			}
			descBuilder << descPart;
			continue;
		} else if (comment.size() > 8 && comment.substr(0, 8) == "@Author:") {
			author = comment.substr(8);
			size_t s = author.find_first_not_of(" \t");
			if (s != std::string::npos) {
				author = author.substr(s);
			}
			continue;
		} else if (comment.size() > 9 && comment.substr(0, 9) == "@Version:") {
			version = comment.substr(9);
			size_t s = version.find_first_not_of(" \t");
			if (s != std::string::npos) {
				version = version.substr(s);
			}
			continue;
		} else if (comment.size() > 10 && comment.substr(0, 10) == "@Shortcut:") {
			shortcut = comment.substr(10);
			size_t s = shortcut.find_first_not_of(" \t");
			if (s != std::string::npos) {
				shortcut = shortcut.substr(s);
			}
			continue;
		} else if (comment.size() > 9 && (comment.substr(0, 9) == "@AutoRun:" || comment.substr(0, 9) == "@Autorun:")) {
			std::string ar = comment.substr(9);
			if (ar.find("true") != std::string::npos) {
				autorun = true;
			}
			continue;
		}

		// First meaningful comment line (that isn't a tag) is the name
		if (!foundName) {
			displayName = comment;
			foundName = true;
		} else {
			// Subsequent lines are description
			if (descBuilder.tellp() > 0) {
				descBuilder << " ";
			}
			descBuilder << comment;
		}
	}

	description = descBuilder.str();
}
