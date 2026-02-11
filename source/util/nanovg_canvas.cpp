#include "app/main.h"
#include "util/nanovg_canvas.h"
#include "rendering/core/text_renderer.h"

#include <glad/glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#include "util/nvg_utils.h"
#include <nanovg_gl.h>
#include "rendering/core/graphics.h"

#include <wx/dcclient.h>
#include <algorithm>

ScopedGLContext::ScopedGLContext(NanoVGCanvas* canvas) : m_canvas(canvas) {
	if (m_canvas) {
		m_canvas->MakeContextCurrent();
	}
}

NanoVGCanvas::NanoVGCanvas(wxWindow* parent, wxWindowID id, long style) :
	wxGLCanvas(parent, id, nullptr, wxDefaultPosition, wxDefaultSize, style) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &NanoVGCanvas::OnPaint, this);
	Bind(wxEVT_SIZE, &NanoVGCanvas::OnSize, this);
	Bind(wxEVT_MOUSEWHEEL, &NanoVGCanvas::OnMouseWheel, this);
	Bind(wxEVT_ERASE_BACKGROUND, &NanoVGCanvas::OnEraseBackground, this);

	// Scrollbar interaction events
	Bind(wxEVT_SCROLLWIN_TOP, &NanoVGCanvas::OnScroll, this);
	Bind(wxEVT_SCROLLWIN_BOTTOM, &NanoVGCanvas::OnScroll, this);
	Bind(wxEVT_SCROLLWIN_LINEUP, &NanoVGCanvas::OnScroll, this);
	Bind(wxEVT_SCROLLWIN_LINEDOWN, &NanoVGCanvas::OnScroll, this);
	Bind(wxEVT_SCROLLWIN_PAGEUP, &NanoVGCanvas::OnScroll, this);
	Bind(wxEVT_SCROLLWIN_PAGEDOWN, &NanoVGCanvas::OnScroll, this);
	Bind(wxEVT_SCROLLWIN_THUMBTRACK, &NanoVGCanvas::OnScroll, this);
	Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &NanoVGCanvas::OnScroll, this);
}

NanoVGCanvas::~NanoVGCanvas() {
	if (m_glContext) {
		SetCurrent(*m_glContext);
		ClearImageCache();
	}
}

void NanoVGCanvas::InitGL() {
	if (m_glInitialized) {
		return;
	}

	m_glContext = std::make_unique<wxGLContext>(this);
	if (!m_glContext->IsOK()) {
		m_glContext.reset();
		return;
	}

	SetCurrent(*m_glContext);

	if (!gladLoadGL()) {
		return;
	}

	m_nvg.reset(nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES));
	if (m_nvg) {
		// Load default font - same as TextRenderer for consistency
		TextRenderer::LoadFont(m_nvg.get());
		m_glInitialized = true;
	}
}

bool NanoVGCanvas::MakeContextCurrent() {
	if (!m_glContext) {
		return false;
	}
	SetCurrent(*m_glContext);
	return true;
}

void NanoVGCanvas::OnPaint(wxPaintEvent&) {
	wxPaintDC dc(this); // validates the paint event

	InitGL();
	if (!m_nvg || !m_glContext) {
		return;
	}

	SetCurrent(*m_glContext);

	int w, h;
	GetClientSize(&w, &h);

	glViewport(0, 0, w, h);
	glClearColor(m_bgRed, m_bgGreen, m_bgBlue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	NVGcontext* vg = m_nvg.get();
	nvgBeginFrame(vg, w, h, GetContentScaleFactor());
	nvgSave(vg);
	nvgTranslate(vg, 0, static_cast<float>(-m_scrollPos));

	// Call subclass implementation
	OnNanoVGPaint(vg, w, h);

	nvgRestore(vg);
	nvgEndFrame(vg);

	SwapBuffers();
}

void NanoVGCanvas::OnSize(wxSizeEvent& evt) {
	UpdateScrollbar(m_contentHeight);
	Refresh();
	evt.Skip();
}

void NanoVGCanvas::OnMouseWheel(wxMouseEvent& evt) {
	int rotation = evt.GetWheelRotation();
	m_scrollPos -= (rotation / 120) * m_scrollStep;

	int maxScroll = std::max(0, m_contentHeight - GetClientSize().y);
	m_scrollPos = std::clamp(m_scrollPos, 0, maxScroll);

	UpdateScrollbar(m_contentHeight);
	Refresh();
}

void NanoVGCanvas::OnEraseBackground(wxEraseEvent&) {
	// No-op to prevent flicker
}

void NanoVGCanvas::OnScroll(wxScrollWinEvent& evt) {
	int h = GetClientSize().y;
	int maxScroll = std::max(0, m_contentHeight - h);

	wxEventType type = evt.GetEventType();

	if (type == wxEVT_SCROLLWIN_TOP) {
		m_scrollPos = 0;
	} else if (type == wxEVT_SCROLLWIN_BOTTOM) {
		m_scrollPos = maxScroll;
	} else if (type == wxEVT_SCROLLWIN_LINEUP) {
		m_scrollPos = std::max(0, m_scrollPos - m_scrollStep);
	} else if (type == wxEVT_SCROLLWIN_LINEDOWN) {
		m_scrollPos = std::min(maxScroll, m_scrollPos + m_scrollStep);
	} else if (type == wxEVT_SCROLLWIN_PAGEUP) {
		m_scrollPos = std::max(0, m_scrollPos - h);
	} else if (type == wxEVT_SCROLLWIN_PAGEDOWN) {
		m_scrollPos = std::min(maxScroll, m_scrollPos + h);
	} else if (type == wxEVT_SCROLLWIN_THUMBTRACK || type == wxEVT_SCROLLWIN_THUMBRELEASE) {
		m_scrollPos = evt.GetPosition();
	}

	m_scrollPos = std::clamp(m_scrollPos, 0, maxScroll);
	UpdateScrollbar(m_contentHeight);
	Refresh();
}

void NanoVGCanvas::SetScrollPosition(int pos) {
	int maxScroll = std::max(0, m_contentHeight - GetClientSize().y);
	m_scrollPos = std::clamp(pos, 0, maxScroll);
	UpdateScrollbar(m_contentHeight);
	Refresh();
}

int NanoVGCanvas::GetOrCreateItemImage(uint16_t itemId) {
	int tex = GetCachedImage(itemId);
	if (tex > 0) {
		return tex;
	}

	NVGcontext* vg = GetNVGContext();
	if (!vg) {
		return 0;
	}

	tex = NvgUtils::CreateItemTexture(vg, itemId);
	if (tex > 0) {
		AddCachedImage(static_cast<uint64_t>(itemId), tex);
	}
	return tex;
}

int NanoVGCanvas::GetOrCreateSpriteTexture(NVGcontext* vg, Sprite* sprite) {
	if (!sprite) {
		return 0;
	}

	// Use full pointer as unique ID for 64-bit systems
	uint64_t spriteId = reinterpret_cast<uint64_t>(sprite);

	// Check cache first
	int existingTex = GetCachedImage(spriteId);
	if (existingTex > 0) {
		return existingTex;
	}

	// Try to get as GameSprite for RGBA access (Fast Path)
	GameSprite* gs = dynamic_cast<GameSprite*>(sprite);
	if (gs && !gs->spriteList.empty()) {
		return CreateGameSpriteTexture(vg, gs, spriteId);
	}

	// Generic Fallback (Slow Path via wxDC)
	return CreateGenericSpriteTexture(vg, sprite, spriteId);
}

int NanoVGCanvas::CreateGameSpriteTexture(NVGcontext* vg, GameSprite* gs, uint64_t spriteId) {
	// Calculate composite size
	int w = gs->width * 32;
	int h = gs->height * 32;
	if (w <= 0 || h <= 0) {
		return 0;
	}

	// Create composite RGBA buffer
	size_t bufferSize = static_cast<size_t>(w) * h * 4;
	std::vector<uint8_t> composite(bufferSize, 0);

	// Composite all layers
	int px = (gs->pattern_x >= 3) ? 2 : 0;
	for (int l = 0; l < gs->layers; ++l) {
		for (int sw = 0; sw < gs->width; ++sw) {
			for (int sh = 0; sh < gs->height; ++sh) {
				int idx = gs->getIndex(sw, sh, l, px, 0, 0, 0);
				if (idx < 0 || static_cast<size_t>(idx) >= gs->spriteList.size()) {
					continue;
				}

				auto image = gs->spriteList[idx];
				if (!image) {
					continue;
				}

				auto data = image->getRGBAData();
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
							composite[di + 0] = static_cast<uint8_t>(data[si + 0] * a + composite[di + 0] * ia);
							composite[di + 1] = static_cast<uint8_t>(data[si + 1] * a + composite[di + 1] * ia);
							composite[di + 2] = static_cast<uint8_t>(data[si + 2] * a + composite[di + 2] * ia);
							composite[di + 3] = std::max(composite[di + 3], sa);
						}
					}
				}
			}
		}
	}

	// Create NanoVG image
	return GetOrCreateImage(spriteId, composite.data(), w, h);
}

int NanoVGCanvas::CreateGenericSpriteTexture(NVGcontext* vg, Sprite* sprite, uint64_t spriteId) {
	wxSize sz = sprite->GetSize();
	int w = sz.x;
	int h = sz.y;

	// Determine best SpriteSize for DrawTo
	SpriteSize drawSize = SPRITE_SIZE_32x32;
	if (w <= 16 && h <= 16) {
		drawSize = SPRITE_SIZE_16x16;
	} else if (w > 32 || h > 32) {
		drawSize = SPRITE_SIZE_64x64;
	}

	wxBitmap bmp(w, h);
	// Need a DC to draw
	{
		wxMemoryDC mdc(bmp);
		// Initialize with transparent background
		mdc.SetBackground(wxBrush(wxColor(0, 0, 0), wxBRUSHSTYLE_TRANSPARENT));
		mdc.Clear();
		// Draw at 0,0 with its size
		sprite->DrawTo(&mdc, drawSize, 0, 0, w, h);
	}

	wxImage img = bmp.ConvertToImage();
	if (!img.IsOk()) {
		return 0;
	}

	// Convert to RGBA
	std::vector<uint8_t> rgba(w * h * 4);
	const uint8_t* data = img.GetData();
	const uint8_t* alpha = img.GetAlpha();
	bool hasAlpha = img.HasAlpha();

	for (int i = 0; i < w * h; ++i) {
		rgba[i * 4 + 0] = data[i * 3 + 0];
		rgba[i * 4 + 1] = data[i * 3 + 1];
		rgba[i * 4 + 2] = data[i * 3 + 2];
		if (hasAlpha && alpha) {
			rgba[i * 4 + 3] = alpha[i];
		} else {
			rgba[i * 4 + 3] = 255;
		}
	}

	return GetOrCreateImage(spriteId, rgba.data(), w, h);
}

void NanoVGCanvas::UpdateScrollbar(int contentHeight) {
	m_contentHeight = contentHeight;
	int h = GetClientSize().y;
	SetScrollbar(wxVERTICAL, m_scrollPos, h, contentHeight);
}

int NanoVGCanvas::GetOrCreateImage(uint64_t id, const uint8_t* data, int width, int height) {
	ScopedGLContext ctx(this);
	if (!m_nvg) {
		return 0;
	}

	auto it = m_imageCache.find(id);
	if (it != m_imageCache.end()) {
		// Update LRU
		m_lruList.remove(id);
		m_lruList.push_front(id);
		return it->second;
	}

	int tex = nvgCreateImageRGBA(m_nvg.get(), width, height, 0, data);
	if (tex > 0) {
		AddCachedImage(id, tex);
	}
	return tex;
}

void NanoVGCanvas::DeleteCachedImage(uint64_t id) {
	ScopedGLContext ctx(this);
	if (!m_nvg) {
		return;
	}

	auto it = m_imageCache.find(id);
	if (it != m_imageCache.end()) {
		nvgDeleteImage(m_nvg.get(), it->second);
		m_imageCache.erase(it);
		m_lruList.remove(id);
	}
}

void NanoVGCanvas::AddCachedImage(uint64_t id, int imageHandle) {
	if (imageHandle > 0) {
		ScopedGLContext ctx(this);
		auto it = m_imageCache.find(id);
		if (it != m_imageCache.end()) {
			if (m_nvg) {
				nvgDeleteImage(m_nvg.get(), it->second);
			}
			m_lruList.remove(id);
		}

		// Evict if over limit
		if (m_imageCache.size() >= m_maxCacheSize) {
			uint64_t last = m_lruList.back();
			auto lastIt = m_imageCache.find(last);
			if (lastIt != m_imageCache.end()) {
				if (m_nvg) {
					nvgDeleteImage(m_nvg.get(), lastIt->second);
				}
				m_imageCache.erase(lastIt);
			}
			m_lruList.pop_back();
		}

		m_imageCache[id] = imageHandle;
		m_lruList.push_front(id);
	}
}

void NanoVGCanvas::ClearImageCache() {
	ScopedGLContext ctx(this);
	if (!m_nvg) {
		return;
	}

	for (const auto& [id, tex] : m_imageCache) {
		nvgDeleteImage(m_nvg.get(), tex);
	}
	m_imageCache.clear();
}

int NanoVGCanvas::GetCachedImage(uint64_t id) const {
	// Const-cast to call MakeContextCurrent if needed, or just assume it's for lookup
	// Actually GetCachedImage doesn't call GL, it just looks in the map.
	// So it's fine.
	auto it = m_imageCache.find(id);
	if (it != m_imageCache.end()) {
		// Update LRU
		m_lruList.remove(id);
		m_lruList.push_front(id);
		return it->second;
	}
	return 0;
}

wxSize NanoVGCanvas::DoGetBestClientSize() const {
	return FromDIP(wxSize(200, 200));
}
