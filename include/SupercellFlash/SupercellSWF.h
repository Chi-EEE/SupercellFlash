#pragma once

#include <stdint.h>
#include <filesystem>

#include "types/SWFStream.hpp"
#include "types/SWFContainer.hpp"

#include "objects/ExportName.h"

#define MULTIRES_DEFAULT_SUFFIX "_highres"
#define LOWRES_DEFAULT_SUFFIX "_lowres"

namespace sc
{
	class Shape;
	class MovieClip;
	class TextField;
	class MovieClipModifier;
	class MatrixBank;
	class SWFTexture;
	struct SupercellSWF
	{
	public:
		std::filesystem::path current_file;

		SWFVector<ExportName> exports;
		SWFVector<MatrixBank> matrixBanks;

		SWFVector<SWFTexture> textures;
		SWFVector<Shape> shapes;
		SWFVector<MovieClip> movieclips;
		SWFVector<TextField> textfields;
		SWFVector<MovieClipModifier> movieclip_modifiers;

		SWFStream stream;

		// Saves all textures to _tex.sc if true
		bool use_external_texture = false;
		bool use_multi_resolution = false;
		bool use_low_resolution = true;
		bool use_precision_matrix = false;

		// Saves all textures to zktx files if true and use_external_texture is true
		bool use_external_texture_files = true;

		// Saves custom properties in MovieClips
		bool save_custom_property = true;

		bool low_memory_usage_mode = false;

		SWFString multi_resolution_suffix = MULTIRES_DEFAULT_SUFFIX;
		SWFString low_resolution_suffix = LOWRES_DEFAULT_SUFFIX;
	};
}
