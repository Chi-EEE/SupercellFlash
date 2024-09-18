#pragma once

#include <stdint.h>
#include "math/matrix2x3.h"

namespace sc
{
	class SupercellSWF;

	struct Matrix2D : public Matrix2x3<float>
	{
	public:
		Matrix2D() = default;
		virtual ~Matrix2D() = default;

	public:
		void load(SupercellSWF& swf, uint8_t tag);
		void save(SupercellSWF& swf) const;

		virtual uint8_t tag(SupercellSWF& swf) const;

	public:
		bool operator==(const Matrix2D& matrix) const;
	};
}
