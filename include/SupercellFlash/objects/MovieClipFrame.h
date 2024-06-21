#pragma once

#include <stdint.h>
#include "SupercellFlash/types/SWFString.hpp"

namespace sc
{
	class SupercellSWF;

	struct MovieClipFrame
	{
	public:
		MovieClipFrame() {};
		~MovieClipFrame() = default;

	public:
		uint16_t elements_count = 0;
		SWFString label;

	public:
		void load(SupercellSWF& swf);
		void save(SupercellSWF& swf) const;

		uint8_t tag(SupercellSWF& swf) const;
	};
}
