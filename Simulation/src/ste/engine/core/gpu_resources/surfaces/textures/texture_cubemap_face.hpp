// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Core {

enum class CubeMapFace {
	CubeMapFaceNone = 0,
	CubeMapFaceRight = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	CubeMapFaceLeft = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	CubeMapFaceTop = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	CubeMapFaceBottom = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	CubeMapFaceNear = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	CubeMapFaceFar = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

}
}
