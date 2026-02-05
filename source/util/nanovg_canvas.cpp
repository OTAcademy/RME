#include "app/main.h"
#include "util/nanovg_canvas.h"
#include "rendering/core/text_renderer.h"

#include <glad/glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>
#include "rendering/core/graphics.h"

#include <wx/dcclient.h>
#include <algorithm>

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
	wxPaintDC dc(this); // Required even if unused

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
	nvgBeginFrame(vg, w, h, 1.0f);
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

void NanoVGCanvas::UpdateScrollbar(int contentHeight) {
	m_contentHeight = contentHeight;
	int h = GetClientSize().y;
	SetScrollbar(wxVERTICAL, m_scrollPos, h, contentHeight);
}

int NanoVGCanvas::GetOrCreateImage(uint32_t id, const uint8_t* data, int width, int height) {
	if (!m_nvg) {
		return 0;
	}

	auto it = m_imageCache.find(id);
	if (it != m_imageCache.end()) {
		return it->second;
	}

	int tex = nvgCreateImageRGBA(m_nvg.get(), width, height, 0, data);
	if (tex > 0) {
		m_imageCache[id] = tex;
	}
	return tex;
}

void NanoVGCanvas::DeleteCachedImage(uint32_t id) {
	if (!m_nvg) {
		return;
	}

	auto it = m_imageCache.find(id);
	if (it != m_imageCache.end()) {
		nvgDeleteImage(m_nvg.get(), it->second);
		m_imageCache.erase(it);
	}
}

void NanoVGCanvas::ClearImageCache() {
	if (!m_nvg) {
		return;
	}

	for (const auto& [id, tex] : m_imageCache) {
		nvgDeleteImage(m_nvg.get(), tex);
	}
	m_imageCache.clear();
}

int NanoVGCanvas::GetCachedImage(uint32_t id) const {
	auto it = m_imageCache.find(id);
	if (it != m_imageCache.end()) {
		return it->second;
	}
	return 0;
}

wxSize NanoVGCanvas::DoGetBestClientSize() const {
	return FromDIP(wxSize(200, 200));
}
