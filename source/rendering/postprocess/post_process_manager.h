#ifndef RME_RENDERING_POSTPROCESS_MANAGER_H
#define RME_RENDERING_POSTPROCESS_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

class ShaderProgram;

struct PostProcessEffect {
	std::string name;
	std::string fragment_source;
	std::string vertex_source;
	std::shared_ptr<ShaderProgram> shader;

	// Constructor for easy registration
	PostProcessEffect(std::string n, std::string frag, std::string vert = "") : name(n), fragment_source(frag), vertex_source(vert) { }
};

class PostProcessManager {
public:
	static PostProcessManager& Instance();

	void Register(const std::string& name, const std::string& fragment_source, const std::string& vertex_source = "");
	void Initialize(const std::string& default_vertex_source); // Compiles all registered shaders

	// Returns the shader program for the given name.
	// If not found, returns the first available shader (usually "None") or nullptr.
	ShaderProgram* GetEffect(const std::string& name);

	// Returns a list of all registered effect names (for UI)
	std::vector<std::string> GetEffectNames() const;

private:
	PostProcessManager() = default;

	// We maintain insertion order for UI consistency usually,
	// but a map by name is good for lookup.
	// Let's keep a vector for order and a map for lookup, or just search the vector (small N).
	std::vector<std::shared_ptr<PostProcessEffect>> effects;

	bool initialized = false;
};

#endif
