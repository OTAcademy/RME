#ifndef RME_RENDERING_CORE_SPRITE_INSTANCE_H_
#define RME_RENDERING_CORE_SPRITE_INSTANCE_H_

/**
 * Per-sprite instance data for instanced rendering.
 * Layout matches vertex attributes in sprite_batch.vert.
 *
 * 64 bytes per instance (aligned for GPU efficiency).
 */
struct SpriteInstance {
	float x, y, w, h; // Byte 0-15:  Screen rect (Location 2)
	float u_min, v_min, u_max, v_max; // Byte 16-31: UV rect (Location 3)
	float r, g, b, a; // Byte 32-47: Tint color (Location 4)
	float atlas_layer; // Byte 48-51: Texture layer (Location 5)
	float _pad1, _pad2, _pad3; // Byte 52-63: Padding
};
static_assert(sizeof(SpriteInstance) == 64, "SpriteInstance must be 64 bytes");

#endif
