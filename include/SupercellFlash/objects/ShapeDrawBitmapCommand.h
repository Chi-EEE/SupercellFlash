#pragma once

#include <stdint.h>

#include "SupercellFlash/SupercellSWF.h"

#include "SupercellFlash/types/SWFContainer.hpp"

namespace sc
{
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
		void load(SupercellSWF& swf, uint8_t tag) {
			texture_index = swf.stream.read_unsigned_byte();

			uint8_t vertices_count = tag == 4 ? 4 : swf.stream.read_unsigned_byte();
			vertices.resize(vertices_count);

			for (ShapeDrawBitmapCommandVertex& vertex : vertices)
			{
				vertex.x = swf.stream.read_twip();
				vertex.y = swf.stream.read_twip();
			}

			for (ShapeDrawBitmapCommandVertex& vertex : vertices)
			{
				vertex.u = (float)swf.stream.read_unsigned_short() / 65535.0f;
				vertex.v = (float)swf.stream.read_unsigned_short() / 65535.0f;
			}
		}
		void save(SupercellSWF& swf) const {
			uint8_t verticesCount = static_cast<uint8_t>(vertices.size());

			swf.stream.write_unsigned_byte(texture_index);
			swf.stream.write_unsigned_byte(verticesCount);

			for (const ShapeDrawBitmapCommandVertex& vertex : vertices)
			{
				swf.stream.write_twip(vertex.x);
				swf.stream.write_twip(vertex.y);
			}

			for (const ShapeDrawBitmapCommandVertex& vertex : vertices)
			{
				swf.stream.write_unsigned_short((uint16_t)(vertex.u * 65535.0f));
				swf.stream.write_unsigned_short((uint16_t)(vertex.v * 65535.0f));
			}
		}

		uint8_t tag(SupercellSWF& swf) const {
			return TAG_SHAPE_DRAW_BITMAP_COMMAND_3;
		}
	};
}
