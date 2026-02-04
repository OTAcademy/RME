#include "rendering/postprocess/post_process_manager.h"
#include "rendering/core/shader_program.h"
#include <spdlog/spdlog.h>
#include <algorithm>

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
			effect->shader = nullptr; // Ensure it's null so we can prune it
		}
	}

	// Remove invalid effects (where load failed)
	effects.erase(std::remove_if(effects.begin(), effects.end(), [](const std::shared_ptr<PostProcessEffect>& effect) {
					  return !effect->shader || !effect->shader->IsValid();
				  }),
				  effects.end());

	initialized = true;
}

ShaderProgram* PostProcessManager::GetEffect(const std::string& name) {
	auto find_shader = [this](const std::string& effect_name) -> ShaderProgram* {
		for (const auto& effect : effects) {
			if (effect->name == effect_name) {
				return effect->shader.get();
			}
		}
		return nullptr;
	};

	ShaderProgram* shader = nullptr;

	// Try to find the requested shader, or "None" if the name is empty
	std::string target_name = name.empty() ? ShaderNames::NONE : name;
	shader = find_shader(target_name);

	// If the requested shader was not found, fall back to "None"
	if (!shader && target_name != ShaderNames::NONE) {
		shader = find_shader(ShaderNames::NONE);
	}

	// If still no shader, fall back to the first one available
	if (!shader && !effects.empty()) {
		return effects[0]->shader.get();
	}

	return shader;
}

std::vector<std::string> PostProcessManager::GetEffectNames() const {
	std::vector<std::string> names;
	for (const auto& effect : effects) {
		names.push_back(effect->name);
	}
	return names;
}
