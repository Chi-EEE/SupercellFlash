#pragma once

#include <stdint.h>

#include "SupercellFlash/types/SWFContainer.hpp"

namespace sc
{
	class SupercellSWF;

	struct ShapeDrawBitmapCommandVertex
	{
		float x;
		float y;

		float u;
		float v;
	};

	class ShapeDrawBitmapCommand
	{
	public:
		ShapeDrawBitmapCommand() = default;
		~ShapeDrawBitmapCommand() = default;

	public:
		uint8_t texture_index = 0;
		SWFVector<ShapeDrawBitmapCommandVertex> vertices;

	public:
		void load(SupercellSWF& swf, uint8_t tag);
		void save(SupercellSWF& swf) const;

		uint8_t tag(SupercellSWF& swf) const;
	};
}
