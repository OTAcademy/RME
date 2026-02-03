#include "ui/replace_tool/visual_similarity_service.h"
#include "ui/gui.h"
#include "game/items.h"
#include "rendering/core/graphics.h"
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/bitmap.h>
#include <algorithm>

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
	m_timer.Start(50); // Every 50ms calculate a few more
}

// 64-bit Popcount (Hamming weight) software fallback
static int HammingDistance(uint64_t hash1, uint64_t hash2) {
	uint64_t x = hash1 ^ hash2;
	int count = 0;
	while (x) {
		x &= (x - 1);
		count++;
	}
	return count;
}

uint64_t VisualSimilarityService::CalculateHash(uint16_t itemId) {
	// Re-use sprite drawing logic
	const ItemType& it = g_items.getItemType(itemId);
	if (it.id == 0) {
		return 0;
	}

	// Create a temporary bitmap to draw the sprite
	wxBitmap bmp(32, 32);
	wxMemoryDC dc(bmp);
	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();

	// We need a 32x32 image to start with
	Sprite* sprite = g_gui.gfx.getSprite(itemId);
	if (sprite && g_version.getLoadedVersion()) {
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, 0, 0);
	} else {
		return 0; // No sprite or version not loaded
	}
	dc.SelectObject(wxNullBitmap);

	wxImage img = bmp.ConvertToImage();
	if (!img.IsOk()) {
		return 0;
	}

	// dHash Logic:
	// 1. Convert to grayscale
	// 2. Scale to 9x8 (unbalanced to get "horizontal" differences) or 8x8
	// 3. For each pixel, if pixel[x] > pixel[x+1], bit = 1

	wxImage smallImg = img.Scale(9, 8, wxIMAGE_QUALITY_NORMAL).ConvertToGreyscale();

	uint64_t hash = 0;
	int bit = 0;
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			if (smallImg.GetRed(x, y) > smallImg.GetRed(x + 1, y)) {
				hash |= (1ULL << bit);
			}
			bit++;
		}
	}
	return hash;
}

void VisualSimilarityService::OnTimer(wxTimerEvent&) {
	// Index in small chunks to avoid UI lag
	int processed = 0;
	while (processed < 50 && m_nextIdToIndex <= g_items.getMaxID()) {
		uint16_t id = m_nextIdToIndex++;
		const ItemType& it = g_items.getItemType(id);
		if (it.id != 0) {
			uint64_t hash = CalculateHash(id);
			if (hash != 0) {
				std::lock_guard<std::mutex> lock(hashMutex);
				itemHashes[id] = hash;
			}
			processed++;
		}
	}

	if (m_nextIdToIndex > g_items.getMaxID()) {
		m_timer.Stop();
		isIndexed = true;
	}
}

std::vector<uint16_t> VisualSimilarityService::FindSimilar(uint16_t itemId, size_t count) {
	uint64_t targetHash = 0;
	{
		std::lock_guard<std::mutex> lock(hashMutex);
		auto it = itemHashes.find(itemId);
		if (it != itemHashes.end()) {
			targetHash = it->second;
		}
	}

	if (targetHash == 0) {
		targetHash = CalculateHash(itemId);
		if (targetHash == 0) {
			return {};
		}
	}

	struct ScoredItem {
		uint16_t id;
		int score;
		bool operator<(const ScoredItem& other) const {
			return score < other.score;
		}
	};
	std::vector<ScoredItem> potentials;

	{
		std::lock_guard<std::mutex> lock(hashMutex);
		for (const auto& [id, hash] : itemHashes) {
			if (id == itemId) {
				continue;
			}
			int dist = HammingDistance(targetHash, hash);
			if (dist < 15) { // Threshold for similarity
				potentials.push_back({ id, dist });
			}
		}
	}

	std::sort(potentials.begin(), potentials.end());

	std::vector<uint16_t> results;
	for (size_t i = 0; i < std::min(count, potentials.size()); ++i) {
		results.push_back(potentials[i].id);
	}
	return results;
}
