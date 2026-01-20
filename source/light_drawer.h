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

#ifndef RME_LIGHDRAWER_H
#define RME_LIGHDRAWER_H

#include "graphics.h"
#include "position.h"

class LightDrawer {
	struct Light {
		uint16_t map_x = 0;
		uint16_t map_y = 0;
		uint8_t color = 0;
		uint8_t intensity = 0;
	};

public:
	LightDrawer();
	virtual ~LightDrawer();

	void draw(int map_x, int map_y, int end_x, int end_y, int scroll_x, int scroll_y, bool fog);

	void setGlobalLightColor(uint8_t color);
	void addLight(int map_x, int map_y, int map_z, const SpriteLight& light);
	void clear();

private:
	void createGLTexture();
	void unloadGLTexture();

	inline float calculateIntensity(int map_x, int map_y, const Light& light) {
		int dx = map_x - light.map_x;
		int dy = map_y - light.map_y;
		float distance = std::sqrt(dx * dx + dy * dy);
		if (distance > MaxLightIntensity) {
			return 0.f;
		}
		float intensity = (-distance + light.intensity) * 0.2f;
		if (intensity < 0.01f) {
			return 0.f;
		}
		return std::min(intensity, 1.f);
	}

	// Light caching
	bool dirty_ = true;
	int last_view_x_ = -1;
	int last_view_y_ = -1;
	int last_floor_ = -1;

public:
	void invalidate() {
		dirty_ = true;
	}
	bool needsUpdate(int vx, int vy, int flr) {
		if (dirty_ || vx != last_view_x_ || vy != last_view_y_ || flr != last_floor_) {
			last_view_x_ = vx;
			last_view_y_ = vy;
			last_floor_ = flr;
			dirty_ = false;
			return true;
		}
		return false;
	}

private:
	GLuint texture;
	std::vector<Light> lights;
	std::vector<uint8_t> buffer;
	wxColor global_color;
};

#endif
