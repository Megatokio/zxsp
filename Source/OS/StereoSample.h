#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

typedef float		 Sample;
typedef const Sample cSample;
class StereoSample;
typedef const StereoSample cStereoSample;


class StereoSample
{
public:
	Sample left, right;

	StereoSample() noexcept : left(0), right(0) {}
	StereoSample(Sample m) noexcept : left(m), right(m) {}
	StereoSample(Sample l, Sample r) noexcept : left(l), right(r) {}
	StereoSample(cStereoSample& q) noexcept = default;

	StereoSample& operator=(Sample q) noexcept
	{
		left = right = q;
		return *this;
	}
	StereoSample& operator=(cStereoSample& q) noexcept = default;
	StereoSample& operator+=(Sample q) noexcept
	{
		left += q;
		right += q;
		return *this;
	}
	StereoSample& operator+=(cStereoSample& q) noexcept
	{
		left += q.left;
		right += q.right;
		return *this;
	}
	StereoSample& operator-=(Sample q) noexcept
	{
		left -= q;
		right -= q;
		return *this;
	}
	StereoSample& operator-=(cStereoSample& q) noexcept
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
	bool		 operator>=(Sample q) const noexcept { return mono() >= q; }
	bool		 operator<=(Sample q) const noexcept { return mono() <= q; }
	bool		 operator<(Sample q) const noexcept { return mono() < q; }

	Sample mono() const noexcept { return (left + right) / 2.0f; }
};
