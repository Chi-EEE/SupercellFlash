#pragma once

#include <stdint.h>

#include "SupercellFlash/SupercellSWF.h"

#include "SupercellFlash/objects/DisplayObject.h"
#include "SupercellFlash/Tags.h"

namespace sc
{
	class MovieClipModifier : public DisplayObject
	{
	public:
		MovieClipModifier() = default;
		~MovieClipModifier() = default;

	public:
		enum class Type : uint8_t
		{
			Mask = TAG_MOVIE_CLIP_MODIFIER,
			Masked = TAG_MOVIE_CLIP_MODIFIER_2,
			Unmasked = TAG_MOVIE_CLIP_MODIFIER_3,
		};

	public:
		Type type = Type::Mask;

	public:
		void load(SupercellSWF& swf, uint8_t tag) {
			id = swf.stream.read_unsigned_short();
			type = (Type)tag;
		}

		void save(SupercellSWF& swf) const {
			swf.stream.write_unsigned_short(id);
		}

		uint8_t tag(SupercellSWF& swf) const {
			return (uint8_t)type;
		}

		bool is_modifier() const {
			return true;
		}
	};
}
