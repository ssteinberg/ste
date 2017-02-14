
#include <stdafx.hpp>
#include <sampler.hpp>

using namespace StE::Core;

std::unique_ptr<sampler> sampler::_sampler_nearest{ nullptr };
std::unique_ptr<sampler> sampler::_sampler_linear{ nullptr };
std::unique_ptr<sampler> sampler::_sampler_anisotropic_linear{ nullptr };
std::unique_ptr<sampler> sampler::_sampler_nearest_clamp{ nullptr };
std::unique_ptr<sampler> sampler::_sampler_linear_clamp{ nullptr };
std::unique_ptr<sampler> sampler::_sampler_anisotropic_linear_clamp{ nullptr };

std::unique_ptr<sampler_mipmapped> sampler_mipmapped::_mipmapped_sampler_nearest{ nullptr };
std::unique_ptr<sampler_mipmapped> sampler_mipmapped::_mipmapped_sampler_linear{ nullptr };
std::unique_ptr<sampler_mipmapped> sampler_mipmapped::_mipmapped_sampler_anisotropic_linear{ nullptr };
std::unique_ptr<sampler_mipmapped> sampler_mipmapped::_mipmapped_sampler_nearest_clamp{ nullptr };
std::unique_ptr<sampler_mipmapped> sampler_mipmapped::_mipmapped_sampler_linear_clamp{ nullptr };
std::unique_ptr<sampler_mipmapped> sampler_mipmapped::_mipmapped_sampler_anisotropic_linear_clamp{ nullptr };

sampler *sampler::sampler_nearest() {
	if (_sampler_nearest.get() == nullptr)
		_sampler_nearest = std::make_unique<sampler>(texture_filtering::Nearest, texture_filtering::Nearest);
	return _sampler_nearest.get();
}

sampler *sampler::sampler_linear() {
	if (_sampler_linear.get() == nullptr)
		_sampler_linear = std::make_unique<sampler>(texture_filtering::Linear, texture_filtering::Linear);
	return _sampler_linear.get();
}

sampler *sampler::sampler_anisotropic_linear() {
	if (_sampler_anisotropic_linear.get()==nullptr)
		_sampler_anisotropic_linear = std::make_unique<sampler>(texture_filtering::Linear, texture_filtering::Linear, 16);
	return _sampler_anisotropic_linear.get();
}

sampler *sampler::sampler_nearest_clamp() {
	if (_sampler_nearest_clamp.get() == nullptr) {
		_sampler_nearest_clamp = std::make_unique<sampler>(texture_filtering::Nearest, texture_filtering::Nearest, texture_wrap_mode::ClampToEdge, texture_wrap_mode::ClampToEdge);
		_sampler_nearest_clamp->set_wrap_r(texture_wrap_mode::ClampToEdge);
	}
	return _sampler_nearest_clamp.get();
}

sampler *sampler::sampler_linear_clamp() {
	if (_sampler_linear_clamp.get() == nullptr) {
		_sampler_linear_clamp = std::make_unique<sampler>(texture_filtering::Linear, texture_filtering::Linear, texture_wrap_mode::ClampToEdge, texture_wrap_mode::ClampToEdge);
		_sampler_linear_clamp->set_wrap_r(texture_wrap_mode::ClampToEdge);
	}
	return _sampler_linear_clamp.get();
}

sampler *sampler::sampler_anisotropic_linear_clamp() {
	if (_sampler_anisotropic_linear_clamp.get() == nullptr) {
		_sampler_anisotropic_linear_clamp = std::make_unique<sampler>(texture_filtering::Linear, texture_filtering::Linear, texture_wrap_mode::ClampToEdge, texture_wrap_mode::ClampToEdge, 16);
		_sampler_anisotropic_linear_clamp->set_wrap_r(texture_wrap_mode::ClampToEdge);
	}
	return _sampler_anisotropic_linear_clamp.get();
}


sampler_mipmapped *sampler_mipmapped::mipmapped_sampler_nearest() {
	if (_mipmapped_sampler_nearest.get() == nullptr) {
		_mipmapped_sampler_nearest = std::make_unique<sampler_mipmapped>(texture_filtering::Nearest, texture_filtering::Nearest);
		_mipmapped_sampler_nearest->set_mipmap_filter(texture_filtering::Nearest);
	}
	return _mipmapped_sampler_nearest.get();
}

sampler_mipmapped *sampler_mipmapped::mipmapped_sampler_linear() {
	if (_mipmapped_sampler_linear.get() == nullptr) {
		_mipmapped_sampler_linear = std::make_unique<sampler_mipmapped>(texture_filtering::Linear, texture_filtering::Linear);
		_mipmapped_sampler_linear->set_mipmap_filter(texture_filtering::Nearest);
	}
	return _mipmapped_sampler_linear.get();
}

sampler_mipmapped *sampler_mipmapped::mipmapped_sampler_anisotropic_linear() {
	if (_mipmapped_sampler_anisotropic_linear.get() == nullptr) {
		_mipmapped_sampler_anisotropic_linear = std::make_unique<sampler_mipmapped>(texture_filtering::Linear, texture_filtering::Linear, 16);
		_mipmapped_sampler_anisotropic_linear->set_mipmap_filter(texture_filtering::Nearest);
	}
	return _mipmapped_sampler_anisotropic_linear.get();
}

sampler_mipmapped *sampler_mipmapped::mipmapped_sampler_nearest_clamp() {
	if (_mipmapped_sampler_nearest_clamp.get() == nullptr) {
		_mipmapped_sampler_nearest_clamp = std::make_unique<sampler_mipmapped>(texture_filtering::Nearest, texture_filtering::Nearest, texture_wrap_mode::ClampToEdge, texture_wrap_mode::ClampToEdge);
		_mipmapped_sampler_nearest_clamp->set_mipmap_filter(texture_filtering::Nearest);
		_mipmapped_sampler_nearest_clamp->set_wrap_r(texture_wrap_mode::ClampToEdge);
	}
	return _mipmapped_sampler_nearest_clamp.get();
}

sampler_mipmapped *sampler_mipmapped::mipmapped_sampler_linear_clamp() {
	if (_mipmapped_sampler_linear_clamp.get() == nullptr) {
		_mipmapped_sampler_linear_clamp = std::make_unique<sampler_mipmapped>(texture_filtering::Linear, texture_filtering::Linear, texture_wrap_mode::ClampToEdge, texture_wrap_mode::ClampToEdge);
		_mipmapped_sampler_linear_clamp->set_mipmap_filter(texture_filtering::Nearest);
		_mipmapped_sampler_linear_clamp->set_wrap_r(texture_wrap_mode::ClampToEdge);
	}
	return _mipmapped_sampler_linear_clamp.get();
}

sampler_mipmapped *sampler_mipmapped::mipmapped_sampler_anisotropic_linear_clamp() {
	if (_mipmapped_sampler_anisotropic_linear_clamp.get() == nullptr) {
		_mipmapped_sampler_anisotropic_linear_clamp = std::make_unique<sampler_mipmapped>(texture_filtering::Linear, texture_filtering::Linear, texture_wrap_mode::ClampToEdge, texture_wrap_mode::ClampToEdge, 16);
		_mipmapped_sampler_anisotropic_linear_clamp->set_mipmap_filter(texture_filtering::Nearest);
		_mipmapped_sampler_anisotropic_linear_clamp->set_wrap_r(texture_wrap_mode::ClampToEdge);
	}
	return _mipmapped_sampler_anisotropic_linear_clamp.get();
}
