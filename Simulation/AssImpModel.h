// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <memory>
#include <vector>
#include <map>

#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "Texture2D.h"
#include "GLSLProgram.h"

namespace StE {
namespace Resource {

class AssImpModel {
private:
	bool bLoaded;
	std::unique_ptr<StE::LLR::VertexBufferObject> vbo;
	std::unique_ptr<StE::LLR::VertexArrayObject> vao;
	std::map<int, std::unique_ptr<StE::LLR::Texture2D>> textures;
	std::map<int, std::unique_ptr<StE::LLR::Texture2D>> normal_maps;
	std::map<int, std::unique_ptr<StE::LLR::Texture2D>> masks;
	std::vector<int> start_indices;
	std::vector<int> indices_sizes;
	std::vector<int> material_indices;
	int materials;

public:
	AssImpModel();
	virtual ~AssImpModel() {}

	bool load_model(const std::string &file_path);

	void render(const LLR::GLSLProgram &program);
};

}
}
