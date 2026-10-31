/*
	This file is part of D2DX.

	Copyright (C) 2021  Bolrog

	D2DX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	D2DX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with D2DX.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "pch.h"
#include <algorithm>
#include "D2DXContext.h"
#include "D2DXContextFactory.h"
#include "Detours.h"
#include "BuiltinMods.h"
#include "RenderContext.h"
#include "GameHelper.h"
#include "Metrics.h"
#include "Utils.h"
#include "Vertex.h"
#include "dx256_bmp.h"
#include "Profiler.h"
#include "CompatModeCheck.h"

using namespace d2dx;
using namespace DirectX::PackedVector;
using namespace std;

extern uint32_t currentlyDrawingWeatherParticles;
extern uint32_t* currentlyDrawingWeatherParticleIndexPtr;

#define D2DX_GLIDE_ALPHA_BLEND(rgb_sf, rgb_df, alpha_sf, alpha_df) \
		(uint16_t)(((rgb_sf & 0xF) << 12) | ((rgb_df & 0xF) << 8) | ((alpha_sf & 0xF) << 4) | (alpha_df & 0xF))

static Options GetCommandLineOptions()
{
	Options options;
	auto fileData = ReadTextFile("d2dx.cfg");
	options.ApplyCfg(fileData.items);
	options.ApplyCommandLine(GetCommandLineA());
	return options;
}

D2DXContext::D2DXContext() :
	_frame(0),
	_majorGameState(MajorGameState::Unknown),
	_paletteKeys(D2DX_MAX_PALETTES, true),
	_batchCount(0),
	_batches(D2DX_MAX_BATCHES_PER_FRAME),
	_vertexCount(0),
	_vertices(D2DX_MAX_VERTICES_PER_FRAME),
	_customGameSize{ 0,0 },
	_suggestedGameSize{ 0, 0 },
	_options{ GetCommandLineOptions() },
	_lastScreenOpenMode{ 0 },
	_initialScreenMode(strstr(GetCommandLineA(), "-w") ? ScreenMode::Windowed : ScreenMode::FullscreenDefault)
{
	_threadId = GetCurrentThreadId();

	auto windowsVersion = GetWindowsVersion();
	D2DX_LOG("Windows version: %u.%u (build %u).", windowsVersion.major, windowsVersion.minor, windowsVersion.build);

	bool requireNoCompat = !_options.GetFlag(OptionsFlag::NoCompatModeFix);
	CompatModeState mode = CheckCompatMode(requireNoCompat);
	switch (mode)
	{
	case CompatModeState::Updated:
		MessageBox(NULL, L"D2DX detected that 'compatibility mode' (e.g. for Windows XP) was set for the game, but this isn't necessary for D2DX and will cause problems.\n\nThis has now been fixed for you. Please re-launch the game.", L"D2DX", MB_OK);
		TerminateProcess(GetCurrentProcess(), -1);
		break;

	case CompatModeState::Enabled:
		if (requireNoCompat)
		{
			MessageBox(NULL, L"D2DX detected that 'compatibility mode' (e.g. for Windows XP) was set for the game, but this isn't necessary for D2DX and will cause problems.\n\nPlease disable 'compatibility mode' for both 'Game.exe' and 'Diablo II.exe'.", L"D2DX", MB_OK);
			TerminateProcess(GetCurrentProcess(), -1);
		}
		D2DX_LOG("Compatibility mode detected");
		windowsVersion = GetActualWindowsVersion();
		if (windowsVersion.major != 0)
		{
			D2DX_LOG("Actual windows version: %u.%u (build %u).", windowsVersion.major, windowsVersion.minor, windowsVersion.build);
		}
		break;

	case CompatModeState::Disabled:
	case CompatModeState::Unknown:
	default:
		break;
	}


#ifndef D2DX_UNITTEST
	_builtinMods.Init(GetModuleHandleW(L"glide3x.dll"), GetSuggestedCustomResolution(), _options);
#else
	_options.SetFlag(OptionsFlag::NoResMod, true);
	_options.SetFlag(OptionsFlag::NoFpsMod, true);
#endif

	AttachDetours(_gameHelper, *this);
}

D2DXContext::~D2DXContext() noexcept
{
	DetachDetours(_gameHelper, *this);
}

_Use_decl_annotations_
const char* D2DXContext::OnGetString(
	uint32_t pname)
{
	switch (pname)
	{
	case GR_EXTENSION:
		return " ";
	case GR_HARDWARE:
		return "Banshee";
	case GR_RENDERER:
		return "Glide";
	case GR_VENDOR:
		return "3Dfx Interactive";
	case GR_VERSION:
		return "3.0";
	}
	return NULL;
}

_Use_decl_annotations_
uint32_t D2DXContext::OnGet(
	uint32_t pname,
	uint32_t plength,
	int32_t* params)
{
	switch (pname)
	{
	case GR_MAX_TEXTURE_SIZE:
		*params = 256;
		return 4;
	case GR_MAX_TEXTURE_ASPECT_RATIO:
		*params = 3;
		return 4;
	case GR_NUM_BOARDS:
		*params = 1;
		return 4;
	case GR_NUM_FB:
		*params = 1;
		return 4;
	case GR_NUM_TMU:
		*params = 1;
		return 4;
	case GR_TEXTURE_ALIGN:
		*params = D2DX_TMU_ADDRESS_ALIGNMENT;
		return 4;
	case GR_MEMORY_UMA:
		*params = 0;
		return 4;
	case GR_GAMMA_TABLE_ENTRIES:
		*params = 256;
		return 4;
	case GR_BITS_GAMMA:
		*params = 8;
		return 4;
	default:
		return 0;
	}
}

_Use_decl_annotations_
void D2DXContext::OnSstWinOpen(
	uint32_t hWnd,
	int32_t width,
	int32_t height)
{
	_threadId = GetCurrentThreadId();

	Size windowSize = _gameHelper.GetConfiguredGameSize();
	if (!_options.GetFlag(OptionsFlag::NoResMod))
	{
		windowSize = { 800,600 };
	}

	Size gameSize{ width, height };

	if (_customGameSize.width > 0)
	{
		gameSize = _customGameSize;
		_customGameSize = { 0,0 };
	}

	if (gameSize.width != 640 || gameSize.height != 480)
	{
		windowSize = gameSize;
	}

	if (!_renderContext)
	{
		_renderContext = std::make_shared<RenderContext>(
			(HWND)hWnd,
			gameSize,
			windowSize * _options.GetWindowScale(),
			_initialScreenMode,
			this);
	}
	else
	{
		if (width > windowSize.width || height > windowSize.height)
		{
			windowSize.width = width;
			windowSize.height = height;
		}
		_renderContext->SetSizes(gameSize, windowSize * _options.GetWindowScale(), _renderContext->GetScreenMode());
	}

	_batchCount = 0;
	_vertexCount = 0;
	_scratchBatch = Batch();
}

_Use_decl_annotations_
void D2DXContext::OnVertexLayout(
	uint32_t param,
	int32_t offset)
{
	switch (param) {
	case GR_PARAM_XY:
		assert(offset == 0);
		break;
	case GR_PARAM_PARGB:
		assert(offset == 8);
		break;
	case GR_PARAM_ST0:
		assert(offset == 16);
		break;
	}
}

_Use_decl_annotations_
void D2DXContext::OnTexDownload(
	uint32_t tmu,
	const uint8_t* sourceAddress,
	uint32_t startAddress,
	int32_t width,
	int32_t height,
	int32_t format)
{
	Timer _timer(ProfCategory::TextureDownload);
	assert(tmu == 0 && (startAddress & 255) == 0);
	if (!(tmu == 0 && (startAddress & 255) == 0))
	{
		return;
	}

	_textureHasher.Invalidate(startAddress);

	/* GR_TEXFMT_RGB_565. The only such upload in the game is the Smacker
	   banner ad - see ConvertRgb565ToIndexed. */
	if (format == 0xa && width > 0 && height > 0 &&
		width * height <= (int32_t)_rgb565Histogram.size())
	{
		if (ConvertRgb565ToIndexed(sourceAddress, startAddress, width, height))
		{
			return;
		}
		/* No slot free: fall through to the plain copy. The texture will look
		   wrong, but nothing else is affected. */
	}

	/* A normal paletted upload landing on a banner range means the address has
	   been reused, so give the slot back. */
	for (int32_t i = 0; i < Rgb565SlotCount; ++i)
	{
		if (_rgb565Slots[i].used &&
			startAddress >= _rgb565Slots[i].startAddress &&
			startAddress < _rgb565Slots[i].startAddress + _rgb565Slots[i].size)
		{
			_rgb565Slots[i] = Rgb565Slot{};
		}
	}

	uint32_t memRequired = (uint32_t)(width * height);

	auto pStart = _glideState.tmuMemory.items + startAddress;
	auto pEnd = _glideState.tmuMemory.items + startAddress + memRequired;
	assert(pEnd <= (_glideState.tmuMemory.items + _glideState.tmuMemory.capacity));
	memcpy_s(pStart, _glideState.tmuMemory.capacity - startAddress, sourceAddress, memRequired);
}

bool D2DXContext::ConvertRgb565ToIndexed(
	const uint8_t* source,
	uint32_t startAddress,
	int32_t width,
	int32_t height)
{
	const int32_t pixelCount = width * height;
	const uint32_t byteCount = (uint32_t)pixelCount;

	int32_t slotIndex = FindRgb565Slot(startAddress);
	if (slotIndex < 0)
	{
		for (int32_t i = 0; i < Rgb565SlotCount; ++i)
		{
			if (!_rgb565Slots[i].used)
			{
				slotIndex = i;
				_rgb565Slots[i] = Rgb565Slot{};
				_rgb565Slots[i].used = true;
				_rgb565Slots[i].startAddress = startAddress;
				_rgb565Slots[i].size = byteCount;
				break;
			}
		}
	}
	if (slotIndex < 0)
	{
		return false;
	}

	Rgb565Slot& slot = _rgb565Slots[slotIndex];
	slot.startAddress = startAddress;
	slot.size = byteCount;

	const uint64_t hash = XXH3_64bits(source, (size_t)pixelCount * 2);
	if (hash != slot.hash)
	{
		slot.hash = hash;
		_rgb565Histogram.fill(0);

		auto pixelOf = [&](int32_t i) {
			return (uint16_t)(source[i * 2] | (source[i * 2 + 1] << 8));
		};

		/* Histogram in 5-5-5: fine enough to keep bands out of the dark end,
		   small enough that the median cut below is instant. */
		for (int32_t i = 0; i < pixelCount; ++i)
		{
			const uint16_t pixel = pixelOf(i);
			const uint32_t r = (pixel >> 11) & 0x1f;
			const uint32_t g = (pixel >> 6) & 0x1f;
			const uint32_t b = pixel & 0x1f;
			_rgb565Histogram[(r << 10) | (g << 5) | b]++;
		}

		struct Box
		{
			uint32_t begin;
			uint32_t end;
			uint32_t population;
			uint8_t widest;
			uint32_t extent;
		};

		std::array<uint16_t, 32768> keys{};
		int32_t keyCount = 0;
		for (uint32_t i = 0; i < 32768; ++i)
		{
			if (_rgb565Histogram[i] != 0)
			{
				keys[keyCount++] = (uint16_t)i;
			}
		}

		auto keyChannel = [](uint16_t k, int32_t c) -> uint32_t {
			return c == 0 ? ((k >> 10) & 0x1f) : (c == 1 ? ((k >> 5) & 0x1f) : (k & 0x1f));
		};

		auto measure = [&](Box& box) {
			uint32_t lo[3] = { 31, 31, 31 };
			uint32_t hi[3] = { 0, 0, 0 };
			box.population = 0;
			for (uint32_t i = box.begin; i < box.end; ++i)
			{
				const uint16_t k = keys[i];
				for (int32_t c = 0; c < 3; ++c)
				{
					const uint32_t v = keyChannel(k, c);
					lo[c] = min(lo[c], v);
					hi[c] = max(hi[c], v);
				}
				box.population += _rgb565Histogram[k];
			}
			box.widest = 0;
			box.extent = 0;
			for (int32_t c = 0; c < 3; ++c)
			{
				const uint32_t e = hi[c] - lo[c];
				if (e > box.extent)
				{
					box.extent = e;
					box.widest = (uint8_t)c;
				}
			}
		};

		std::array<Box, 256> boxes{};
		int32_t boxCount = 0;
		if (keyCount > 0)
		{
			boxes[0] = Box{ 0, (uint32_t)keyCount, 0, 0, 0 };
			measure(boxes[0]);
			boxCount = 1;

			while (boxCount < 256)
			{
				int32_t pick = -1;
				uint32_t best = 0;
				for (int32_t i = 0; i < boxCount; ++i)
				{
					if (boxes[i].end - boxes[i].begin > 1 && boxes[i].extent != 0)
					{
						const uint32_t score = boxes[i].extent * boxes[i].population;
						if (score > best)
						{
							best = score;
							pick = i;
						}
					}
				}
				if (pick < 0)
				{
					break;
				}

				Box& box = boxes[pick];
				const int32_t channel = box.widest;
				std::sort(keys.begin() + box.begin, keys.begin() + box.end,
					[&](uint16_t a, uint16_t b) {
						return keyChannel(a, channel) < keyChannel(b, channel);
					});

				const uint32_t half = box.population / 2;
				uint32_t running = 0;
				uint32_t split = box.end - 1;
				for (uint32_t i = box.begin; i < box.end; ++i)
				{
					running += _rgb565Histogram[keys[i]];
					if (running >= half && i > box.begin)
					{
						split = i;
						break;
					}
				}
				if (split <= box.begin)
				{
					split = box.begin + 1;
				}

				Box second = Box{ split, box.end, 0, 0, 0 };
				box.end = split;
				measure(box);
				measure(second);
				boxes[boxCount++] = second;
			}
		}

		uint32_t palette[256] = {};
		for (int32_t i = 0; i < 256; ++i)
		{
			if (i >= boxCount)
			{
				continue;
			}
			uint64_t sum[3] = { 0, 0, 0 };
			uint32_t total = 0;
			for (uint32_t k = boxes[i].begin; k < boxes[i].end; ++k)
			{
				const uint16_t key = keys[k];
				const uint32_t n = _rgb565Histogram[key];
				sum[0] += (uint64_t)n * keyChannel(key, 0);
				sum[1] += (uint64_t)n * keyChannel(key, 1);
				sum[2] += (uint64_t)n * keyChannel(key, 2);
				total += n;
			}
			if (total != 0)
			{
				const uint32_t r = (uint32_t)(sum[0] / total);
				const uint32_t g = (uint32_t)(sum[1] / total);
				const uint32_t b = (uint32_t)(sum[2] / total);
				palette[i] = 0xff000000u | (((r * 255 + 15) / 31) << 16) |
					(((g * 255 + 15) / 31) << 8) | ((b * 255 + 15) / 31);
			}
		}

		/* Nearest-entry lookup for every 5-5-5 bucket, so converting a frame
		   costs one array read per pixel. */
		for (uint32_t key = 0; key < 32768; ++key)
		{
			const int32_t r = (int32_t)((key >> 10) & 0x1f) * 255 / 31;
			const int32_t g = (int32_t)((key >> 5) & 0x1f) * 255 / 31;
			const int32_t b = (int32_t)(key & 0x1f) * 255 / 31;
			int32_t bestIndex = 0;
			int64_t bestDistance = INT64_MAX;
			for (int32_t i = 0; i < 256; ++i)
			{
				const int32_t pr = (int32_t)((palette[i] >> 16) & 0xff);
				const int32_t pg = (int32_t)((palette[i] >> 8) & 0xff);
				const int32_t pb = (int32_t)(palette[i] & 0xff);
				const int64_t distance = (int64_t)(pr - r) * (pr - r) +
					(int64_t)(pg - g) * (pg - g) + (int64_t)(pb - b) * (pb - b);
				if (distance < bestDistance)
				{
					bestDistance = distance;
					bestIndex = i;
				}
			}
			slot.lut[key] = (uint8_t)bestIndex;
		}

		_renderContext->SetPalette(Rgb565FirstPaletteIndex + slotIndex, palette);
	}

	auto pStart = _glideState.tmuMemory.items + startAddress;
	assert(_glideState.tmuMemory.capacity >= startAddress + byteCount);
	for (int32_t i = 0; i < pixelCount; ++i)
	{
		const uint16_t pixel = (uint16_t)(source[i * 2] | (source[i * 2 + 1] << 8));
		const uint32_t r = (pixel >> 11) & 0x1f;
		const uint32_t g = (pixel >> 6) & 0x1f;
		const uint32_t b = pixel & 0x1f;
		pStart[i] = slot.lut[(r << 10) | (g << 5) | b];
	}

	return true;
}

int32_t D2DXContext::FindRgb565Slot(
	uint32_t startAddress) const
{
	for (int32_t i = 0; i < Rgb565SlotCount; ++i)
	{
		if (_rgb565Slots[i].used &&
			startAddress >= _rgb565Slots[i].startAddress &&
			startAddress < _rgb565Slots[i].startAddress + _rgb565Slots[i].size)
		{
			return i;
		}
	}
	return -1;
}

_Use_decl_annotations_
void D2DXContext::OnTexSource(
	uint32_t tmu,
	uint32_t startAddress,
	int32_t width,
	int32_t height,
	uint32_t largeLog2,
	uint32_t ratioLog2)
{
	Timer _timer(ProfCategory::TextureSource);
	assert(tmu == 0 && (startAddress & 255) == 0);
	if (!(tmu == 0 && (startAddress & 255) == 0))
	{
		return;
	}

	_readVertexState.isDirty = true;

	uint8_t* pixels = _glideState.tmuMemory.items + startAddress;
	const uint32_t pixelsSize = width * height;

	int32_t stShift = 0;
	_BitScanReverse((DWORD*)&stShift, max(width, height));
	_glideState.stShift = 8 - stShift;

	uint64_t hash = _textureHasher.GetHash(startAddress, pixels, pixelsSize, largeLog2, ratioLog2);

	/* Patch the '5' to not look like '6'. */
	if (hash == 0xbeed610acac387d3)
	{
		pixels[1 + 10 * 16] = 181;
		pixels[2 + 10 * 16] = 181;
		pixels[1 + 11 * 16] = 29;
	}

	_scratchBatch.SetTextureStartAddress(startAddress);
	_scratchBatch.SetTextureHash(hash);
	_scratchBatch.SetTextureSize(width, height);

	/* The banner ad was converted from RGB565 against its own palette, so that
	   draw must use that palette; everything else must go back to whatever the
	   game last bound. Setting it only for the banner is not enough - the batch
	   carries the value forward, and the whole UI would be tinted by the
	   banner's palette. */
	const int32_t bannerSlot = FindRgb565Slot(startAddress);
	_scratchBatch.SetPaletteIndex(bannerSlot >= 0
		? Rgb565FirstPaletteIndex + bannerSlot
		: _gamePaletteSlot);

	if (_options.GetFlag(OptionsFlag::DbgDumpTextures))
	{
		DumpTexture(hash, width, height, pixels, pixelsSize, _glideState.palettes.items + _scratchBatch.GetPaletteIndex() * 256);
	}
}


_Use_decl_annotations_
void D2DXContext::OnTexFilterMode(
	GrChipID_t tmu,
	GrTextureFilterMode_t filterMode)
{
	assert(tmu == 0);
	_scratchBatch.SetFilterMode(filterMode);
}

void D2DXContext::CheckMajorGameState()
{
	const int32_t batchCount = (int32_t)_batchCount;

	if ((_majorGameState == MajorGameState::Unknown || _majorGameState == MajorGameState::FmvIntro) && batchCount == 0)
	{
		_majorGameState = MajorGameState::FmvIntro;
		return;
	}

	_majorGameState = MajorGameState::Other;

	for (int32_t i = 0; i < batchCount; ++i)
	{
		const Batch& batch = _batches.items[i];
		const float y0 = _vertices.items[batch.GetStartVertex()].GetY();

		if (batch.GetHash() == 0x84ab94c374c42d9a && y0 >= 550.0f)
		{
			_majorGameState = MajorGameState::TitleScreen;
			break;
		}
	}
}

_Use_decl_annotations_
void D2DXContext::DrawBatches(
	uint32_t startVertexLocation)
{
	const int32_t batchCount = (int32_t)_batchCount;

	Batch mergedBatch;
	int32_t drawCalls = 0;

	for (int32_t i = 0; i < batchCount; ++i)
	{
		const Batch& batch = _batches.items[i];

		if (!batch.IsValid())
		{
			D2DX_DEBUG_LOG("Skipping batch %i, it is invalid.", i);
			continue;
		}

		if (!mergedBatch.IsValid())
		{
			mergedBatch = batch;
		}
		else
		{
			if (_renderContext->GetTextureCache(batch) != _renderContext->GetTextureCache(mergedBatch) ||
				batch.GetTextureAtlas() != mergedBatch.GetTextureAtlas() ||
				batch.GetAlphaBlend() != mergedBatch.GetAlphaBlend() ||
				batch.GetFilterMode() != mergedBatch.GetFilterMode() ||
				((mergedBatch.GetVertexCount() + batch.GetVertexCount()) > 65535))
			{
				_renderContext->Draw(mergedBatch, startVertexLocation);
				++drawCalls;
				mergedBatch = batch;
			}
			else
			{
				mergedBatch.SetVertexCount(mergedBatch.GetVertexCount() + batch.GetVertexCount());
			}
		}
	}

	if (mergedBatch.IsValid())
	{
		_renderContext->Draw(mergedBatch, startVertexLocation);
		++drawCalls;
	}

	if (!(_frame & 255))
	{
		D2DX_DEBUG_LOG("Nr draw calls: %i", drawCalls);
	}
}


void D2DXContext::OnBufferSwap()
{
	CheckMajorGameState();
	InsertLogoOnTitleScreen();

	{
		Timer _timer(ProfCategory::DrawBatches);
		auto startVertexLocation = _renderContext->BulkWriteVertices(_vertices.items, _vertexCount);
		DrawBatches(startVertexLocation);
	}

	_renderContext->Present();

	++_frame;

	_batchCount = 0;
	_vertexCount = 0;
	_nextSurface = D2DX_SURFACE_FIRST;

	_renderContext->GetCurrentMetrics(&_gameSize, nullptr);

	_avgDir = { 0.0f, 0.0f };

	_readVertexState.isDirty = true;
}

_Use_decl_annotations_
void D2DXContext::OnColorCombine(
	GrCombineFunction_t function,
	GrCombineFactor_t factor,
	GrCombineLocal_t local,
	GrCombineOther_t other,
	bool invert)
{
	auto rgbCombine = RgbCombine::ColorMultipliedByTexture;

	if (function == GR_COMBINE_FUNCTION_SCALE_OTHER && factor == GR_COMBINE_FACTOR_LOCAL &&
		local == GR_COMBINE_LOCAL_ITERATED && other == GR_COMBINE_OTHER_TEXTURE)
	{
		rgbCombine = RgbCombine::ColorMultipliedByTexture;
	}
	else if (function == GR_COMBINE_FUNCTION_LOCAL && factor == GR_COMBINE_FACTOR_ZERO &&
		local == GR_COMBINE_LOCAL_CONSTANT && other == GR_COMBINE_OTHER_CONSTANT)
	{
		rgbCombine = RgbCombine::ConstantColor;
	}
	else
	{
		assert(false && "Unhandled color combine.");
	}

	_scratchBatch.SetRgbCombine(rgbCombine);

	_readVertexState.isDirty = true;
}

_Use_decl_annotations_
void D2DXContext::OnAlphaCombine(
	GrCombineFunction_t function,
	GrCombineFactor_t factor,
	GrCombineLocal_t local,
	GrCombineOther_t other,
	bool invert)
{
	auto alphaCombine = AlphaCombine::One;

	if (function == GR_COMBINE_FUNCTION_LOCAL && factor == GR_COMBINE_FACTOR_ZERO &&
		local == GR_COMBINE_LOCAL_CONSTANT && other == GR_COMBINE_OTHER_CONSTANT)
	{
		alphaCombine = AlphaCombine::FromColor;
	}

	_scratchBatch.SetAlphaCombine(alphaCombine);
}

_Use_decl_annotations_
void D2DXContext::OnConstantColorValue(
	uint32_t color)
{
	_glideState.constantColor = (color >> 8) | (color << 24);
	_readVertexState.isDirty = true;
}

_Use_decl_annotations_
void D2DXContext::OnAlphaBlendFunction(
	GrAlphaBlendFnc_t rgb_sf,
	GrAlphaBlendFnc_t rgb_df,
	GrAlphaBlendFnc_t alpha_sf,
	GrAlphaBlendFnc_t alpha_df)
{
	auto alphaBlend = AlphaBlend::Opaque;

	switch (D2DX_GLIDE_ALPHA_BLEND(rgb_sf, rgb_df, alpha_sf, alpha_df))
	{
	case D2DX_GLIDE_ALPHA_BLEND(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO):
		alphaBlend = AlphaBlend::Opaque;
		break;
	case D2DX_GLIDE_ALPHA_BLEND(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ZERO, GR_BLEND_ZERO):
		alphaBlend = AlphaBlend::SrcAlphaInvSrcAlpha;
		break;
	case D2DX_GLIDE_ALPHA_BLEND(GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO):
		alphaBlend = AlphaBlend::Additive;
		break;
	case D2DX_GLIDE_ALPHA_BLEND(GR_BLEND_ZERO, GR_BLEND_SRC_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO):
		alphaBlend = AlphaBlend::Multiplicative;
		break;
	}

	_scratchBatch.SetAlphaBlend(alphaBlend);

	_readVertexState.isDirty = true;
}

_Use_decl_annotations_
void D2DXContext::OnDrawPoint(
	const void* pt,
	uint32_t gameContext)
{
	Timer _timer(ProfCategory::Draw);
	Batch batch = _scratchBatch;
	batch.SetStartVertex(_vertexCount);

	EnsureReadVertexStateUpdated(batch);

	auto vertex0 = _readVertexState.templateVertex;

	const uint32_t iteratedColorMask = _readVertexState.iteratedColorMask;
	const uint32_t maskedConstantColor = _readVertexState.maskedConstantColor;
	const int32_t stShift = _glideState.stShift;

	const D2::Vertex* d2Vertex = (const D2::Vertex*)pt;

	vertex0.SetPosition(d2Vertex->x, d2Vertex->y);
	vertex0.SetTexcoord((int32_t)d2Vertex->s >> stShift, (int32_t)d2Vertex->t >> stShift);
	vertex0.SetColor(maskedConstantColor | (d2Vertex->color & iteratedColorMask));

	Vertex vertex1 = vertex0;
	Vertex vertex2 = vertex0;

	vertex1.AddOffset(1, 0);
	vertex2.AddOffset(1, 1);

	assert((_vertexCount + 3) < _vertices.capacity);
	_vertices.items[_vertexCount++] = vertex0;
	_vertices.items[_vertexCount++] = vertex1;
	_vertices.items[_vertexCount++] = vertex2;

	batch.SetVertexCount(3);
	FillVertexSurfaceId(batch);

	assert(_batchCount < _batches.capacity);
	_batches.items[_batchCount++] = batch;
}

_Use_decl_annotations_
void D2DXContext::OnDrawLine(
	const void* v1,
	const void* v2,
	uint32_t gameContext)
{
	Timer _timer(ProfCategory::Draw);
	Batch batch = _scratchBatch;
	batch.SetStartVertex(_vertexCount);
	batch.SetPaletteIndex(D2DX_WHITE_PALETTE_INDEX);

	EnsureReadVertexStateUpdated(batch);

	auto vertex0 = _readVertexState.templateVertex;

	const uint32_t iteratedColorMask = _readVertexState.iteratedColorMask;
	const uint32_t maskedConstantColor = _readVertexState.maskedConstantColor;

	const D2::Vertex* d2Vertex0 = (const D2::Vertex*)v1;
	const D2::Vertex* d2Vertex1 = (const D2::Vertex*)v2;

	vertex0.SetTexcoord((int32_t)d2Vertex1->s >> _glideState.stShift, (int32_t)d2Vertex1->t >> _glideState.stShift);
	vertex0.SetColor(maskedConstantColor | (d2Vertex1->color & iteratedColorMask));

	OffsetF widening = { d2Vertex1->y - d2Vertex0->y, d2Vertex1->x - d2Vertex0->x };
	widening.NormalizeTo(0.5f);

	Vertex vertex1 = vertex0;
	Vertex vertex2 = vertex0;
	Vertex vertex3 = vertex0;

	vertex0.SetPosition(d2Vertex1->x + widening.x, d2Vertex1->y - widening.y);
	vertex1.SetPosition(d2Vertex0->x + widening.x, d2Vertex0->y - widening.y);
	vertex2.SetPosition(d2Vertex1->x - widening.x, d2Vertex1->y + widening.y);
	vertex3.SetPosition(d2Vertex0->x - widening.x, d2Vertex0->y + widening.y);

	assert((_vertexCount + 6) < _vertices.capacity);
	_vertices.items[_vertexCount++] = vertex0;
	_vertices.items[_vertexCount++] = vertex1;
	_vertices.items[_vertexCount++] = vertex2;
	_vertices.items[_vertexCount++] = vertex1;
	_vertices.items[_vertexCount++] = vertex3;
	_vertices.items[_vertexCount++] = vertex2;

	batch.SetVertexCount(6);
	FillVertexSurfaceId(batch);

	assert(_batchCount < _batches.capacity);
	_batches.items[_batchCount++] = batch;
}

_Use_decl_annotations_
const Batch D2DXContext::PrepareBatchForSubmit(
	Batch batch,
	PrimitiveType primitiveType,
	uint32_t vertexCount,
	uint32_t gameContext) const
{
	auto tcl = _renderContext->UpdateTexture(batch, _glideState.tmuMemory.items, _glideState.tmuMemory.capacity);

	if (tcl._textureAtlas < 0)
	{
		return batch;
	}

	batch.SetTextureAtlas(tcl._textureAtlas);
	batch.SetTextureIndex(tcl._textureIndex);

	batch.SetStartVertex(_vertexCount);
	batch.SetVertexCount(vertexCount);
	return batch;
}

_Use_decl_annotations_
void D2DXContext::FillVertexSurfaceId(
	const Batch& batch)
{
	uint16_t id = batch.GetSurfaceId();
	for (auto i = &_vertices.items[batch.GetStartVertex()], end = i + batch.GetVertexCount(); i < end; ++i)
	{
		i->SetSurfaceId(id);
	}
}

_Use_decl_annotations_
void D2DXContext::EnsureReadVertexStateUpdated(
	const Batch& batch)
{
	if (!_readVertexState.isDirty)
	{
		return;
	}

	_readVertexState.templateVertex = Vertex(
		0, 0,
		0, 0,
		0,
		batch.IsChromaKeyEnabled(),
		batch.GetTextureIndex(),
		batch.GetRgbCombine() == RgbCombine::ColorMultipliedByTexture ? batch.GetPaletteIndex() : D2DX_WHITE_PALETTE_INDEX,
		0);

	const bool isIteratedColor = batch.GetRgbCombine() == RgbCombine::ColorMultipliedByTexture;
	const uint32_t constantColorMask = isIteratedColor ? 0xFF000000 : 0xFFFFFFFF;
	_readVertexState.constantColorMask = constantColorMask;
	_readVertexState.iteratedColorMask = isIteratedColor ? 0x00FFFFFF : 0x00000000;
	_readVertexState.maskedConstantColor = constantColorMask & (_glideState.constantColor | (batch.GetAlphaBlend() != AlphaBlend::SrcAlphaInvSrcAlpha ? 0xFF000000 : 0));
	_readVertexState.isDirty = false;
}

_Use_decl_annotations_
void D2DXContext::OnDrawVertexArray(
	uint32_t mode,
	uint32_t count,
	uint8_t** pointers,
	uint32_t gameContext)
{
	Timer _timer(ProfCategory::Draw);
	assert(mode == GR_TRIANGLE_STRIP || mode == GR_TRIANGLE_FAN);

	if (count < 3 || (mode != GR_TRIANGLE_STRIP && mode != GR_TRIANGLE_FAN))
	{
		return;
	}

	EnsureReadVertexStateUpdated(_scratchBatch);

	Vertex v = _readVertexState.templateVertex;

	const uint32_t iteratedColorMask = _readVertexState.iteratedColorMask;
	const uint32_t maskedConstantColor = _readVertexState.maskedConstantColor;

	Vertex* pVertices = &_vertices.items[_vertexCount];

	bool onScreen = false;
	for (int32_t i = 0; i < 3; ++i)
	{
		const D2::Vertex* d2Vertex = (const D2::Vertex*)pointers[i];
		v.SetPosition(d2Vertex->x, d2Vertex->y);
		v.SetTexcoord((int32_t)d2Vertex->s >> _glideState.stShift, (int32_t)d2Vertex->t >> _glideState.stShift);
		v.SetColor(maskedConstantColor | (d2Vertex->color & iteratedColorMask));
		*pVertices++ = v;
		onScreen |= 0 <= d2Vertex->x && d2Vertex->x < _gameSize.width &&
			0 <= d2Vertex->y && d2Vertex->y < _gameSize.height;
	}

	if (mode == GR_TRIANGLE_FAN)
	{
		auto vertex0 = pVertices[-3];

		for (uint32_t i = 0; i < (count - 3); ++i)
		{
			*pVertices++ = vertex0;
			*pVertices++ = pVertices[-2];
			const D2::Vertex* d2Vertex = (const D2::Vertex*)pointers[i + 3];
			v.SetPosition(d2Vertex->x, d2Vertex->y);
			v.SetTexcoord((int32_t)d2Vertex->s >> _glideState.stShift, (int32_t)d2Vertex->t >> _glideState.stShift);
			v.SetColor(maskedConstantColor | (d2Vertex->color & iteratedColorMask));
			*pVertices++ = v;
			onScreen |= 0 <= d2Vertex->x && d2Vertex->x < _gameSize.width &&
				0 <= d2Vertex->y && d2Vertex->y < _gameSize.height;
		}
	}
	else
	{
		for (uint32_t i = 0; i < (count - 3); ++i)
		{
			*pVertices++ = pVertices[-2];
			*pVertices++ = pVertices[-2];
			const D2::Vertex* d2Vertex = (const D2::Vertex*)pointers[i + 3];
			v.SetPosition(d2Vertex->x, d2Vertex->y);
			v.SetTexcoord((int32_t)d2Vertex->s >> _glideState.stShift, (int32_t)d2Vertex->t >> _glideState.stShift);
			v.SetColor(maskedConstantColor | (d2Vertex->color & iteratedColorMask));
			*pVertices++ = v;
			onScreen |= 0 <= d2Vertex->x && d2Vertex->x < _gameSize.width &&
				0 <= d2Vertex->y && d2Vertex->y < _gameSize.height;
		}
	}

	if (onScreen)
	{
		Batch batch = PrepareBatchForSubmit(_scratchBatch, PrimitiveType::Triangles, 3 * (count - 2), gameContext);
		if (!batch.IsValid())
		{
			return;
		}

		pVertices = &_vertices.items[_vertexCount];
		for (uint32_t i = 0; i < 3 * (count - 2); ++i)
		{
			pVertices[i].SetAtlasIndex(batch.GetTextureIndex());
		}

		_vertexCount += 3 * (count - 2);
		FillVertexSurfaceId(batch);

		assert(_batchCount < _batches.capacity);
		_batches.items[_batchCount++] = batch;
	}
	else
	{
		AddDroppedDraw();
	}
}

_Use_decl_annotations_
void D2DXContext::OnDrawVertexArrayContiguous(
	uint32_t mode,
	uint32_t count,
	uint8_t* vertex,
	uint32_t stride,
	uint32_t gameContext)
{
	Timer _timer(ProfCategory::Draw);
	assert(count == 4);
	assert(mode == GR_TRIANGLE_FAN);
	assert(stride == sizeof(D2::Vertex));

	if (mode != GR_TRIANGLE_FAN || count != 4 || stride != sizeof(D2::Vertex))
	{
		return;
	}

	EnsureReadVertexStateUpdated(_scratchBatch);

	const uint32_t iteratedColorMask = _readVertexState.iteratedColorMask;
	const uint32_t maskedConstantColor = _readVertexState.maskedConstantColor;

	const D2::Vertex* d2Vertices = (const D2::Vertex*)vertex;

	Vertex v = _readVertexState.templateVertex;

	Vertex* pVertices = &_vertices.items[_vertexCount];

	bool onScreen = false;
	for (int32_t i = 0; i < 4; ++i)
	{
		v.SetPosition(d2Vertices[i].x, d2Vertices[i].y);
		v.SetTexcoord((int32_t)d2Vertices[i].s >> _glideState.stShift, (int32_t)d2Vertices[i].t >> _glideState.stShift);
		v.SetColor(maskedConstantColor | (d2Vertices[i].color & iteratedColorMask));
		pVertices[i] = v;
		onScreen |= 0 <= d2Vertices[i].x && d2Vertices[i].x < _gameSize.width &&
			0 <= d2Vertices[i].y && d2Vertices[i].y < _gameSize.height;
	}

	pVertices[4] = pVertices[0];
	pVertices[5] = pVertices[2];

	if (onScreen)
	{
		Batch batch = PrepareBatchForSubmit(_scratchBatch, PrimitiveType::Triangles, 3 * (count - 2), gameContext);
		if (!batch.IsValid())
		{
			return;
		}

		for (int i = 0; i < 6; ++i)
		{
			pVertices[i].SetAtlasIndex(batch.GetTextureIndex());
		}

		_vertexCount += 6;
		FillVertexSurfaceId(batch);

		assert(_batchCount < _batches.capacity);
		_batches.items[_batchCount++] = batch;
	}
	else
	{
		AddDroppedDraw();
	}
}

_Use_decl_annotations_
void D2DXContext::OnTexDownloadTable(
	GrTexTable_t type,
	void* data)
{
	Timer _timer(ProfCategory::TextureDownload);
	if (type != GR_TEXTABLE_PALETTE)
	{
		assert(false && "Unhandled table type.");
		return;
	}

	_readVertexState.isDirty = true;

	uint64_t hash = XXH3_64bits(data, 1024);
	assert(hash != 0);

	for (uint32_t i = 0; i < D2DX_MAX_GAME_PALETTES; ++i)
	{
		if (_paletteKeys.items[i] == 0)
		{
			break;
		}

		if (_paletteKeys.items[i] == hash)
		{
			_scratchBatch.SetPaletteIndex(i);
			_gamePaletteSlot = (int32_t)i;
			return;
		}
	}

	for (uint32_t i = 0; i < D2DX_MAX_GAME_PALETTES; ++i)
	{
		if (_paletteKeys.items[i] == 0)
		{
			_paletteKeys.items[i] = hash;
			_scratchBatch.SetPaletteIndex(i);
			_gamePaletteSlot = (int32_t)i;

			uint32_t* palette = (uint32_t*)data;

			for (int32_t j = 0; j < 256; ++j)
			{
				palette[j] |= 0xFF000000;
			}

			if (_options.GetFlag(OptionsFlag::DbgDumpTextures))
			{
				memcpy(_glideState.palettes.items + 256 * i, palette, 1024);
			}

			_renderContext->SetPalette(i, palette);
			return;
		}
	}

	assert(false && "Too many palettes.");
	D2DX_LOG("Too many palettes.");
}

_Use_decl_annotations_
void D2DXContext::OnChromakeyMode(
	GrChromakeyMode_t mode)
{
	_scratchBatch.SetIsChromaKeyEnabled(mode == GR_CHROMAKEY_ENABLE);

	_readVertexState.isDirty = true;
}

_Use_decl_annotations_
void D2DXContext::OnLoadGammaTable(
	uint32_t nentries,
	uint32_t* red,
	uint32_t* green,
	uint32_t* blue)
{
	Timer _timer(ProfCategory::TextureDownload);
	for (int32_t i = 0; i < (int32_t)min(nentries, 256); ++i)
	{
		_glideState.gammaTable.items[i] = ((blue[i] & 0xFF) << 16) | ((green[i] & 0xFF) << 8) | (red[i] & 0xFF);
	}

	_renderContext->LoadGammaTable(_glideState.gammaTable.items, _glideState.gammaTable.capacity);
}

_Use_decl_annotations_
void D2DXContext::OnLfbUnlock(
	const uint8_t* lfbPtr,
	bool is16Bit)
{
	bool forCinematic = !(_majorGameState == MajorGameState::Unknown || _majorGameState == MajorGameState::FmvIntro);
	_renderContext->WriteToScreen(lfbPtr, 640, 480, is16Bit, forCinematic);
}

_Use_decl_annotations_
void D2DXContext::OnGammaCorrectionRGB(
	float red,
	float green,
	float blue)
{
	Timer _timer(ProfCategory::TextureDownload);
	uint32_t gammaTable[256];

	for (int32_t i = 0; i < 256; ++i)
	{
		float v = i / 255.0f;
		float r = powf(v, 1.0f / red);
		float g = powf(v, 1.0f / green);
		float b = powf(v, 1.0f / blue);
		uint32_t ri = (uint32_t)(r * 255.0f);
		uint32_t gi = (uint32_t)(g * 255.0f);
		uint32_t bi = (uint32_t)(b * 255.0f);
		gammaTable[i] = (ri << 16) | (gi << 8) | bi;
	}

	_renderContext->LoadGammaTable(gammaTable, ARRAYSIZE(gammaTable));
}

void D2DXContext::PrepareLogoTextureBatch()
{
	if (_logoTextureBatch.IsValid())
	{
		return;
	}

	const uint8_t* srcPixels = dx_logo256 + 0x436;

	Buffer<uint32_t> palette(256);
	memcpy_s(palette.items, palette.capacity * sizeof(uint32_t), (uint32_t*)(dx_logo256 + 0x36), 256 * sizeof(uint32_t));

	for (int32_t i = 0; i < 256; ++i)
	{
		palette.items[i] |= 0xFF000000;
	}

	_renderContext->SetPalette(D2DX_LOGO_PALETTE_INDEX, palette.items);

	uint64_t hash = XXH3_64bits((void*)srcPixels, sizeof(uint8_t) * 81 * 40);

	uint8_t* data = _glideState.sideTmuMemory.items;

	_logoTextureBatch.SetTextureStartAddress(0);
	_logoTextureBatch.SetTextureHash(hash);
	_logoTextureBatch.SetTextureSize(128, 128);
	_logoTextureBatch.SetAlphaBlend(AlphaBlend::SrcAlphaInvSrcAlpha);
	_logoTextureBatch.SetIsChromaKeyEnabled(true);
	_logoTextureBatch.SetRgbCombine(RgbCombine::ColorMultipliedByTexture);
	_logoTextureBatch.SetAlphaCombine(AlphaCombine::One);
	_logoTextureBatch.SetPaletteIndex(D2DX_LOGO_PALETTE_INDEX);
	_logoTextureBatch.SetVertexCount(6);
	_logoTextureBatch.SetSurfaceId(D2DX_SURFACE_UI);

	memset(data, 0, _logoTextureBatch.GetTextureWidth() * _logoTextureBatch.GetTextureHeight());

	for (int32_t y = 0; y < 41; ++y)
	{
		for (int32_t x = 0; x < 80; ++x)
		{
			data[x + (40 - y) * 128] = *srcPixels++;
		}
	}
}

void D2DXContext::InsertLogoOnTitleScreen()
{
	Timer _timer(ProfCategory::Draw);
	if (_options.GetFlag(OptionsFlag::NoLogo) || _majorGameState != MajorGameState::TitleScreen || _batchCount <= 0)
		return;

	PrepareLogoTextureBatch();

	auto tcl = _renderContext->UpdateTexture(_logoTextureBatch, _glideState.sideTmuMemory.items, _glideState.sideTmuMemory.capacity);

	_logoTextureBatch.SetTextureAtlas(tcl._textureAtlas);
	_logoTextureBatch.SetTextureIndex(tcl._textureIndex);
	_logoTextureBatch.SetStartVertex(_vertexCount);

	Size gameSize;
	_renderContext->GetCurrentMetrics(&gameSize, nullptr);

	const float x1 = static_cast<float>(gameSize.width - 90 - 16);
	const float x2 = static_cast<float>(gameSize.width - 10 - 16);
	const float y1 = static_cast<float>(gameSize.height - 50 - 16);
	const float y2 = static_cast<float>(gameSize.height - 9 - 16);
	const uint32_t color = 0xFFFFa090;

	Vertex vertex0(x1, y1, 0, 0, color, true, _logoTextureBatch.GetTextureIndex(), D2DX_LOGO_PALETTE_INDEX, D2DX_SURFACE_UI);
	Vertex vertex1(x2, y1, 80, 0, color, true, _logoTextureBatch.GetTextureIndex(), D2DX_LOGO_PALETTE_INDEX, D2DX_SURFACE_UI);
	Vertex vertex2(x2, y2, 80, 41, color, true, _logoTextureBatch.GetTextureIndex(), D2DX_LOGO_PALETTE_INDEX, D2DX_SURFACE_UI);
	Vertex vertex3(x1, y2, 0, 41, color, true, _logoTextureBatch.GetTextureIndex(), D2DX_LOGO_PALETTE_INDEX, D2DX_SURFACE_UI);

	assert((_vertexCount + 6) < _vertices.capacity);
	_vertices.items[_vertexCount++] = vertex0;
	_vertices.items[_vertexCount++] = vertex1;
	_vertices.items[_vertexCount++] = vertex2;
	_vertices.items[_vertexCount++] = vertex0;
	_vertices.items[_vertexCount++] = vertex2;
	_vertices.items[_vertexCount++] = vertex3;

	_batches.items[_batchCount++] = _logoTextureBatch;
}

_Use_decl_annotations_
Offset D2DXContext::GameToWinCursorPos(
	Offset pos)
{
	Size gameSize;
	Rect renderRect;
	_renderContext->GetCurrentMetrics(&gameSize, &renderRect);

	if (pos.x < 0 || gameSize.width < pos.x ||
		pos.y < 0 || gameSize.height < pos.y)
	{
		return pos;
	}

	const OffsetF scale = {
		(float)renderRect.size.width / gameSize.width,
		(float)renderRect.size.height / gameSize.height,
	};

	Offset cursorPos{ 0, 0 };
	if (GetCursorPos((LPPOINT)&cursorPos))
	{
		ScreenToClient(_renderContext->GetHWnd(), (LPPOINT)&cursorPos);
		if (renderRect.offset.x <= cursorPos.x && cursorPos.x < renderRect.offset.x + renderRect.size.width
			&& renderRect.offset.y <= cursorPos.y && cursorPos.y < renderRect.offset.y + renderRect.size.height)
		{
			// Scale the cursor's movement in order to avoid rounding errors.
			Offset delta = pos - Offset(OffsetF(cursorPos - renderRect.offset) / scale);
			return Offset(OffsetF(delta) * scale) + cursorPos;
		}
	}

	// If the cursor isn't currently over the game just scale the target position.
	return Offset(OffsetF(pos) * scale) + renderRect.offset;
}

_Use_decl_annotations_
Offset D2DXContext::OnSetCursorPos(
	Offset pos)
{
	auto hWnd = _renderContext->GetHWnd();
	if (_initialScreenMode == ScreenMode::Windowed)
	{
		// When launched in windowed mode (-w) the game sets the cursor relative
		// to the window, but it makes incorrect assumptions about window decorations.
		RECT win;
		GetWindowRect(hWnd, &win);
		RECT client;
		GetClientRect(hWnd, &client);
		ClientToScreen(hWnd, (LPPOINT)&client);

		pos.x = pos.x - GetSystemMetrics(SM_CXDLGFRAME) - win.left + client.left;
		pos.y = pos.y - GetSystemMetrics(SM_CYCAPTION) - win.top + client.top;
		ScreenToClient(hWnd, (LPPOINT)&pos);
	}
	pos = GameToWinCursorPos(pos);
	ClientToScreen(hWnd, (LPPOINT)&pos);
	return pos;
}

_Use_decl_annotations_
Offset D2DXContext::OnMouseMoveMessage(
	Offset pos)
{
	return GameToWinCursorPos(pos);
}

_Use_decl_annotations_
bool D2DXContext::IsD2ClientModule(
	HMODULE module)
{
	return this->_gameHelper.d2ClientDll == module;
}

_Use_decl_annotations_
void D2DXContext::SetCustomResolution(
	Size size)
{
	_customGameSize = size;
}

Size D2DXContext::GetSuggestedCustomResolution()
{
	if (_suggestedGameSize.width == 0)
	{
		if (_gameHelper.isProjectDiablo2)
		{
			_suggestedGameSize = { 1068, 600 };
		}
		else
		{
			Size desktopSize{ GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };

			_suggestedGameSize = _options.GetUserSpecifiedGameSize();
			_suggestedGameSize.width = min(_suggestedGameSize.width, desktopSize.width);
			_suggestedGameSize.height = min(_suggestedGameSize.height, desktopSize.height);

			if (_suggestedGameSize.width < 0 || _suggestedGameSize.height < 0)
			{
				_suggestedGameSize = Metrics::GetSuggestedGameSize(desktopSize, !_options.GetFlag(OptionsFlag::NoWide));
			}
		}

		D2DX_LOG("Suggesting game size %ix%i.", _suggestedGameSize.width, _suggestedGameSize.height);
	}

	return _suggestedGameSize;
}

void D2DXContext::DisableBuiltinResMod()
{
	_options.SetFlag(OptionsFlag::NoResMod, true);
}

const Options& D2DXContext::GetOptions() const
{
	return _options;
}

Options& D2DXContext::GetOptions()
{
	return _options;
}

_Use_decl_annotations_
void D2DXContext::InterceptDrawText(
	wchar_t* str)
{
	if (!str)
	{
		return;
	}

	if (_gameHelper.gameVersion == GameVersion::Lod114d)
	{
		// In 1.14d, some color codes are black. Remap them.

		// Bright white -> white
		while (wchar_t* subStr = wcsstr(str, L"\u00FFc/"))
		{
			subStr[2] = L'0';
		}
	}
}
