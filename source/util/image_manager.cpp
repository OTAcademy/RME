//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "util/image_manager.h"
#include "util/file_system.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <nanovg.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <cstdint>

ImageManager& ImageManager::GetInstance() {
	static ImageManager instance;
	return instance;
}

ImageManager::ImageManager() {
}

ImageManager::~ImageManager() {
	ClearCache();
}

void ImageManager::ClearCache() {
	m_bitmapBundleCache.clear();
	m_tintedBitmapCache.clear();
	m_nvgImageCache.clear();
	m_glTextureCache.clear();
}

std::string ImageManager::ResolvePath(const std::string& assetPath) {
	// The path should be relative to the executable's "assets" directory
	static wxString executablePath = wxStandardPaths::Get().GetExecutablePath();
	static wxString assetsRoot = wxFileName(executablePath).GetPath() + wxFileName::GetPathSeparator() + "assets";

	// Use full path joining to avoid stripping subdirectories
	wxString fullPathLine = assetsRoot + wxFileName::GetPathSeparator() + wxString::FromUTF8(assetPath);
	wxFileName fn(fullPathLine);

	std::string fullPath = fn.GetFullPath().ToStdString();
	spdlog::debug("ImageManager: Resolving {} -> {}", assetPath, fullPath);
	return fullPath;
}

wxBitmapBundle ImageManager::GetBitmapBundle(const std::string& assetPath, const wxColour& tint) {
	std::string fullPath = ResolvePath(assetPath);
	std::string cacheKey = assetPath;
	if (tint.IsOk()) {
		cacheKey += "_" + std::to_string(tint.GetRGB());
	}

	auto it = m_bitmapBundleCache.find(cacheKey);
	if (it != m_bitmapBundleCache.end()) {
		return it->second;
	}

	wxBitmapBundle bundle;
	if (wxFileName::FileExists(fullPath)) {
		if (assetPath.ends_with(".svg")) {
			bundle = wxBitmapBundle::FromSVGFile(fullPath, wxSize(16, 16));
			if (!bundle.IsOk()) {
				spdlog::error("ImageManager: Failed to load SVG bundle: {}", fullPath);
			}
		} else {
			wxBitmap bmp(fullPath, wxBITMAP_TYPE_PNG);
			if (bmp.IsOk()) {
				bundle = wxBitmapBundle::FromBitmap(bmp);
			} else {
				spdlog::error("ImageManager: Failed to load PNG bitmap: {}", fullPath);
			}
		}
	} else {
		spdlog::error("ImageManager: Asset file not found: {}", fullPath);
	}

	if (bundle.IsOk() && tint.IsOk()) {
		// If we need a tinted bundle, we might need to recreate it from tinted images
		// For simplicity now, let's just cache it. SVGs can be tinted via XML manipulation
		// but wxBitmapBundle doesn't expose that easily.
		// We could use GetBitmap and create a bundle from that.
	}

	m_bitmapBundleCache[cacheKey] = bundle;
	return bundle;
}

wxBitmap ImageManager::GetBitmap(const std::string& assetPath, const wxSize& size, const wxColour& tint) {
	wxBitmapBundle bundle = GetBitmapBundle(assetPath);
	if (!bundle.IsOk()) {
		return wxNullBitmap;
	}

	wxSize actualSize = size == wxDefaultSize ? bundle.GetDefaultSize() : size;

	if (!tint.IsOk()) {
		return bundle.GetBitmap(actualSize);
	}

	// For tinted bitmaps, use separate cache
	std::pair<std::string, uint32_t> cacheKey = { assetPath, (uint32_t)tint.GetRGB() };
	auto it = m_tintedBitmapCache.find(cacheKey);
	if (it != m_tintedBitmapCache.end()) {
		return it->second;
	}

	wxImage img = bundle.GetBitmap(actualSize).ConvertToImage();
	if (img.IsOk()) {
		img = TintImage(img, tint);
		wxBitmap tintedBmp(img);
		m_tintedBitmapCache[cacheKey] = tintedBmp;
		return tintedBmp;
	}

	return wxNullBitmap;
}

wxImage ImageManager::TintImage(const wxImage& image, const wxColour& tint) {
	wxImage tinted = image.Copy();
	if (!tinted.HasAlpha()) {
		tinted.InitAlpha();
	}

	unsigned char r = tint.Red();
	unsigned char g = tint.Green();
	unsigned char b = tint.Blue();

	unsigned char* data = tinted.GetData();
	unsigned char* alpha = tinted.GetAlpha();
	int size = tinted.GetWidth() * tinted.GetHeight();

	for (int i = 0; i < size; ++i) {
		// Basic tinting: multiply color by tint, keep alpha
		// This works well for white/grayscale icons
		data[i * 3 + 0] = (unsigned char)((data[i * 3 + 0] * r) / 255);
		data[i * 3 + 1] = (unsigned char)((data[i * 3 + 1] * g) / 255);
		data[i * 3 + 2] = (unsigned char)((data[i * 3 + 2] * b) / 255);
	}

	return tinted;
}

int ImageManager::GetNanoVGImage(NVGcontext* vg, const std::string& assetPath) {
	auto it = m_nvgImageCache.find(assetPath);
	if (it != m_nvgImageCache.end()) {
		return it->second;
	}

	std::string fullPath = ResolvePath(assetPath);
	int img = 0;

	if (assetPath.ends_with(".svg")) {
		// NanoVG doesn't native load SVG. We need to rasterize it or use a separate SVG parser.
		// For now, let's Rasterize it via wxImage (easiest bridge)
		wxBitmapBundle bundle = wxBitmapBundle::FromSVGFile(fullPath, wxSize(128, 128)); // High res raster
		if (bundle.IsOk()) {
			wxImage image = bundle.GetBitmap(wxSize(128, 128)).ConvertToImage();
			if (image.IsOk()) {
				int w = image.GetWidth();
				int h = image.GetHeight();
				std::vector<uint8_t> rgba(w * h * 4);
				unsigned char* data = image.GetData();
				unsigned char* alpha = image.GetAlpha();
				bool hasAlpha = image.HasAlpha();

				for (int i = 0; i < w * h; ++i) {
					rgba[i * 4 + 0] = data[i * 3 + 0];
					rgba[i * 4 + 1] = data[i * 3 + 1];
					rgba[i * 4 + 2] = data[i * 3 + 2];
					rgba[i * 4 + 3] = (hasAlpha && alpha) ? alpha[i] : 255;
				}
				img = nvgCreateImageRGBA(vg, w, h, 0, rgba.data());
			}
		}
	} else {
		img = nvgCreateImage(vg, fullPath.c_str(), 0);
	}

	if (img != 0) {
		m_nvgImageCache[assetPath] = img;
	} else {
		spdlog::error("Failed to load NanoVG image: {}", assetPath);
	}

	return img;
}

uint32_t ImageManager::GetGLTexture(const std::string& assetPath) {
	// Not implemented yet - usually we can use NanoVG's image as GL texture if we know how it's stored,
	// or load it via glad.
	return 0;
}
