// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <memory>
#include <vector>
#include <map>

#include "ElementBufferObject.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "Texture2D.h"
#include "GLSLProgram.h"

namespace StE {
namespace Resource {

class Model {
protected:
	struct VertexBufferModel {
		glm::vec3 p;
		glm::vec2 uv;
		glm::vec3 n;

		using descriptor = StE::LLR::VBODescriptorWithTypes<glm::vec3, glm::vec2, glm::vec3>::descriptor;
	};

	using vbo_type = StE::LLR::VertexBufferObject<VertexBufferModel, VertexBufferModel::descriptor>;

private:
	bool bLoaded;
	std::shared_ptr<vbo_type> vbo;
	std::unique_ptr<StE::LLR::VertexArrayObject> vao;
	std::shared_ptr<StE::LLR::ElementBufferObject<>> indices;
	std::map<int, std::unique_ptr<StE::LLR::TextureGeneric>> textures;
	std::map<int, std::unique_ptr<StE::LLR::TextureGeneric>> normal_maps;
	std::map<int, std::unique_ptr<StE::LLR::TextureGeneric>> masks;
	std::vector<int> indices_offset;
	std::vector<int> indices_sizes;
	std::vector<int> material_indices;
	int materials;

public:
	Model();
	virtual ~Model() {}

	bool load_model(const std::string &file_path);

	void render(const LLR::GLSLProgram &program);
};

}
}
