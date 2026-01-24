#include "light_buffer.h"

void LightBuffer::AddLight(int map_x, int map_y, int map_z, const SpriteLight& light) {
	if (map_z <= GROUND_LAYER) {
		map_x -= (GROUND_LAYER - map_z);
		map_y -= (GROUND_LAYER - map_z);
	}

	if (map_x <= 0 || map_x >= MAP_MAX_WIDTH || map_y <= 0 || map_y >= MAP_MAX_HEIGHT) {
		return;
	}

	uint8_t intensity = std::min(light.intensity, static_cast<uint8_t>(255)); // Assumed max

	if (!lights.empty()) {
		Light& previous = lights.back();
		if (previous.map_x == map_x && previous.map_y == map_y && previous.color == light.color) {
			previous.intensity = std::max(previous.intensity, intensity);
			return;
		}
	}

	lights.push_back(Light { static_cast<uint16_t>(map_x), static_cast<uint16_t>(map_y), light.color, intensity });
}

void LightBuffer::Clear() {
	lights.clear();
}
