#ifndef RME_VISUAL_SIMILARITY_SERVICE_H_
#define RME_VISUAL_SIMILARITY_SERVICE_H_

#include "app/main.h"
#include <wx/event.h>
#include <wx/timer.h>
#include <vector>
#include <map>
#include <mutex>
#include <cstdint>

class VisualSimilarityService : public wxEvtHandler {
public:
	static VisualSimilarityService& Get();

	struct VisualItemData {
		uint16_t id;
		bool isOpaque;
		uint64_t aHash;
		int width;
		int height;
		std::vector<bool> binaryMask; // Flattened mask
		int truePixels; // Count of 1s
		std::vector<float> histogram; // 512-bin RGB histogram (normalized)
	};

	// Find top N similar items
	std::vector<uint16_t> FindSimilar(uint16_t itemId, size_t count = 50);

	// Start background indexing
	void StartIndexing();

	// Calculate data for a single item (useful for preview/debug)
	VisualItemData CalculateData(uint16_t itemId);

private:
	VisualSimilarityService();
	~VisualSimilarityService();

	void OnTimer(wxTimerEvent& event);

	std::map<uint16_t, VisualItemData> itemDataCache;
	std::mutex dataMutex;

	wxTimer m_timer;
	uint16_t m_nextIdToIndex;
	bool isIndexed;
};

#endif
