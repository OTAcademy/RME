#ifndef RME_INGAME_PREVIEW_MANAGER_H_
#define RME_INGAME_PREVIEW_MANAGER_H_

#include "app/main.h"

#include <memory>

namespace IngamePreview {

	class IngamePreviewWindow;

	class IngamePreviewManager {
	public:
		IngamePreviewManager();
		~IngamePreviewManager();

		void Create();
		void Hide();
		void Destroy();
		void Update();
		bool IsVisible() const;

		IngamePreviewWindow* GetWindow() {
			return window.get();
		}

	private:
		std::unique_ptr<IngamePreviewWindow> window;
	};

} // namespace IngamePreview

extern IngamePreview::IngamePreviewManager g_preview;

#endif // RME_INGAME_PREVIEW_MANAGER_H_
