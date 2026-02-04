#ifndef RME_RENDERING_DRAWERS_ENTITIES_CREATURE_NAME_DRAWER_H_
#define RME_RENDERING_DRAWERS_ENTITIES_CREATURE_NAME_DRAWER_H_

#include "app/definitions.h"
#include <string>
#include <vector>

struct RenderView;
struct NVGcontext;
class Creature;

#include "map/position.h"

struct CreatureLabel {
	Position pos;
	std::string name;
	const Creature* creature;
};

class CreatureNameDrawer {
public:
	CreatureNameDrawer();
	~CreatureNameDrawer();

	void clear();
	void addLabel(const Position& pos, const std::string& name, const Creature* c);
	void draw(NVGcontext* vg, const RenderView& view);

private:
	std::vector<CreatureLabel> labels;
};

#endif
