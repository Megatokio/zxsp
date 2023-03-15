#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


using Sample = float;


class StereoSample
{
public:
	Sample left	 = 0;
	Sample right = 0;

	StereoSample() noexcept									= default;
	StereoSample(const StereoSample& q) noexcept			= default;
	StereoSample& operator=(const StereoSample& q) noexcept = default;

	StereoSample(Sample m) noexcept : left(m), right(m) {}
	StereoSample(Sample l, Sample r) noexcept : left(l), right(r) {}

	operator Sample() const noexcept { return (left + right) * 0.5f; }

	StereoSample& operator+=(const StereoSample& q) noexcept
	{
		left += q.left;
		right += q.right;
		return *this;
	}

	StereoSample& operator-=(const StereoSample& q) noexcept
	{
		left -= q.left;
		right -= q.right;
		return *this;
	}

	StereoSample& operator*=(float f) noexcept
	{
		left *= f;
		right *= f;
		return *this;
	}

	StereoSample operator*(float f) const noexcept { return StereoSample(left * f, right * f); }
};


/*









  
  
*/
