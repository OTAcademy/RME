//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "rendering/core/outfit_colorizer.h"
#include "rendering/core/outfit_colors.h"

void OutfitColorizer::ColorizePixel(uint8_t color, uint8_t& red, uint8_t& green, uint8_t& blue) {
	// Thanks! Khaos, or was it mips? Hmmm... =)
	uint8_t ro = (TemplateOutfitLookupTable[color] & 0xFF0000) >> 16; // rgb outfit
	uint8_t go = (TemplateOutfitLookupTable[color] & 0xFF00) >> 8;
	uint8_t bo = (TemplateOutfitLookupTable[color] & 0xFF);
	red = (uint8_t)(red * (ro / 255.f));
	green = (uint8_t)(green * (go / 255.f));
	blue = (uint8_t)(blue * (bo / 255.f));
}
