#pragma once

#include <stdint.h>
#include "math/color.h"

#include "SupercellFlash/SupercellSWF.h"

namespace sc
{
	struct ColorTransform
	{
	public:
		ColorTransform() {};
		~ColorTransform() = default;

	public:
		uint8_t alpha = 255;

		Color<uint8_t> add{ 0, 0, 0 };
		Color<uint8_t> multiply{ 255, 255, 255 };

	public:
		void load(SupercellSWF& swf) {
			add.r = swf.stream.read_unsigned_byte();
			add.g = swf.stream.read_unsigned_byte();
			add.b = swf.stream.read_unsigned_byte();

			alpha = swf.stream.read_unsigned_byte();

			multiply.r = swf.stream.read_unsigned_byte();
			multiply.g = swf.stream.read_unsigned_byte();
			multiply.b = swf.stream.read_unsigned_byte();
		}
		void save(SupercellSWF& swf) const {
			swf.stream.write_unsigned_byte(add.r);
			swf.stream.write_unsigned_byte(add.g);
			swf.stream.write_unsigned_byte(add.b);

			swf.stream.write_unsigned_byte(alpha);

			swf.stream.write_unsigned_byte(multiply.r);
			swf.stream.write_unsigned_byte(multiply.g);
			swf.stream.write_unsigned_byte(multiply.b);
		}

		uint8_t tag(SupercellSWF& swf) const {
			return TAG_COLOR_TRANSFORM;
		}

	public:
		bool operator==(const ColorTransform& color) const {
			if (color.alpha == alpha &&
				color.add.r == add.r &&
				color.add.g == add.g &&
				color.add.b == add.b &&
				color.multiply.r == multiply.r &&
				color.multiply.g == multiply.g &&
				color.multiply.b == multiply.b) {
				return true;
			}

			return false;
		}
	};
}
