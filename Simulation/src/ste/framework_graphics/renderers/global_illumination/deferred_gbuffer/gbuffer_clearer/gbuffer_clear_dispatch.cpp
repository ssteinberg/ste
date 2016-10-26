
#include "stdafx.hpp"
#include "gbuffer_clear_dispatch.hpp"

using namespace StE::Graphics;

void gbuffer_clear_dispatch::dispatch() const {
	gbuffer->clear();
}
