//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/managers/doodad_preview_manager.h"
#include "ui/gui.h"
#include "map/map.h"
#include "brushes/doodad_brush.h"
#include "brushes/brush.h"
#include "brushes/managers/brush_manager.h"
#include "game/sprites.h"

DoodadPreviewManager g_doodad_preview;

DoodadPreviewManager::DoodadPreviewManager() : doodad_buffer_map(std::make_unique<BaseMap>()) {
}

DoodadPreviewManager::~DoodadPreviewManager() = default;

void DoodadPreviewManager::FillBuffer() {
	Brush* current_brush = g_brush_manager.GetCurrentBrush();
	if (!current_brush || !current_brush->isDoodad()) {
		return;
	}

	doodad_buffer_map->clear();

	DoodadBrush* brush = current_brush->asDoodad();
	if (brush->isEmpty(g_brush_manager.GetBrushVariation())) {
		return;
	}

	int object_count = 0;
	int area;
	if (g_brush_manager.GetBrushShape() == BRUSHSHAPE_SQUARE) {
		area = 2 * g_brush_manager.GetBrushSize();
		area = area * area + 1;
	} else {
		if (g_brush_manager.GetBrushSize() == 1) {
			// There is a huge deviation here with the other formula.
			area = 5;
		} else {
			area = int(0.5 + g_brush_manager.GetBrushSize() * g_brush_manager.GetBrushSize() * PI);
		}
	}
	const int object_range = (g_brush_manager.UseCustomThickness() ? int(area * g_brush_manager.GetCustomThicknessMod()) : brush->getThickness() * area / std::max(1, brush->getThicknessCeiling()));
	const int final_object_count = std::max(1, object_range + random(object_range));

	Position center_pos(0x8000, 0x8000, 0x8);

	if (g_brush_manager.GetBrushSize() > 0 && !brush->oneSizeFitsAll()) {
		while (object_count < final_object_count) {
			int retries = 0;
			bool exit = false;

			// Try to place objects 5 times
			while (retries < 5 && !exit) {

				int pos_retries = 0;
				int xpos = 0, ypos = 0;
				bool found_pos = false;
				if (g_brush_manager.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
					while (pos_retries < 5 && !found_pos) {
						xpos = random(-g_brush_manager.GetBrushSize(), g_brush_manager.GetBrushSize());
						ypos = random(-g_brush_manager.GetBrushSize(), g_brush_manager.GetBrushSize());
						float distance = sqrt(float(xpos * xpos) + float(ypos * ypos));
						if (distance < g_brush_manager.GetBrushSize() + 0.005) {
							found_pos = true;
						} else {
							++pos_retries;
						}
					}
				} else {
					found_pos = true;
					xpos = random(-g_brush_manager.GetBrushSize(), g_brush_manager.GetBrushSize());
					ypos = random(-g_brush_manager.GetBrushSize(), g_brush_manager.GetBrushSize());
				}

				if (!found_pos) {
					++retries;
					continue;
				}

				// Decide whether the zone should have a composite or several single objects.
				bool fail = false;
				if (random(brush->getTotalChance(g_brush_manager.GetBrushVariation())) <= brush->getCompositeChance(g_brush_manager.GetBrushVariation())) {
					// Composite
					const CompositeTileList& composites = brush->getComposite(g_brush_manager.GetBrushVariation());

					// Figure out if the placement is valid
					for (const auto& composite : composites) {
						Position pos = center_pos + composite.first + Position(xpos, ypos, 0);
						if (Tile* tile = doodad_buffer_map->getTile(pos)) {
							if (!tile->empty()) {
								fail = true;
								break;
							}
						}
					}
					if (fail) {
						++retries;
						break;
					}

					// Transfer items to the stack
					for (const auto& composite : composites) {
						Position pos = center_pos + composite.first + Position(xpos, ypos, 0);
						const ItemVector& items = composite.second;
						Tile* tile = doodad_buffer_map->getTile(pos);

						if (!tile) {
							tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));
						}

						for (auto item : items) {
							tile->addItem(item->deepCopy());
						}
						doodad_buffer_map->setTile(tile->getPosition(), tile);
					}
					exit = true;
				} else if (brush->hasSingleObjects(g_brush_manager.GetBrushVariation())) {
					Position pos = center_pos + Position(xpos, ypos, 0);
					Tile* tile = doodad_buffer_map->getTile(pos);
					if (tile) {
						if (!tile->empty()) {
							fail = true;
							break;
						}
					} else {
						tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));
					}
					int variation = g_brush_manager.GetBrushVariation();
					brush->draw(doodad_buffer_map.get(), tile, &variation);

					doodad_buffer_map->setTile(tile->getPosition(), tile);
					exit = true;
				}
				if (fail) {
					++retries;
					break;
				}
			}
			++object_count;
		}
	} else {
		if (brush->hasCompositeObjects(g_brush_manager.GetBrushVariation()) && random(brush->getTotalChance(g_brush_manager.GetBrushVariation())) <= brush->getCompositeChance(g_brush_manager.GetBrushVariation())) {
			// Composite
			const CompositeTileList& composites = brush->getComposite(g_brush_manager.GetBrushVariation());

			// All placement is valid...

			// Transfer items to the buffer
			for (const auto& composite : composites) {
				Position pos = center_pos + composite.first;
				const ItemVector& items = composite.second;
				Tile* tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));

				for (auto item : items) {
					tile->addItem(item->deepCopy());
				}
				doodad_buffer_map->setTile(tile->getPosition(), tile);
			}
		} else if (brush->hasSingleObjects(g_brush_manager.GetBrushVariation())) {
			Tile* tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(center_pos));
			int variation = g_brush_manager.GetBrushVariation();
			brush->draw(doodad_buffer_map.get(), tile, &variation);

			doodad_buffer_map->setTile(center_pos, tile);
		}
	}
}
