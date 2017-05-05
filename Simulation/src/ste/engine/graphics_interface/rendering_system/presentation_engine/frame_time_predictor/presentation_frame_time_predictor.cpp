
#include <stdafx.hpp>
#include <presentation_frame_time_predictor.hpp>

#include <presentation_frame_time_predictor_model.hpp>

using namespace ste::gl;

struct presentation_frame_time_predictor_impl {
	Kalman::ExtendedKalmanFilter<presentation_frame_time_predictor_model::state<float>> ekf;
	presentation_frame_time_predictor_model::system_model<float> sm;
	presentation_frame_time_predictor_model::measurement_model<float> mm;
};

presentation_frame_time_predictor::presentation_frame_time_predictor(float covariance) {
	presentation_frame_time_predictor_impl *pimpl = new presentation_frame_time_predictor_impl;

	gl::presentation_frame_time_predictor_model::state<float> x;
	x.setZero();
	pimpl->ekf.init(x);

	Kalman::Covariance<gl::presentation_frame_time_predictor_model::state<float>> var;
	var << covariance;
	pimpl->sm.setCovariance(var);


	this->pimpl = pimpl;
}

presentation_frame_time_predictor::~presentation_frame_time_predictor() noexcept {
	auto *pimpl = reinterpret_cast<presentation_frame_time_predictor_impl*>(this->pimpl);
	delete pimpl;
}

void presentation_frame_time_predictor::update(std::uint64_t ns) {
	auto *pimpl = reinterpret_cast<presentation_frame_time_predictor_impl*>(this->pimpl);

	gl::presentation_frame_time_predictor_model::measurement<float> measurement;
	float time = static_cast<float>(ns) / 1e+6f;
	measurement << time;

	pimpl->ekf.predict(pimpl->sm);
	predicted = pimpl->ekf.update(pimpl->mm, pimpl->mm.h(measurement))[0];	
}
