
#include "stdafx.h"
#include "AssImpModel.h"
#include "SurfaceIO.h"
#include "GLSLProgram.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

using namespace StE::Resource;
using StE::LLR::Texture2D;
using StE::LLR::VertexBufferObject;
using StE::LLR::VertexArrayObject;

AssImpModel::AssImpModel() {
	bLoaded = false;
}

bool AssImpModel::load_model(const std::string &file_path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file_path,	aiProcess_CalcTangentSpace |
														aiProcess_Triangulate |
														aiProcess_JoinIdenticalVertices |
														aiProcess_SortByPType);

	if (!scene) {
		ste_log_error() << "Couldn't load model " << file_path;
		return false;
	}

	const int vertex_size = sizeof(aiVector3D) * 2 + sizeof(aiVector2D);

	this->vbo = std::unique_ptr<VertexBufferObject>(new VertexBufferObject);

	int vertices = 0;

	for (int i=0; i<scene->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[i];
		int mesh_faces = mesh->mNumFaces;
		auto pre_size = vbo->size();
		start_indices.push_back(pre_size / vertex_size);
		material_indices.push_back(mesh->mMaterialIndex);
		for (int j=0; j<mesh_faces; ++j) {
			const aiFace& face = mesh->mFaces[j];
			for (int k=0; k<3; ++k) {
				aiVector3D pos = mesh->mVertices[face.mIndices[k]];
				aiVector3D uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][face.mIndices[k]] : aiVector3D(.0f, .0f, .0f);
				aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]] : aiVector3D(1.0f, 1.0f, 1.0f);
				vbo->append(&pos, sizeof(aiVector3D));
				vbo->append(&uv, sizeof(aiVector2D));
				vbo->append(&normal, sizeof(aiVector3D));
			}
		}
		int mesh_vertices = mesh->mNumVertices;
		vertices += mesh_vertices;
		indices_sizes.push_back((vbo->size() - pre_size) / vertex_size);
	}
	materials = scene->mNumMaterials;

	for (int i = 0; i < materials; ++i) {
		const aiMaterial* material = scene->mMaterials[i];
		aiString path;  // filename

		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
			std::string dir = { file_path.begin(), std::find_if(file_path.rbegin(), file_path.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };
			std::string texture_name = path.data;
			std::string full_path = dir + texture_name;

			auto tex = SurfaceIO::load_2d(full_path);
			if (tex == nullptr)
				ste_log_warn() << "Couldn't load texture " << file_path;
			textures[i] = std::move(tex);
		}
		if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS ||
			material->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS) {
			std::string dir = { file_path.begin(), std::find_if(file_path.rbegin(), file_path.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };
			std::string texture_name = path.data;
			std::string full_path = dir + texture_name;

			auto tex = SurfaceIO::load_2d(full_path);
			if (tex == nullptr)
				ste_log_warn() << "Couldn't load texture " << file_path;
			normal_maps[i] = std::move(tex);
		}
		if (material->GetTexture(aiTextureType_OPACITY, 0, &path) == AI_SUCCESS) {
			std::string dir = { file_path.begin(), std::find_if(file_path.rbegin(), file_path.rend(), [](char c) { return c == '/' || c == '\\'; }).base() };
			std::string texture_name = path.data;
			std::string full_path = dir + texture_name;

			auto tex = SurfaceIO::load_2d(full_path);
			if (tex == nullptr)
				ste_log_warn() << "Couldn't load texture " << file_path;
			masks[i] = std::move(tex);
		}
	}

	vbo->upload(GL_ARRAY_BUFFER, GL_STATIC_DRAW);

	vao = std::unique_ptr<VertexArrayObject>(new VertexArrayObject);
	vao->bind();
	vbo->bind_vertex_buffer_to_array(0, 0,										 2 * sizeof(aiVector3D) + sizeof(aiVector2D), 3, GL_FLOAT, false);
	vbo->bind_vertex_buffer_to_array(1, sizeof(aiVector3D),						 2 * sizeof(aiVector3D) + sizeof(aiVector2D), 2, GL_FLOAT, false);
	vbo->bind_vertex_buffer_to_array(2, sizeof(aiVector3D) + sizeof(aiVector2D), 2 * sizeof(aiVector3D) + sizeof(aiVector2D), 3, GL_FLOAT, false);

	return (bLoaded = true);
}

void AssImpModel::render(const LLR::GLSLProgram &program) {
	if (!bLoaded)
		return;

	vao->bind();
	VertexArrayObject::enable_vertex_attrib_array(0);
	VertexArrayObject::enable_vertex_attrib_array(1);
	VertexArrayObject::enable_vertex_attrib_array(2);

	for (int i = 0; i < indices_sizes.size(); ++i) {
		auto mat_index = material_indices[i];

		auto& tex = textures[mat_index];
		auto& nm = normal_maps[mat_index];
		auto& mask = masks[mat_index];

		tex != nullptr && tex->is_valid() ? tex->bind(0) : LLR::Texture2D::unbind(0);
		nm != nullptr && nm->is_valid() ? nm->bind(1) : LLR::Texture2D::unbind(1);
		mask != nullptr && mask->is_valid() ? mask->bind(2) : LLR::Texture2D::unbind(2);

		program.set_uniform("has_mask", mask != nullptr);
		program.set_uniform("has_normal_map", nm != nullptr);

		glDrawArrays(GL_TRIANGLES, start_indices[i], indices_sizes[i]);
	}
}
