#ifndef RME_UTIL_NVG_UTILS_H_
#define RME_UTIL_NVG_UTILS_H_

#include "game/items.h"
#include "ui/gui.h"
#include <nanovg.h>
#include <vector>
#include <algorithm>
#include <memory>

namespace NvgUtils {

	// Generates RGBA pixel data for a given item ID.
	// Returns a texture ID created in the given NanoVG context.
	// Returns 0 if generation fails.
	// Creates a composite RGBA buffer from a GameSprite.
	// Returns a unique_ptr to the buffer, or nullptr on failure.
	inline std::unique_ptr<uint8_t[]> CreateCompositeRGBA(GameSprite& gs, int& outW, int& outH) {
		outW = gs.width * 32;
		outH = gs.height * 32;

		if (outW <= 0 || outH <= 0) {
			return nullptr;
		}

		size_t bufferSize = static_cast<size_t>(outW) * outH * 4;
		auto composite = std::make_unique<uint8_t[]>(bufferSize);
		std::fill(composite.get(), composite.get() + bufferSize, 0);

		int pattern_x = (gs.pattern_x >= 3) ? 2 : 0;
		int pattern_y = 0;
		int pattern_z = 0;
		int frame = 0;

		for (int l = 0; l < gs.layers; ++l) {
			for (int w = 0; w < gs.width; ++w) {
				for (int h = 0; h < gs.height; ++h) {
					int spriteIdx = gs.getIndex(w, h, l, pattern_x, pattern_y, pattern_z, frame);
					if (spriteIdx < 0 || (size_t)spriteIdx >= gs.spriteList.size()) {
						continue;
					}

					auto spriteData = gs.spriteList[spriteIdx]->getRGBAData();
					if (!spriteData) {
						continue;
					}

					// Right-to-left, bottom-to-top arrangement (standard RME rendering order)
					int part_x = (gs.width - w - 1) * 32;
					int part_y = (gs.height - h - 1) * 32;

					for (int sy = 0; sy < 32; ++sy) {
						for (int sx = 0; sx < 32; ++sx) {
							int dy = part_y + sy;
							int dx = part_x + sx;

							if (dx < 0 || dx >= outW || dy < 0 || dy >= outH) {
								continue;
							}

							int src_idx = (sy * 32 + sx) * 4;
							int dst_idx = (dy * outW + dx) * 4;

							uint8_t sa = spriteData[src_idx + 3];
							if (sa == 0) {
								continue;
							}

							if (sa == 255) {
								composite[dst_idx + 0] = spriteData[src_idx + 0];
								composite[dst_idx + 1] = spriteData[src_idx + 1];
								composite[dst_idx + 2] = spriteData[src_idx + 2];
								composite[dst_idx + 3] = 255;
							} else {
								float a = sa / 255.0f;
								float inv_a = 1.0f - a;
								composite[dst_idx + 0] = (uint8_t)(spriteData[src_idx + 0] * a + composite[dst_idx + 0] * inv_a);
								composite[dst_idx + 1] = (uint8_t)(spriteData[src_idx + 1] * a + composite[dst_idx + 1] * inv_a);
								composite[dst_idx + 2] = (uint8_t)(spriteData[src_idx + 2] * a + composite[dst_idx + 2] * inv_a);
								composite[dst_idx + 3] = std::max(composite[dst_idx + 3], sa);
							}
						}
					}
				}
			}
		}
		return composite;
	}

	// Generates RGBA pixel data for a given item ID.
	// Returns a texture ID created in the given NanoVG context.
	// Returns 0 if generation fails.
	inline int CreateItemTexture(NVGcontext* vg, uint16_t id) {
		const ItemType& it = g_items.getItemType(id);
		GameSprite* gs = dynamic_cast<GameSprite*>(g_gui.gfx.getSprite(it.clientID));
		if (!gs) {
			return 0;
		}

		int w, h;
		auto composite = CreateCompositeRGBA(*gs, w, h);
		if (!composite) {
			return 0;
		}

		return nvgCreateImageRGBA(vg, w, h, 0, composite.get());
	}

} // namespace NvgUtils

#endif
