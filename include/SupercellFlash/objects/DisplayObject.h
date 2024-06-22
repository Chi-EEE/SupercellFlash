#pragma once

#include <stdint.h>

#include "SupercellFlash/SupercellSWF.h"

namespace sc
{
	class DisplayObject
	{
	public:
		~DisplayObject() = default;

	public:
		uint16_t id = 0;

	public:
		virtual void load(SupercellSWF& swf, uint8_t tag) = 0;
		virtual void save(SupercellSWF& swf) const = 0;

		bool is_shape() const { return false; };
		bool is_movieclip() const { return false; };
		bool is_modifier() const { return false; };
		bool is_textfield() const { return false; };
	};
}
