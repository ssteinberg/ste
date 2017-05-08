//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#define EIGEN_NO_DEBUG

#include <kalman/LinearizedSystemModel.hpp>
#include <kalman/LinearizedMeasurementModel.hpp>

#include <kalman/ExtendedKalmanFilter.hpp>

namespace ste {
namespace gl {

namespace presentation_frame_time_predictor_model {

template<typename T>
class state : public Kalman::Vector<T, 1> {
public:
	KALMAN_VECTOR(state, T, 1);

	static constexpr size_t TIME = 0;

	T time() const { return (*this)[TIME]; }
	T& time() { return (*this)[TIME]; }
};

template<typename T, template<class> class CovarianceBase = Kalman::StandardBase>
class system_model : public Kalman::LinearizedSystemModel<
	state<T>,
	Kalman::Vector<T, 1>,
	CovarianceBase
>
{
public:
	using S = state<T>;
	using C = Kalman::Vector<T, 1>;

	S f(const S& x, const C&) const override final {
		return x;
	}

protected:
	void updateJacobians(const S&, const C&) override final {
		// F = df/dx (Jacobian of state transition w.r.t. the state)
		this->F.setZero();

		// partial derivative of x.time() w.r.t. x.time()
		this->F(S::TIME, S::TIME) = 1;

		// W = df/dw (Jacobian of state transition w.r.t. the noise)
		this->W.setIdentity();
	}
};

template<typename T>
class measurement : public Kalman::Vector<T, 1> {
public:
	KALMAN_VECTOR(measurement, T, 1);

	// Time
	static constexpr size_t TIME = 0;

	T time()  const { return (*this)[TIME]; }
	T& time() { return (*this)[TIME]; }
};

template<typename T, template<class> class CovarianceBase = Kalman::StandardBase>
class measurement_model : public Kalman::LinearizedMeasurementModel<
	state<T>,
	measurement<T>,
	CovarianceBase
>
{
public:
	using S = state<T>;
	using M = measurement<T>;

	measurement_model() {
		// Setup jacobians. As these are static, we can define them once
		// and do not need to update them dynamically
		this->H.setIdentity();
		this->V.setIdentity();
	}

	/**
	* @brief Definition of (possibly non-linear) measurement function
	*
	* This function maps the system state to the measurement that is expected
	* to be received from the sensor assuming the system is currently in the
	* estimated state.
	*
	* @param [in] x The system state in current time-step
	* @returns The (predicted) sensor measurement for the system state
	*/
	M h(const S& x) const override final {
		M m;
		m.time() = x.time();
		return m;
	}
};

}

}
}
