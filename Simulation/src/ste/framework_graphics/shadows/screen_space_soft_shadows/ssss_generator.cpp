
#include "stdafx.hpp"
#include "ssss_generator.hpp"

#include "ssss_bilateral_blur_x.hpp"
#include "ssss_bilateral_blur_y.hpp"
#include "ssss_write_penumbras.hpp"

using namespace StE::Graphics;

ssss_generator::ssss_generator(const StEngineControl &ctx,
							   const Scene *scene,
							   const shadowmap_storage *shadows_storage,
							   const ssss_storage *ssss,
							   const deferred_gbuffer *gbuffer) : shadows_storage(shadows_storage),
							   									  ssss(ssss),
																  scene(scene),
																  gbuffer(gbuffer) {
	bilateral_blur_x.load(ctx, this);
	bilateral_blur_y.load(ctx, this);
	write_penumbras.load(ctx, this);
}

ssss_generator::~ssss_generator() {}
