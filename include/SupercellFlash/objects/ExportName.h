#pragma once

#include <stdint.h>
#include "SupercellFlash/types/SWFString.hpp"

namespace sc {
	struct ExportName
	{
		ExportName() = default;
		virtual ~ExportName() = default;

		SWFString name;
		uint16_t id;
	};
}
