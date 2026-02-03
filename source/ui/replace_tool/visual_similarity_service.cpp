#include "ui/replace_tool/visual_similarity_service.h"
#include "ui/gui.h"
#include "game/items.h"
#include "rendering/core/graphics.h"
#include <algorithm>
#include <cmath>
#include <vector>

VisualSimilarityService& VisualSimilarityService::Get() {
	static VisualSimilarityService instance;
	return instance;
}

VisualSimilarityService::VisualSimilarityService() : m_timer(this), m_nextIdToIndex(1), isIndexed(false) {
	Bind(wxEVT_TIMER, &VisualSimilarityService::OnTimer, this, m_timer.GetId());
}

VisualSimilarityService::~VisualSimilarityService() {
	m_timer.Stop();
}

void VisualSimilarityService::StartIndexing() {
	if (isIndexed || m_timer.IsRunning()) {
		return;
	}
	m_timer.Start(20); // Fast interval
}

// ============================================================================
// CORE ALGORITHM HELPERS (RGBA versions)
// ============================================================================

static bool IsFullyOpaqueRGBA(const uint8_t* rgba, int count) {
	for (int i = 0; i < count; ++i) {
		if (rgba[i * 4 + 3] <= 10) {
			return false;
		}
	}
	return true;
}

static uint64_t CalculateAHashRGBA(const uint8_t* rgba, int w, int h) {
	// Simple 8x8 downsample and average
	// If it's not 32x32, we'll just use the first 8x8 block or scale poorly for now
	// but Tibia sprites are 32x32.

	long total = 0;
	uint8_t gray[64];

	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			// Box average 4x4 block for 32x32 source
			int r_sum = 0, g_sum = 0, b_sum = 0;
			int samples = 0;

			int sx_start = (col * w) / 8;
			int sx_end = ((col + 1) * w) / 8;
			int sy_start = (row * h) / 8;
			int sy_end = ((row + 1) * h) / 8;

			for (int sy = sy_start; sy < sy_end; ++sy) {
				for (int sx = sx_start; sx < sx_end; ++sx) {
					int idx = (sy * w + sx) * 4;
					r_sum += rgba[idx + 0];
					g_sum += rgba[idx + 1];
					b_sum += rgba[idx + 2];
					samples++;
				}
			}

			int gray_val = samples > 0 ? (r_sum + g_sum + b_sum) / (3 * samples) : 0;
			gray[row * 8 + col] = (uint8_t)gray_val;
			total += gray_val;
		}
	}

	int avg = total / 64;
	uint64_t hash = 0;
	for (int i = 0; i < 64; ++i) {
		if (gray[i] >= avg) {
			hash |= (1ULL << i);
		}
	}
	return hash;
}

static std::vector<bool> ExtractBinaryMaskRGBA(const uint8_t* rgba, int count, int& outTruePixels) {
	std::vector<bool> mask(count);
	outTruePixels = 0;
	for (int i = 0; i < count; ++i) {
		bool val = (rgba[i * 4 + 3] > 10);
		mask[i] = val;
		if (val) {
			outTruePixels++;
		}
	}
	return mask;
}

static int HammingDistance(uint64_t h1, uint64_t h2) {
	uint64_t x = h1 ^ h2;
	int dist = 0;
	while (x) {
		x &= (x - 1);
		dist++;
	}
	return dist;
}

// Nearest Neighbor Resize for Mask
static std::vector<bool> ResizeMask(const std::vector<bool>& src, int srcW, int srcH, int dstW, int dstH) {
	std::vector<bool> dst(dstW * dstH);
	for (int y = 0; y < dstH; ++y) {
		for (int x = 0; x < dstW; ++x) {
			int sx = (x * srcW) / dstW;
			int sy = (y * srcH) / dstH;
			if (sx >= srcW) {
				sx = srcW - 1;
			}
			if (sy >= srcH) {
				sy = srcH - 1;
			}
			dst[y * dstW + x] = src[sy * srcW + sx];
		}
	}
	return dst;
}

// ============================================================================
// SERVICE IMPLEMENTATION
// ============================================================================

VisualSimilarityService::VisualItemData VisualSimilarityService::CalculateData(uint16_t itemId) {
	VisualItemData data;
	data.id = itemId;
	data.isOpaque = false;
	data.aHash = 0;
	data.width = 0;
	data.height = 0;
	data.truePixels = 0;

	if (!g_version.getLoadedVersion()) {
		return data;
	}

	const ItemType& it = g_items.getItemType(itemId);
	if (it.id == 0 || it.clientID == 0) {
		return data;
	}

	Sprite* sprite = g_gui.gfx.getSprite(it.clientID);
	if (!sprite) {
		return data;
	}

	GameSprite* gs = dynamic_cast<GameSprite*>(sprite);
	if (!gs || gs->spriteList.empty()) {
		return data;
	}

	// Use first sprite for shape similarity
	// This avoids complex compositing and is 1:1 with how shape search usually works
	auto rgba = gs->spriteList[0]->getRGBAData();
	if (!rgba) {
		return data;
	}

	// Standard Tibia sprite size
	int w = 32;
	int h = 32;

	// Populate Data
	data.width = w;
	data.height = h;
	data.isOpaque = IsFullyOpaqueRGBA(rgba.get(), w * h);

	if (data.isOpaque) {
		data.aHash = CalculateAHashRGBA(rgba.get(), w, h);
	} else {
		data.binaryMask = ExtractBinaryMaskRGBA(rgba.get(), w * h, data.truePixels);
	}

	return data;
}

void VisualSimilarityService::OnTimer(wxTimerEvent&) {
	int processed = 0;
	int maxId = g_items.getMaxID();

	// Process batch
	while (processed < 20 && m_nextIdToIndex <= maxId) {
		uint16_t id = m_nextIdToIndex++;
		const ItemType& it = g_items.getItemType(id);

		// Skip invalid or empty items
		if (it.id != 0 && it.clientID != 0) {
			VisualItemData data = CalculateData(id);
			if (data.width > 0) {
				std::lock_guard<std::mutex> lock(dataMutex);
				itemDataCache[id] = std::move(data);
			}
		}
		processed++;
	}

	if (m_nextIdToIndex > maxId) {
		m_timer.Stop();
		isIndexed = true;
	}
}

std::vector<uint16_t> VisualSimilarityService::FindSimilar(uint16_t itemId, size_t count) {
	// 1. Get Source Data
	VisualItemData sourceData;
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		auto it = itemDataCache.find(itemId);
		if (it != itemDataCache.end()) {
			sourceData = it->second;
		}
	}

	// If not cached (e.g. indexing not done), calculate on fly
	if (sourceData.width == 0) {
		sourceData = CalculateData(itemId);
		if (sourceData.width == 0) {
			return {};
		}
	}

	struct ScoredItem {
		uint16_t id;
		double score;
		bool operator<(const ScoredItem& other) const {
			// Sort DESC by score, then ASC by ID
			if (std::abs(score - other.score) > 0.0001) {
				return score > other.score;
			}
			return id < other.id;
		}
	};
	std::vector<ScoredItem> candidates;

	// 2. Select Algorithm based on Source Type
	// Copy cache keys/values to avoid locking mutex during expensive loop?
	// Or just lock. Iteration over map is fast, comparison takes time.
	// For responsiveness, we should copy the vector of items then process?
	// The cache might be large (10k items). Copying is okay.
	std::vector<VisualItemData> targets;
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		targets.reserve(itemDataCache.size());
		for (const auto& pair : itemDataCache) {
			targets.push_back(pair.second);
		}
	}

	if (sourceData.isOpaque) {
		// ALGORITHM A: Opaque (aHash)
		for (const auto& target : targets) {
			if (target.id == itemId) {
				continue;
			}
			if (!target.isOpaque) {
				continue; // Only compare opaque with opaque per doc
			}

			int dist = HammingDistance(sourceData.aHash, target.aHash);
			double score = 1.0 - (dist / 64.0);
			if (score > 0.0) {
				candidates.push_back({ target.id, score });
			}
		}
	} else {
		// ALGORITHM B: Transparent (Dice on Binary Mask)
		for (const auto& target : targets) {
			if (target.id == itemId) {
				continue;
			}
			if (target.isOpaque) {
				continue; // Skip opaque targets? Doc says "compare masks", implies similar types.
			}
			// Actually doc says "For opaque... use aHash... DO NOT compare opaque with transparent".
			// So if source is transparent, we should ideally compare with transparent.
			// But if we compare with opaque, the Opaque item has no mask stored in my structure (to save RAM).
			// So yes, skip opaque.

			// Dice Coefficient: 2*TP / (|A| + |B|)
			// Denominator
			int denom = sourceData.truePixels + target.truePixels;
			if (denom == 0) {
				continue;
			}

			int tp = 0;

			// Optimized path: Same Dimensions
			if (sourceData.width == target.width && sourceData.height == target.height) {
				// Linear compare
				size_t sz = sourceData.binaryMask.size();
				// Use raw access if possible, or just iter
				for (size_t i = 0; i < sz; ++i) {
					if (sourceData.binaryMask[i] && target.binaryMask[i]) {
						tp++;
					}
				}
			} else {
				// Resizing path
				int w = std::max(sourceData.width, target.width);
				int h = std::max(sourceData.height, target.height);

				std::vector<bool> m1 = ResizeMask(sourceData.binaryMask, sourceData.width, sourceData.height, w, h);
				std::vector<bool> m2 = ResizeMask(target.binaryMask, target.width, target.height, w, h);

				for (size_t i = 0; i < m1.size(); ++i) {
					if (m1[i] && m2[i]) {
						tp++;
					}
				}
				// NOTICE: Denominator strictly should be count of 1s in RESIZED masks?
				// Math wise: Scaling up preserves ratio of 1s approx.
				// Dice = 2*|A n B| / (|A| + |B|).
				// If we resize, |A| changes (scales up).
				// So we should recount 1s in m1/m2.
				denom = 0;
				for (bool b : m1) {
					if (b) {
						denom++;
					}
				}
				for (bool b : m2) {
					if (b) {
						denom++;
					}
				}
			}

			if (denom > 0) {
				double score = (2.0 * tp) / denom;
				if (score > 0.1) { // Min threshold
					candidates.push_back({ target.id, score });
				}
			}
		}
	}

	// 3. Sort and Limit
	// Use partial sort for top N
	if (count > candidates.size()) {
		count = candidates.size();
	}
	std::partial_sort(candidates.begin(), candidates.begin() + count, candidates.end());

	std::vector<uint16_t> results;
	results.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		results.push_back(candidates[i].id);
	}
	return results;
}
