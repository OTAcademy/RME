#include "rendering/postprocess/post_process_manager.h"
#include "rendering/core/shader_program.h"
#include <spdlog/spdlog.h>

PostProcessManager& PostProcessManager::Instance() {
	static PostProcessManager instance;
	return instance;
}

void PostProcessManager::Register(const std::string& name, const std::string& fragment_source, const std::string& vertex_source) {
	// Check if already registered
	for (const auto& effect : effects) {
		if (effect->name == name) {
			return;
		}
	}

	effects.push_back(std::make_shared<PostProcessEffect>(name, fragment_source, vertex_source));
}

void PostProcessManager::Initialize(const std::string& default_vertex_source) {
	if (initialized) {
		return;
	}

	for (auto& effect : effects) {
		effect->shader = std::make_shared<ShaderProgram>();

		std::string v_source = effect->vertex_source;
		if (v_source.empty()) {
			v_source = default_vertex_source;
		}

		if (!effect->shader->Load(v_source, effect->fragment_source)) {
			spdlog::error("PostProcessManager: Failed to load shader '{}'", effect->name);
			// Fallback? Keep it null or valid object?
			// ShaderProgram state will be invalid but object exists.
		}
	}
	initialized = true;
}

ShaderProgram* PostProcessManager::GetEffect(const std::string& name) {
	if (name.empty() || name == "None") {
		// Assume the first one is "None" if registered in order, or search for it
		for (const auto& effect : effects) {
			if (effect->name == "None") {
				return effect->shader.get();
			}
		}
		if (!effects.empty()) {
			return effects[0]->shader.get();
		}
		return nullptr;
	}

	for (const auto& effect : effects) {
		if (effect->name == name) {
			return effect->shader.get();
		}
	}

	// Fallback to None if specific not found
	for (const auto& effect : effects) {
		if (effect->name == "None") {
			return effect->shader.get();
		}
	}

	// Fallback to any
	if (!effects.empty()) {
		return effects[0]->shader.get();
	}

	return nullptr;
}

std::vector<std::string> PostProcessManager::GetEffectNames() const {
	std::vector<std::string> names;
	for (const auto& effect : effects) {
		names.push_back(effect->name);
	}
	return names;
}
