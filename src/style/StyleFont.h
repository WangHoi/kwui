#pragma once

namespace style {

struct FontMetrics {
	float ascent = 0;
	float descent = 0;
	float line_gap = 0;
	float cap_height = 0;
	float x_height = 0;
	float underline_offset = 0; // positive to above baseline
	float underline_thickness = 0;
	float line_through_offset = 0; // positive to above baseline
	float line_through_thickness = 0;

	inline float lineHeight() const
	{
		return ascent + descent + line_gap;
	}
	inline float baseline() const
	{
		return 0.5f * line_gap + ascent;
	}
};

}
