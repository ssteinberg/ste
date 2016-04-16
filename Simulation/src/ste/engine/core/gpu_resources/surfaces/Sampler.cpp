
#include "stdafx.hpp"
#include "Sampler.hpp"

using namespace StE::Core;

std::unique_ptr<Sampler> Sampler::sampler_nearest{ nullptr };
std::unique_ptr<Sampler> Sampler::sampler_linear{ nullptr };
std::unique_ptr<Sampler> Sampler::sampler_anisotropic_linear{ nullptr };
std::unique_ptr<Sampler> Sampler::sampler_nearest_clamp{ nullptr };
std::unique_ptr<Sampler> Sampler::sampler_linear_clamp{ nullptr };
std::unique_ptr<Sampler> Sampler::sampler_anisotropic_linear_clamp{ nullptr };

std::unique_ptr<SamplerMipmapped> SamplerMipmapped::mipmapped_sampler_nearest{ nullptr };
std::unique_ptr<SamplerMipmapped> SamplerMipmapped::mipmapped_sampler_linear{ nullptr };
std::unique_ptr<SamplerMipmapped> SamplerMipmapped::mipmapped_sampler_anisotropic_linear{ nullptr };
std::unique_ptr<SamplerMipmapped> SamplerMipmapped::mipmapped_sampler_nearest_clamp{ nullptr };
std::unique_ptr<SamplerMipmapped> SamplerMipmapped::mipmapped_sampler_linear_clamp{ nullptr };
std::unique_ptr<SamplerMipmapped> SamplerMipmapped::mipmapped_sampler_anisotropic_linear_clamp{ nullptr };

Sampler *Sampler::SamplerNearest() {
	if (sampler_nearest.get() == nullptr)
		sampler_nearest = std::make_unique<Sampler>(TextureFiltering::Nearest, TextureFiltering::Nearest);
	return sampler_nearest.get();
}

Sampler *Sampler::SamplerLinear() {
	if (sampler_linear.get() == nullptr)
		sampler_linear = std::make_unique<Sampler>(TextureFiltering::Linear, TextureFiltering::Linear);
	return sampler_linear.get();
}

Sampler *Sampler::SamplerAnisotropicLinear() {
	if (sampler_anisotropic_linear.get()==nullptr)
		sampler_anisotropic_linear = std::make_unique<Sampler>(TextureFiltering::Linear, TextureFiltering::Linear, 16);
	return sampler_anisotropic_linear.get();
}

Sampler *Sampler::SamplerNearestClamp() {
	if (sampler_nearest_clamp.get() == nullptr)
		sampler_nearest_clamp = std::make_unique<Sampler>(TextureFiltering::Nearest, TextureFiltering::Nearest, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge);
	return sampler_nearest_clamp.get();
}

Sampler *Sampler::SamplerLinearClamp() {
	if (sampler_linear_clamp.get() == nullptr)
		sampler_linear_clamp = std::make_unique<Sampler>(TextureFiltering::Linear, TextureFiltering::Linear, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge);
	return sampler_linear_clamp.get();
}

Sampler *Sampler::SamplerAnisotropicLinearClamp() {
	if (sampler_anisotropic_linear_clamp.get() == nullptr)
		sampler_anisotropic_linear_clamp = std::make_unique<Sampler>(TextureFiltering::Linear, TextureFiltering::Linear, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge, 16);
	return sampler_anisotropic_linear_clamp.get();
}


SamplerMipmapped *SamplerMipmapped::MipmappedSamplerNearest() {
	if (mipmapped_sampler_nearest.get() == nullptr)
		mipmapped_sampler_nearest = std::make_unique<SamplerMipmapped>(TextureFiltering::Nearest, TextureFiltering::Nearest);
	return mipmapped_sampler_nearest.get();
}

SamplerMipmapped *SamplerMipmapped::MipmappedSamplerLinear() {
	if (mipmapped_sampler_linear.get() == nullptr)
		mipmapped_sampler_linear = std::make_unique<SamplerMipmapped>(TextureFiltering::Linear, TextureFiltering::Linear);
	return mipmapped_sampler_linear.get();
}

SamplerMipmapped *SamplerMipmapped::MipmappedSamplerAnisotropicLinear() {
	if (mipmapped_sampler_anisotropic_linear.get() == nullptr)
		mipmapped_sampler_anisotropic_linear = std::make_unique<SamplerMipmapped>(TextureFiltering::Linear, TextureFiltering::Linear, 16);
	return mipmapped_sampler_anisotropic_linear.get();
}

SamplerMipmapped *SamplerMipmapped::MipmappedSamplerNearestClamp() {
	if (mipmapped_sampler_nearest_clamp.get() == nullptr)
		mipmapped_sampler_nearest_clamp = std::make_unique<SamplerMipmapped>(TextureFiltering::Nearest, TextureFiltering::Nearest, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge);
	return mipmapped_sampler_nearest_clamp.get();
}

SamplerMipmapped *SamplerMipmapped::MipmappedSamplerLinearClamp() {
	if (mipmapped_sampler_linear_clamp.get() == nullptr)
		mipmapped_sampler_linear_clamp = std::make_unique<SamplerMipmapped>(TextureFiltering::Linear, TextureFiltering::Linear, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge);
	return mipmapped_sampler_linear_clamp.get();
}

SamplerMipmapped *SamplerMipmapped::MipmappedSamplerAnisotropicLinearClamp() {
	if (mipmapped_sampler_anisotropic_linear_clamp.get() == nullptr)
		mipmapped_sampler_anisotropic_linear_clamp = std::make_unique<SamplerMipmapped>(TextureFiltering::Linear, TextureFiltering::Linear, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge, 16);
	return mipmapped_sampler_anisotropic_linear_clamp.get();
}
