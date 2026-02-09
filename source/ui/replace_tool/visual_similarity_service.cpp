#include "app/main.h"
#include "ui/replace_tool/visual_similarity_service.h"
#include "ui/gui.h"
#include "game/items.h"
#include "rendering/core/graphics.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <memory>

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
	m_timer.Start(50); // Faster background indexing
}

// ============================================================================
// CORE ALGORITHM HELPERS (Mappingtool 1:1)
// ============================================================================

static bool IsFullyOpaqueRGBA(const uint8_t* rgba, int count, uint8_t threshold = 10) {
	for (int i = 0; i < count; ++i) {
		if (rgba[i * 4 + 3] <= threshold) {
			return false;
		}
	}
	return true;
}

static uint64_t CalculateAHashRGBA(const uint8_t* rgba, int w, int h) {
	// mappingtool: 1. Resize to 8x8 using box sampling
	// grayscale = 0.299*R + 0.587*G + 0.114*B
	uint8_t gray_8x8[64];
	double total_brightness = 0;

	float block_w = (float)w / 8.0f;
	float block_h = (float)h / 8.0f;

	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			double block_sum = 0;
			int pixel_count = 0;

			int start_x = (int)(x * block_w);
			int start_y = (int)(y * block_h);
			int end_x = (int)((x + 1) * block_w);
			int end_y = (int)((y + 1) * block_h);

			for (int py = start_y; py < end_y && py < h; ++py) {
				for (int px = start_x; px < end_x && px < w; ++px) {
					int idx = (py * w + px) * 4;
					double gray = 0.299 * rgba[idx] + 0.587 * rgba[idx + 1] + 0.114 * rgba[idx + 2];
					block_sum += gray;
					pixel_count++;
				}
			}

			uint8_t avg = (uint8_t)((pixel_count > 0) ? (block_sum / pixel_count) : 0);
			gray_8x8[y * 8 + x] = avg;
			total_brightness += avg;
		}
	}

	uint8_t global_avg = (uint8_t)(total_brightness / 64.0);
	uint64_t hash = 0;
	for (int i = 0; i < 64; ++i) {
		if (gray_8x8[i] >= global_avg) {
			hash |= (1ULL << i);
		}
	}
	return hash;
}

static std::vector<bool> ExtractBinaryMaskRGBA(const uint8_t* rgba, int count, int& outTruePixels) {
	std::vector<bool> mask(count);
	outTruePixels = 0;
	for (int i = 0; i < count; ++i) {
		// Strictly > 10 per mappingtool docs
		bool val = (rgba[i * 4 + 3] > 10);
		mask[i] = val;
		if (val) {
			outTruePixels++;
		}
	}
	return mask;
}

static std::vector<float> CalculateHistogramRGBA(const uint8_t* rgba, int count, uint8_t threshold = 10) {
	const int BINS = 8;
	std::vector<int> hist(BINS * BINS * BINS, 0);
	int pixel_count = 0;

	for (int i = 0; i < count; ++i) {
		if (rgba[i * 4 + 3] > threshold) {
			int r_bin = std::min((int)(rgba[i * 4 + 0] * BINS / 256), BINS - 1);
			int g_bin = std::min((int)(rgba[i * 4 + 1] * BINS / 256), BINS - 1);
			int b_bin = std::min((int)(rgba[i * 4 + 2] * BINS / 256), BINS - 1);

			int bin_idx = r_bin * BINS * BINS + g_bin * BINS + b_bin;
			hist[bin_idx]++;
			pixel_count++;
		}
	}

	std::vector<float> normalized(hist.size(), 0.0f);
	if (pixel_count > 0) {
		for (size_t i = 0; i < hist.size(); ++i) {
			normalized[i] = (float)hist[i] / pixel_count;
		}
	}
	return normalized;
}

static float CompareHistograms(const std::vector<float>& h1, const std::vector<float>& h2) {
	if (h1.empty() || h2.empty()) {
		return 0.0f;
	}
	float intersection = 0.0f;
	size_t minSize = std::min(h1.size(), h2.size());
	for (size_t i = 0; i < minSize; ++i) {
		intersection += std::min(h1[i], h2[i]);
	}
	return intersection;
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

static std::vector<bool> ResizeMask(const std::vector<bool>& src, int srcW, int srcH, int dstW, int dstH) {
	std::vector<bool> dst(dstW * dstH);
	for (int y = 0; y < dstH; ++y) {
		for (int x = 0; x < dstW; ++x) {
			// mappingtool: nearest neighbor scaling
			int sx = (x * srcW) / dstW;
			int sy = (y * srcH) / dstH;
			sx = std::min(sx, srcW - 1);
			sy = std::min(sy, srcH - 1);
			dst[y * dstW + x] = src[sy * srcW + sx];
		}
	}
	return dst;
}

static std::unique_ptr<uint8_t[]> CreateCompositeRGBA(GameSprite& gs, int& outW, int& outH) {
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

				// mappingtool logic: Right-to-left, bottom-to-top arrangement
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
	if (!gs) {
		return data;
	}

	int w, h;
	auto composite = CreateCompositeRGBA(*gs, w, h);
	if (!composite) {
		return data;
	}

	data.width = w;
	data.height = h;
	data.isOpaque = IsFullyOpaqueRGBA(composite.get(), w * h);

	// Store both for robustness
	data.aHash = CalculateAHashRGBA(composite.get(), w, h);
	data.binaryMask = ExtractBinaryMaskRGBA(composite.get(), w * h, data.truePixels);
	data.histogram = CalculateHistogramRGBA(composite.get(), w * h);

	return data;
}

void VisualSimilarityService::OnTimer(wxTimerEvent&) {
	int processed = 0;
	int maxId = g_items.getMaxID();

	// Process fewer items per tick to prevent UI lag and handle spikes
	while (processed < 100 && m_nextIdToIndex <= maxId) {
		uint16_t id = m_nextIdToIndex++;
		const ItemType& it = g_items.getItemType(id);
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
	VisualItemData sourceData;
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		auto it = itemDataCache.find(itemId);
		if (it != itemDataCache.end()) {
			sourceData = it->second;
		}
	}

	if (!isIndexed) {
		return {};
	}

	if (sourceData.width == 0) {
		sourceData = CalculateData(itemId);
		if (sourceData.width == 0) {
			return {};
		}
	}

	struct ScoredItem {
		uint16_t id;
		double score;
		float histogram;
		bool operator<(const ScoredItem& other) const {
			// mappingtool: sort by similarity_score DESC, then ID ASC
			if (std::abs(score - other.score) > 0.00001) {
				return score > other.score;
			}
			return id < other.id;
		}
	};
	std::vector<ScoredItem> candidates;

	std::vector<VisualItemData> targets;
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		targets.reserve(itemDataCache.size());
		for (const auto& pair : itemDataCache) {
			targets.push_back(pair.second);
		}
	}

	for (const auto& target : targets) {
		if (target.id == itemId) {
			continue;
		}

		double score = 0.0;
		float histScore = CompareHistograms(sourceData.histogram, target.histogram);

		if (sourceData.isOpaque) {
			// Strategy A: aHash for opaque sprites
			// Mappingtool: We only compare against other opaque sprites to avoid matching walls with items
			if (target.isOpaque) {
				int dist = HammingDistance(sourceData.aHash, target.aHash);
				// Score: 1.0 is perfect match (dist 0), 0.0 is max distance (64)
				score = 1.0 - (static_cast<double>(dist) / 64.0);
			} else {
				continue; // Strict 1:1 behavior
			}
		} else {
			// Strategy B: Standard Binary Mask for transparent sprites
			if (target.isOpaque) {
				continue; // Mappingtool: Transparent target does not match opaque candidate
			}

			int tp = 0;
			int sourceOnes = sourceData.truePixels;
			int targetOnes = target.truePixels;

			if (sourceData.width == target.width && sourceData.height == target.height) {
				for (size_t i = 0; i < sourceData.binaryMask.size(); ++i) {
					if (sourceData.binaryMask[i] && target.binaryMask[i]) {
						tp++;
					}
				}
			} else {
				// 1:1 Resize Logic (Nearest Neighbor)
				int w = std::max(sourceData.width, target.width);
				int h = std::max(sourceData.height, target.height);
				auto m1 = ResizeMask(sourceData.binaryMask, sourceData.width, sourceData.height, w, h);
				auto m2 = ResizeMask(target.binaryMask, target.width, target.height, w, h);
				sourceOnes = 0;
				targetOnes = 0;
				for (size_t i = 0; i < m1.size(); ++i) {
					if (m1[i]) {
						sourceOnes++;
					}
					if (m2[i]) {
						targetOnes++;
					}
					if (m1[i] && m2[i]) {
						tp++;
					}
				}
			}

			if (tp > 0) {
				// Dice: (2.0 * TP) / (|A| + |B|)
				score = (2.0 * tp) / (double)(sourceOnes + targetOnes);
			}
		}

		// Weighted Final Score: 70% Shape, 30% Color
		// mappingtool: The new score calculation already incorporates the shape similarity directly.
		// The histogram score is still calculated but not used in the final score for now.
		// score = (shapeScore * 0.7) + (histScore * 0.3); // Removed as per new strategy

		if (score > 0.0) {
			candidates.push_back({ target.id, score, histScore });
		}
	}

	if (count > candidates.size()) {
		count = candidates.size();
	}
	std::partial_sort(candidates.begin(), candidates.begin() + count, candidates.end());

	std::vector<uint16_t> results;
	for (size_t i = 0; i < count; ++i) {
		results.push_back(candidates[i].id);
	}
	return results;
}
