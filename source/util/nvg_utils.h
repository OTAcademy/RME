#ifndef RME_UTIL_NVG_UTILS_H_
#define RME_UTIL_NVG_UTILS_H_

#include "game/items.h"
#include "ui/gui.h"
#include <nanovg.h>
#include <vector>
#include <algorithm>

namespace NvgUtils {

	// Generates RGBA pixel data for a given item ID.
	// Returns a texture ID created in the given NanoVG context.
	// Returns 0 if generation fails.
	inline int CreateItemTexture(NVGcontext* vg, uint16_t id) {
		const ItemType& it = g_items.getItemType(id);
		GameSprite* gs = dynamic_cast<GameSprite*>(g_gui.gfx.getSprite(it.clientID));
		if (!gs) {
			return 0;
		}

		int w = gs->width * 32;
		int h = gs->height * 32;
		if (w <= 0 || h <= 0) {
			return 0;
		}

		size_t bufferSize = static_cast<size_t>(w) * h * 4;
		std::vector<uint8_t> composite(bufferSize, 0);

		int px = (gs->pattern_x >= 3) ? 2 : 0;
		for (int l = 0; l < gs->layers; ++l) {
			for (int sw = 0; sw < gs->width; ++sw) {
				for (int sh = 0; sh < gs->height; ++sh) {
					int idx = gs->getIndex(sw, sh, l, px, 0, 0, 0);
					if (idx < 0 || (size_t)idx >= gs->spriteList.size()) {
						continue;
					}

					auto data = gs->spriteList[idx]->getRGBAData();
					if (!data) {
						continue;
					}

					int part_x = (gs->width - sw - 1) * 32;
					int part_y = (gs->height - sh - 1) * 32;

					for (int sy = 0; sy < 32; ++sy) {
						for (int sx = 0; sx < 32; ++sx) {
							int dy = part_y + sy;
							int dx = part_x + sx;
							int di = (dy * w + dx) * 4;
							int si = (sy * 32 + sx) * 4;

							uint8_t sa = data[si + 3];
							if (sa == 0) {
								continue;
							}

							if (sa == 255) {
								composite[di + 0] = data[si + 0];
								composite[di + 1] = data[si + 1];
								composite[di + 2] = data[si + 2];
								composite[di + 3] = 255;
							} else {
								float a = sa / 255.0f;
								float ia = 1.0f - a;
								composite[di + 0] = (uint8_t)(data[si + 0] * a + composite[di + 0] * ia);
								composite[di + 1] = (uint8_t)(data[si + 1] * a + composite[di + 1] * ia);
								composite[di + 2] = (uint8_t)(data[si + 2] * a + composite[di + 2] * ia);
								composite[di + 3] = std::max(composite[di + 3], sa);
							}
						}
					}
				}
			}
		}

		return nvgCreateImageRGBA(vg, w, h, 0, composite.data());
	}

} // namespace NvgUtils

#endif
