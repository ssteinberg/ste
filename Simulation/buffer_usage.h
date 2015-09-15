// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {
namespace BufferUsage {

enum buffer_usage {
	BufferUsageNone = 0,
	BufferUsageMapRead = GL_MAP_READ_BIT,
	BufferUsageMapWrite = GL_MAP_WRITE_BIT,
	BufferUsageMapPersistent = GL_MAP_PERSISTENT_BIT,
	BufferUsageMapCoherent = GL_MAP_COHERENT_BIT,
	BufferUsageDynamic = GL_DYNAMIC_STORAGE_BIT,
};

enum buffer_mapping {
	BufferMapNone = 0,
	BufferMapInvalidateRange = GL_MAP_INVALIDATE_RANGE_BIT,
	BufferMapInvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT,
	BufferMapFlushExplicit = GL_MAP_FLUSH_EXPLICIT_BIT,
	BufferMapPersistent = GL_MAP_PERSISTENT_BIT,
	BufferMapCoherent = GL_MAP_COHERENT_BIT,
	BufferMapUnsynchronized = GL_MAP_UNSYNCHRONIZED_BIT,
};

}
}
}
