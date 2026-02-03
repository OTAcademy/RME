#ifndef RME_VISUAL_SIMILARITY_SERVICE_H_
#define RME_VISUAL_SIMILARITY_SERVICE_H_

#include "app/main.h"
#include <wx/timer.h>
#include <vector>
#include <map>
#include <mutex>
#include <cstdint>

class VisualSimilarityService : public wxEvtHandler {
public:
	static VisualSimilarityService& Get();

	// Find top N similar items
	std::vector<uint16_t> FindSimilar(uint16_t itemId, size_t count = 20);

	// Start background indexing
	void StartIndexing();

	// Calculate a single hash (useful for preview)
	uint64_t CalculateHash(uint16_t itemId);

private:
	VisualSimilarityService();
	~VisualSimilarityService();

	void OnTimer(wxTimerEvent& event);

	std::map<uint16_t, uint64_t> itemHashes;
	std::mutex hashMutex;

	wxTimer m_timer;
	uint16_t m_nextIdToIndex;
	bool isIndexed;
};

#endif
