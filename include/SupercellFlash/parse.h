#pragma once

#include <filesystem>

#include <SupercellCompression/Zstd/Compressor.h>

#include "SupercellSWF.h"

#include "objects/SWFTexture.h"
#include "objects/Shape.h"
#include "objects/MovieClip.h"
#include "objects/TextField.h"
#include "objects/MovieClipModifier.h"
#include "objects/MatrixBank.h"

namespace sc {
	inline SupercellSWF load(const std::filesystem::path& filePath);
	inline bool load_internal(SupercellSWF& swf, const std::filesystem::path& filePath, bool is_texture);
	inline void save(SupercellSWF& swf, const std::filesystem::path& filePath, SWFStream::Signature signature);
	inline void save_internal(SupercellSWF& swf, bool is_texture, bool is_lowres);
	inline bool _load_tags(SupercellSWF& swf);
	inline void _save_tags(SupercellSWF& swf);
	inline void _save_textures(SupercellSWF& swf, bool has_data, bool is_lowres);

	inline SupercellSWF load(const std::filesystem::path& filePath) {
		SupercellSWF swf;
		swf.current_file = filePath;

		swf.use_external_texture = load_internal(swf, filePath, false);

		if (swf.use_external_texture)
		{
			std::filesystem::path basename = filePath.stem();
			std::filesystem::path dirname = filePath.parent_path();

			std::filesystem::path multi_resolution_path = dirname / std::filesystem::path(basename).concat(swf.multi_resolution_suffix.string()).concat("_tex.sc");
			std::filesystem::path low_resolution_path = dirname / std::filesystem::path(basename).concat(swf.low_resolution_suffix.string()).concat("_tex.sc");
			std::filesystem::path common_file_path = dirname / std::filesystem::path(basename).concat("_tex.sc");

			if (swf.low_memory_usage_mode && swf.use_low_resolution && std::filesystem::exists(low_resolution_path))
			{
				load_internal(swf, low_resolution_path, true);
			}
			else if (swf.use_multi_resolution && std::filesystem::exists(multi_resolution_path))
			{
				load_internal(swf, multi_resolution_path, true);
			}
			else if (std::filesystem::exists(common_file_path))
			{
				load_internal(swf, common_file_path, true);
			}
			else
			{
				throw FileExistException(common_file_path);
			}
		}

		swf.stream.clear();

		return swf;
	}

	inline bool load_internal(SupercellSWF& swf, const std::filesystem::path& filePath, bool is_texture) {
		swf.stream.open_file(filePath);

		// Reading .sc file
		if (!is_texture)
		{
			uint16_t shapes_count = swf.stream.read_unsigned_short();
			swf.shapes.resize(shapes_count);

			uint16_t movie_clips_count = swf.stream.read_unsigned_short();
			swf.movieclips.resize(movie_clips_count);

			uint16_t textures_count = swf.stream.read_unsigned_short();
			swf.textures.resize(textures_count);

			uint16_t textfield_count = swf.stream.read_unsigned_short();
			swf.textfields.resize(textfield_count);

			uint16_t matrices_count = swf.stream.read_unsigned_short();
			uint16_t colors_count = swf.stream.read_unsigned_short();
			swf.matrixBanks.resize(1, MatrixBank(matrices_count, colors_count));

			swf.stream.seek(5, Seek::Add); // unused

			uint16_t exports_count = swf.stream.read_unsigned_short();
			swf.exports.resize(exports_count);

			for (ExportName& export_name : swf.exports)
			{
				export_name.id = swf.stream.read_unsigned_short();
			}

			for (ExportName& export_name : swf.exports)
			{
				swf.stream.read_string(export_name.name);
			}
		}

		return _load_tags(swf);
	}

	inline void save(SupercellSWF& swf, const std::filesystem::path& filePath, SWFStream::Signature signature) {
		swf.current_file = filePath;
		if (swf.matrixBanks.size() == 0) {
			swf.matrixBanks.resize(1);
		}

		save_internal(swf, false, false);
		swf.stream.save_file(filePath, signature);

		if (swf.use_external_texture) {
			std::filesystem::path basename = filePath.stem();
			std::filesystem::path dirname = filePath.parent_path();

			std::filesystem::path multi_resolution_path = dirname / std::filesystem::path(basename).concat(swf.multi_resolution_suffix.string()).concat("_tex.sc");
			std::filesystem::path low_resolution_path = dirname / std::filesystem::path(basename).concat(swf.low_resolution_suffix.string()).concat("_tex.sc");
			std::filesystem::path common_file_path = dirname / std::filesystem::path(basename).concat("_tex.sc");

			save_internal(swf, true, false);
			swf.stream.save_file(swf.use_multi_resolution ? multi_resolution_path : common_file_path, signature);

			if (swf.use_low_resolution)
			{
				save_internal(swf, true, true);
				swf.stream.save_file(low_resolution_path, signature);
			}
		}
	}

	inline void save_internal(SupercellSWF& swf, bool is_texture, bool is_lowres) {
		if (!is_texture)
		{
			swf.stream.write_unsigned_short(swf.shapes.size());
			swf.stream.write_unsigned_short(swf.movieclips.size());
			swf.stream.write_unsigned_short(swf.textures.size());
			swf.stream.write_unsigned_short(swf.textfields.size());

			swf.stream.write_unsigned_short(swf.matrixBanks[0].matrices.size());
			swf.stream.write_unsigned_short(swf.matrixBanks[0].color_transforms.size());

			// unused 5 bytes
			swf.stream.write_unsigned_byte(0);
			swf.stream.write_int(0);

			swf.stream.write_unsigned_short(swf.exports.size());

			for (const ExportName& export_name : swf.exports)
			{
				swf.stream.write_unsigned_short(export_name.id);
			}

			for (const ExportName& export_name : swf.exports)
			{
				swf.stream.write_string(export_name.name);
			}

			_save_tags(swf);
		}
		else
		{
			_save_textures(swf, is_texture, is_lowres);
		}

		swf.stream.write_tag_flag(TAG_END);
	}

	inline bool _load_tags(SupercellSWF& swf) {
		bool has_external_texture = false;

		uint16_t shapes_loaded = 0;
		uint16_t movieclips_loaded = 0;
		uint16_t textures_loaded = 0;
		uint16_t textfields_loaded = 0;
		uint8_t banks_loaded = 0;
		uint16_t matrices_loaded = 0;
		uint16_t colors_loaded = 0;
		uint16_t modifiers_loaded = 0;

		while (true)
		{
			uint8_t tag = swf.stream.read_unsigned_byte();
			int32_t tag_length = swf.stream.read_int();

			if (tag == TAG_END)
				break;

			if (tag_length < 0)
				throw NegativeTagLengthException(tag, swf.stream.position());

			switch (tag)
			{
			case TAG_USE_MULTI_RES_TEXTURE:
				swf.use_multi_resolution = true;
				break;

			case TAG_DISABLE_LOW_RES_TEXTURE:
				swf.use_low_resolution = false;
				break;

			case TAG_USE_EXTERNAL_TEXTURE:
				has_external_texture = true;
				break;

			case TAG_TEXTURE_FILE_SUFFIXES:
				swf.stream.read_string(swf.multi_resolution_suffix);
				swf.stream.read_string(swf.low_resolution_suffix);
				break;

			case TAG_TEXTURE:
			case TAG_TEXTURE_2:
			case TAG_TEXTURE_3:
			case TAG_TEXTURE_4:
			case TAG_TEXTURE_5:
			case TAG_TEXTURE_6:
			case TAG_TEXTURE_7:
			case TAG_TEXTURE_8:
			case TAG_TEXTURE_9:
			case TAG_TEXTURE_10:
				if (swf.textures.size() < textures_loaded) {
					throw ObjectLoadingException("Trying to load too many textures");
				}

				swf.textures[textures_loaded].load(swf, tag, !has_external_texture);
				textures_loaded++;
				break;

			case TAG_MOVIE_CLIP_MODIFIERS_COUNT: {
				uint16_t modifiers_count = swf.stream.read_unsigned_short();
				swf.movieclip_modifiers.resize(modifiers_count);
				break;
			}

			case TAG_MOVIE_CLIP_MODIFIER:
			case TAG_MOVIE_CLIP_MODIFIER_2:
			case TAG_MOVIE_CLIP_MODIFIER_3:
				swf.movieclip_modifiers[modifiers_loaded].load(swf, tag);
				modifiers_loaded++;
				break;

			case TAG_SHAPE:
			case TAG_SHAPE_2:
				if (swf.shapes.size() < shapes_loaded) {
					throw ObjectLoadingException("Trying to load too many Shapes");
				}

				swf.shapes[shapes_loaded].load(swf, tag);
				shapes_loaded++;
				break;

			case TAG_TEXT_FIELD:
			case TAG_TEXT_FIELD_2:
			case TAG_TEXT_FIELD_3:
			case TAG_TEXT_FIELD_4:
			case TAG_TEXT_FIELD_5:
			case TAG_TEXT_FIELD_6:
			case TAG_TEXT_FIELD_7:
			case TAG_TEXT_FIELD_8:
				if (swf.textfields.size() < textfields_loaded) {
					throw ObjectLoadingException("Trying to load too many TextFields");
				}

				swf.textfields[textfields_loaded].load(swf, tag);
				textfields_loaded++;
				break;

			case TAG_MATRIX_BANK:
				matrices_loaded = 0;
				colors_loaded = 0;
				banks_loaded++;
				{
					uint16_t matrix_count = swf.stream.read_unsigned_short();
					uint16_t colors_count = swf.stream.read_unsigned_short();
					swf.matrixBanks.emplace_back(matrix_count, colors_count);
				}
				break;

			case TAG_MATRIX_2x3:
			case TAG_MATRIX_2x3_2:
				swf.matrixBanks[banks_loaded].matrices[matrices_loaded].load(swf, tag);
				matrices_loaded++;
				break;

			case TAG_COLOR_TRANSFORM:
				swf.matrixBanks[banks_loaded].color_transforms[colors_loaded].load(swf);
				colors_loaded++;
				break;

			case TAG_MOVIE_CLIP:
			case TAG_MOVIE_CLIP_2:
			case TAG_MOVIE_CLIP_3:
			case TAG_MOVIE_CLIP_4:
			case TAG_MOVIE_CLIP_5:
			case TAG_MOVIE_CLIP_6:
				if (swf.movieclips.size() < movieclips_loaded) {
					throw ObjectLoadingException("Trying to load too many MovieClips");
				}

				swf.movieclips[movieclips_loaded].load(swf, tag);
				movieclips_loaded++;
				break;

			default:
				swf.stream.seek(tag_length, Seek::Add);
				break;
			}
		}

		return has_external_texture;
	}

	inline void _save_tags(SupercellSWF& swf) {
		if (swf.use_external_texture) {
			if (
				swf.multi_resolution_suffix.compare(MULTIRES_DEFAULT_SUFFIX) != 0 ||
				swf.low_resolution_suffix.compare(LOWRES_DEFAULT_SUFFIX) != 0) {
				size_t position = swf.stream.write_tag_header(TAG_TEXTURE_FILE_SUFFIXES);
				swf.stream.write_string(swf.multi_resolution_suffix);
				swf.stream.write_string(swf.low_resolution_suffix);
				swf.stream.write_tag_final(position);
			}

			if (!swf.use_low_resolution)
				swf.stream.write_tag_flag(TAG_DISABLE_LOW_RES_TEXTURE);

			if (swf.use_multi_resolution)
				swf.stream.write_tag_flag(TAG_USE_MULTI_RES_TEXTURE);

			swf.stream.write_tag_flag(TAG_USE_EXTERNAL_TEXTURE);
		}

		_save_textures(swf, !swf.use_external_texture, false);

		if (swf.movieclip_modifiers.size() > 0) {
			swf.stream.write_unsigned_byte(TAG_MOVIE_CLIP_MODIFIERS_COUNT); // Tag
			swf.stream.write_int(sizeof(uint16_t)); // Tag Size
			swf.stream.write_unsigned_short(swf.movieclip_modifiers.size());

			for (const MovieClipModifier& modifier : swf.movieclip_modifiers)
			{
				size_t position = swf.stream.write_tag_header(modifier.tag(swf));
				modifier.save(swf);
				swf.stream.write_tag_final(position);
			}
		}

		for (const Shape& shape : swf.shapes)
		{
			size_t position = swf.stream.write_tag_header(shape.tag(swf));
			shape.save(swf);
			swf.stream.write_tag_final(position);
		}

		for (const TextField& textField : swf.textfields)
		{
			size_t position = swf.stream.write_tag_header(textField.tag(swf));
			textField.save(swf);
			swf.stream.write_tag_final(position);
		}

		for (uint8_t i = 0; swf.matrixBanks.size() > i; i++)
		{
			const MatrixBank& bank = swf.matrixBanks[i];

			if (i != 0)
			{
				swf.stream.write_unsigned_byte(bank.tag(swf)); // Tag
				swf.stream.write_int(sizeof(uint16_t) * 2); // Tag Size
				swf.stream.write_unsigned_short(bank.matrices.size());
				swf.stream.write_unsigned_short(bank.color_transforms.size());
			}

			for (const Matrix2D& matrix : bank.matrices)
			{
				size_t position = swf.stream.write_tag_header(matrix.tag(swf));
				matrix.save(swf);
				swf.stream.write_tag_final(position);
			}

			for (const ColorTransform& color : bank.color_transforms)
			{
				size_t position = swf.stream.write_tag_header(color.tag(swf));
				color.save(swf);
				swf.stream.write_tag_final(position);
			}
		}

		for (const MovieClip& movieclip : swf.movieclips)
		{
			size_t position = swf.stream.write_tag_header(movieclip.tag(swf));
			movieclip.save(swf);
			swf.stream.write_tag_final(position);
		}
	}

	inline void _save_textures(SupercellSWF& swf, bool has_data, bool is_lowres) {
		for (uint16_t i = 0; swf.textures.size() > i; i++)
		{
			SWFTexture& texture = swf.textures[i];

			size_t position = swf.stream.write_tag_header(texture.tag(swf, has_data));
			if (swf.use_external_texture_files && has_data)
			{
				texture.encoding(SWFTexture::TextureEncoding::KhronosTexture);

				// Path String In Tag
				std::filesystem::path output_filepath = swf.current_file.parent_path();
				output_filepath /= swf.current_file.stem();
				if (is_lowres)
				{
					output_filepath += swf.low_resolution_suffix.string();
				}
				output_filepath += "_";
				output_filepath += std::to_string(i);
				output_filepath += ".zktx";

				{
					std::string texture_filename = output_filepath.filename().string();
					SWFString texture_path(texture_filename);
					swf.stream.write_string(texture_path);
				}

				BufferStream input_data;
				texture.save_buffer(input_data, is_lowres);

				OutputFileStream output_file(output_filepath);

				sc::Compressor::Zstd::Props props;
				sc::Compressor::Zstd ctx(props);
				input_data.seek(0);
				ctx.compress_stream(input_data, output_file);
			}
			texture.save(swf, has_data, is_lowres);
			swf.stream.write_tag_final(position);
		}
	}
}