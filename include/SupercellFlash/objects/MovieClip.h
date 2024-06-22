#pragma once

#include <stdint.h>

#include "math/rect.h"

#include "SupercellFlash/SupercellSWF.h"

#include "SupercellFlash/Tags.h"

#include "SupercellFlash/types/SWFString.hpp"
#include "SupercellFlash/types/SWFContainer.hpp"

#include "SupercellFlash/objects/DisplayObject.h"
#include "SupercellFlash/objects/MovieClipFrame.h"

#include "SupercellFlash/exception/NegativeTagLengthException.h"
#include "SupercellFlash/exception/ObjectLoadingException.h"

namespace sc
{
	struct MovieClipFrameElement
	{
		uint16_t instance_index;
		uint16_t matrix_index = 0xFFFF;
		uint16_t colorTransform_index = 0xFFFF;
	};

	struct DisplayObjectInstance
	{
		enum class BlendMode : uint8_t
		{
			Normal = 0,
			// Normal1 = 1,
			Layer = 2,
			Multiply,
			Screen,
			Lighten,
			Darken,
			Difference,
			Add,
			Subtract,
			Invert,
			Alpha,
			Erase,
			Overlay,
			HardLight,
		};

		uint16_t id;
		BlendMode blend_mode = BlendMode::Normal;
		SWFString name;
	};

	class MovieClip : public DisplayObject
	{
	public:
		MovieClip() = default;
		~MovieClip() = default;

	public:
		SWFVector<MovieClipFrameElement, uint32_t> frame_elements;
		SWFVector<DisplayObjectInstance> instances;
		SWFVector<MovieClipFrame> frames;

	public:
		uint8_t frame_rate = 24;

		uint8_t bank_index = 0;

		bool use_nine_slice = false;
		Rect<float> scaling_grid;

		// TODO
		uint8_t custom_property = 0;

	public:
		bool unknown_flag = false;

	public:
		void load(SupercellSWF& swf, uint8_t tag) {
			id = swf.stream.read_unsigned_short();
			frame_rate = swf.stream.read_unsigned_byte();
			unknown_flag = tag == TAG_MOVIE_CLIP_5;

			if (tag == TAG_MOVIE_CLIP || tag == TAG_MOVIE_CLIP_4)
				throw ObjectLoadingException("TAG_MOVIE_CLIP and TAG_MOVIE_CLIP_4 is unsupported");

			uint16_t frame_count = swf.stream.read_unsigned_short();
			frames.resize(frame_count);

			if (tag == TAG_MOVIE_CLIP_6)
			{
				custom_property = swf.stream.read_unsigned_byte();
			}

			int32_t frame_elements_count = swf.stream.read_int();
			frame_elements.resize(frame_elements_count);

			for (int32_t i = 0; i < frame_elements_count; i++)
			{
				MovieClipFrameElement& element = frame_elements[i];

				element.instance_index = swf.stream.read_unsigned_short();
				element.matrix_index = swf.stream.read_unsigned_short();
				element.colorTransform_index = swf.stream.read_unsigned_short();
			}

			uint16_t instance_count = swf.stream.read_unsigned_short();
			instances.resize(instance_count);

			for (int16_t i = 0; i < instance_count; i++)
			{
				instances[i].id = swf.stream.read_unsigned_short();
			}

			if (tag == TAG_MOVIE_CLIP_3 || tag >= TAG_MOVIE_CLIP_5)
			{
				for (int16_t i = 0; i < instance_count; i++)
				{
					instances[i].blend_mode = (DisplayObjectInstance::BlendMode)swf.stream.read_unsigned_byte();
				}
			}

			for (int16_t i = 0; i < instance_count; i++)
			{
				swf.stream.read_string(instances[i].name);
			}

			uint16_t frames_total = 0;
			while (true)
			{
				uint8_t frame_tag = swf.stream.read_unsigned_byte();
				int32_t frame_tag_length = swf.stream.read_int();

				if (frame_tag == 0)
					break;

				if (frame_tag_length < 0)
					throw NegativeTagLengthException(frame_tag, swf.stream.position());

				switch (frame_tag)
				{
				case TAG_MOVIE_CLIP_FRAME_2:
					frames[frames_total].load(swf);
					frames_total++;
					break;

				case TAG_SCALING_GRID:
					use_nine_slice = true;
					scaling_grid.x = swf.stream.read_twip();
					scaling_grid.y = swf.stream.read_twip();
					scaling_grid.width = swf.stream.read_twip();
					scaling_grid.height = swf.stream.read_twip();
					break;

				case TAG_MATRIX_BANK_INDEX:
					bank_index = swf.stream.read_unsigned_byte();
					break;

				default:
					swf.stream.seek(frame_tag_length, Seek::Add);
					break;
				}
			}
		}
		void save(SupercellSWF& swf) const {
			swf.stream.write_unsigned_short(id);
			swf.stream.write_unsigned_byte(frame_rate);
			swf.stream.write_unsigned_short(frames.size());

			if (swf.save_custom_property)
			{
				swf.stream.write_unsigned_byte(custom_property);
			}

			swf.stream.write_unsigned_int(frame_elements.size());
			for (const MovieClipFrameElement& element : frame_elements)
			{
				swf.stream.write_unsigned_short(element.instance_index);
				swf.stream.write_unsigned_short(element.matrix_index);
				swf.stream.write_unsigned_short(element.colorTransform_index);
			}

			swf.stream.write_short(instances.size());

			for (const DisplayObjectInstance& instance : instances)
			{
				swf.stream.write_unsigned_short(instance.id);
			}

			for (const DisplayObjectInstance& instance : instances)
			{
				swf.stream.write_unsigned_byte((uint8_t)instance.blend_mode);
			}

			for (const DisplayObjectInstance& instance : instances)
			{
				swf.stream.write_string(instance.name);
			}

			if (bank_index != 0)
			{
				swf.stream.write_unsigned_byte(TAG_MATRIX_BANK_INDEX);
				swf.stream.write_int(1);
				swf.stream.write_unsigned_byte(bank_index);
			}

			for (const MovieClipFrame& frame : frames)
			{
				size_t position = swf.stream.write_tag_header(frame.tag(swf));
				frame.save(swf);
				swf.stream.write_tag_final(position);
			}

			if (use_nine_slice)
			{
				swf.stream.write_unsigned_byte(TAG_SCALING_GRID);
				swf.stream.write_int(sizeof(scaling_grid));

				swf.stream.write_int((int)(scaling_grid.x * 20.0f));
				swf.stream.write_int((int)(scaling_grid.y * 20.0f));
				swf.stream.write_int((int)(scaling_grid.width * 20.0f));
				swf.stream.write_int((int)(scaling_grid.height * 20.0f));
			}

			swf.stream.write_tag_flag(TAG_END);
		}

		uint8_t tag(SupercellSWF& swf) const {
			if (swf.save_custom_property)
			{
				return TAG_MOVIE_CLIP_6;
			}

			return unknown_flag ? TAG_MOVIE_CLIP_5 : TAG_MOVIE_CLIP_3;
		}

		bool is_movieclip() const {
			return true;
		}
	};
}
