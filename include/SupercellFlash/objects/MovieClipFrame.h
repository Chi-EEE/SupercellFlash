#pragma once

#include <stdint.h>

#include "SupercellFlash/SupercellSWF.h"

#include "SupercellFlash/types/SWFString.hpp"

namespace sc
{
	struct MovieClipFrame
	{
	public:
		MovieClipFrame() {};
		~MovieClipFrame() = default;

	public:
		uint16_t elements_count = 0;
		SWFString label;

	public:
		void load(SupercellSWF& swf) {
			elements_count = swf.stream.read_unsigned_short();
			swf.stream.read_string(label);
		}
		void save(SupercellSWF& swf) const {
			swf.stream.write_unsigned_short(elements_count);
			swf.stream.write_string(label);
		}

		uint8_t tag(SupercellSWF& swf) const {
			return TAG_MOVIE_CLIP_FRAME_2;
		};
	};
}
