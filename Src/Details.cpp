// Details.cpp
//
// Image details overlay and a meta-data inspector.
//
// Copyright (c) 2022-2024 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <Math/tVector2.h>
#include "imgui.h"
#include "Details.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
#include "GuiUtil.h"
using namespace tMath;
using namespace tImage;


void Viewer::ShowImageDetailsOverlay(bool* popen, float x, float y, float w, float h, int cursorX, int cursorY, float zoom)
{
	// This overlay function is pretty much taken from the DearImGui demo code.
	const float margin = 6.0f;
	Config::ProfileData& profile = Config::GetProfileData();

	tVector2 windowPos = tVector2
	(
		x + ((profile.OverlayCorner & 1) ? w - margin : margin),
		y + ((profile.OverlayCorner & 2) ? h - margin : margin)
	);
	tVector2 windowPivot = tVector2
	(
		(profile.OverlayCorner & 1) ? 1.0f : 0.0f,
		(profile.OverlayCorner & 2) ? 1.0f : 0.0f
	);

	float winWidth = Gutil::GetUIParamScaled(146.0f, 2.5f);
	ImGui::SetNextWindowSize(tVector2(winWidth, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove				| ImGuiWindowFlags_NoTitleBar		| ImGuiWindowFlags_NoResize				|
		ImGuiWindowFlags_AlwaysAutoResize	| ImGuiWindowFlags_NoSavedSettings	| ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav				| ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("ImageDetails", popen, flags))
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + winWidth*0.18f);
		ImGui::Text("Image   Details");

		Gutil::ToolTip
		(
			"Right-Click to Change Anchor"
		);
		ImGui::Separator();

		if (CurrImage)
		{
			tColour4f floatCol(PixelColour);
			tVector4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);

			float colourButtonSize = Gutil::GetUIParamScaled(15.0f, 2.5f);
			if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, tVector2(colourButtonSize, colourButtonSize)))
				ImGui::OpenPopup("CopyColourOverlayAs");

			if (ImGui::BeginPopup("CopyColourOverlayAs"))
				ColourCopyAs();

			ImGui::SameLine(); ImGui::Text("(%d, %d, %d, %d)", PixelColour.R, PixelColour.G, PixelColour.B, PixelColour.A);

			Image::ImgInfo& info = CurrImage->Info;
			if (info.IsValid())
			{
				ImGui::Text("Size: %dx%d", CurrImage->GetWidth(), CurrImage->GetHeight());
				ImGui::Text("Frmt: %s", tImage::tGetPixelFormatName(info.SrcPixelFormat));
				tString bppStr("--");
				int bpp = tImage::tGetBitsPerPixel(info.SrcPixelFormat);
				if (bpp > 0)
				{
					tsPrintf(bppStr, "%d", bpp);
				}
				else
				{
					float bppf = tImage::tGetBitsPerPixelFloat(info.SrcPixelFormat);
					if (bppf > 0.0f)
					{
						tsPrintf(bppStr, "%4.2f", bppf);
						bppStr.ExtractRight(".00");
					}
				}

				const char* colourProfileName = tGetColourProfileShortName(info.SrcColourProfile);
				tAssert(colourProfileName);
				ImGui::Text("Colour Profile: %s", colourProfileName);
				Gutil::ToolTip
				(
					"The Colour Profile is a best guess at the type of pixel data present.\n"
					"LDR means low dynamic range (0.0 to 1.0). HDR means values above 1.0\n\n"
					"sRGB : LDR RGB in the sRGB colour space. LDR alpha in linear space.\n"
					"gRGB : LDR RGB in gamma colour space. LDR alpha in linear space.\n"
					"lRGB : LDR RGBA all in linear space.\n"
					"HDRa : HDR RGB in linear space. LDR alpha in linear space.\n"
					"HDRA : HDR RGBA all in linear space."
				);

				// Only display AlphaMode and ChannelType if they are specified. Most image formats will not
				// be able to provide these values so we don't want to waste screen real-estate.
				if (info.AlphaMode != tAlphaMode::Unspecified)
				{
					const char* alphaModeName = tGetAlphaModeName(info.AlphaMode);
					tAssert(alphaModeName);
					ImGui::Text("Alpha Mode: %s", alphaModeName);
					Gutil::ToolTip
					(
						"The Alpha Mode specifies whether the alpha has been premultiplied into\n"
						"the colour channels. Mult means is has. Norm means is hasn't.\n"
						"\n"
						"Not all images supply this information, so it may not be displayed."
					);
				}

				if (info.ChannelType != tChannelType::Unspecified)
				{
					const char* channelTypeName = tGetChannelTypeShortName(info.ChannelType);
					tAssert(channelTypeName);
					ImGui::Text("Channel Type: %s", channelTypeName);
					Gutil::ToolTip
					(
						"The Channel Type specifies how the data was intended to be interpreted by a\n"
						"graphics API.\n"
						"\n"
						"  UNORM: Channel data is unsigned integer and then normalized to [0.0,1.0].\n"
						"  SNORM: Channel data is signed integer and then normalized to [0.0,1.0].\n"
						"  UINT: Channel data is unsigned integer and not normalised.\n"
						"  SINT: Channel data is signed integer and not normalised.\n"
						"  UFLOAT: Channel data is an unsigned float (no sign bit).\n"
						"  SFLOAT: Channel data is a signed float.\n"
						"\n"
						"Eg. If there are 11 bits used for R:\n"
						"  SINT: R is in [-1024,1023].\n"
						"  UINT: R is in [0,2047].\n"
						"  SNORM: R is in [0,2047] and then is mapped to [0.0,1.0].\n"
						"  UNORM: R is in [-1024,1023] and then is mapped to [0.0,1.0].\n"
						"  SFLOAT: R is in [FLT_MIN,FLT_MAX]. Depends on precision of 11-bit float.\n"
						"  UFLOAT: R is in [0.0,FLT_MAX]. Depends on precision of 11-bit float.\n"
						"\n"
						"Not all images supply this information, so it may not be displayed."
					);
				}

				ImGui::Text("Bits Per Pixel: %s", bppStr.Chr());
				switch (info.Opacity)
				{
					case Image::ImgInfo::OpacityEnum::False:	ImGui::Text("Opaque: False");	Gutil::ToolTip("False means at least one pixel is not opaque.");	break;
					case Image::ImgInfo::OpacityEnum::True:		ImGui::Text("Opaque: True");	Gutil::ToolTip("True means all pixels are opaque.");				break;
					case Image::ImgInfo::OpacityEnum::Varies:	ImGui::Text("Opaque: Varies");	Gutil::ToolTip("Varies means there is more than one frame/mipmap/page/side\nand they don't all match. This is likely not what you want\nbut is reasonable for, say, pages in a tiff.");	break;
				}
				ImGui::Text("Frames: %d", CurrImage->GetNumFrames());
				tString sizeStr; tsPrintf(sizeStr, "File Size: %'d", info.FileSizeBytes);
				ImGui::Text(sizeStr.Chr());
				ImGui::Text("Cursor: (%d, %d)", cursorX, cursorY);
				ImGui::Text("Pan: (%d, %d)", GetPanX(), GetPanY());
				ImGui::Text("Zoom: %.0f%%", zoom);
			}
		}
		ImGui::Text("Images In Folder: %d", Images.GetNumItems());

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left",		nullptr, profile.OverlayCorner == 0)) profile.OverlayCorner = 0;
			if (ImGui::MenuItem("Top-right",	nullptr, profile.OverlayCorner == 1)) profile.OverlayCorner = 1;
			if (ImGui::MenuItem("Bottom-left",  nullptr, profile.OverlayCorner == 2)) profile.OverlayCorner = 2;
			if (ImGui::MenuItem("Bottom-right", nullptr, profile.OverlayCorner == 3)) profile.OverlayCorner = 3;
			if (popen && ImGui::MenuItem("Close")) *popen = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}


void Viewer::ColourCopyAs()
{
	tColour4f floatCol(PixelColour);
	ImGui::Text("Copy As...");
	int ri = PixelColour.R; int gi = PixelColour.G; int bi = PixelColour.B; int ai = PixelColour.A;
	float rf = floatCol.R; float gf = floatCol.G; float bf = floatCol.B; float af = floatCol.A;
	tString cpyTxt;

	tsPrintf(cpyTxt, "%d %d %d %d", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%d %d %d", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%d, %d, %d, %d)", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%d, %d, %d)", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "#%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "#%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "0x%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%.3f, %.3f, %.3f, %.3f", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "%.3ff, %.3ff, %.3ff, %.3ff", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%.3f, %.3f, %.3f, %.3f)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	tsPrintf(cpyTxt, "(%.3ff, %.3ff, %.3ff, %.3ff)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.Chr()))
		ImGui::SetClipboardText(cpyTxt.Chr());
	ImGui::EndPopup();
}


namespace ViewerMetaData
{
	int CopyRowIndex = -1;
}


void Viewer::ShowImageMetaDataOverlay(bool* popen)
{
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::MetaData);
	ImGui::SetNextWindowBgAlpha(0.90f);

	float rowHeight			= Gutil::GetUIParamScaled(18.0f, 2.5f) + 4.0f;
	float tagWidth			= Gutil::GetUIParamScaled(122.0f, 2.5f);
	float valWidth			= Gutil::GetUIParamScaled(200.0f, 2.5f);
	int maxRowsToDisplay	= Gutil::GetUIParamExtent(25, 18);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_AlwaysAutoResize	| ImGuiWindowFlags_NoSavedSettings	| ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoFocusOnAppearing	| ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Meta Data", popen, flags))
	{
		// Get meta data from current image.
		const tMetaData* metaData = CurrImage ? &CurrImage->Cached_MetaData : nullptr;
		uint32 tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter;
		int numDataRows = 1;
		if (metaData && metaData->IsValid())
			numDataRows = metaData->GetNumValidTags();
		const int numRowsToDisplay = tMin(maxRowsToDisplay, numDataRows);
		tVector2 outerSize = ImVec2(0.0f, rowHeight + rowHeight * float(numRowsToDisplay));
		if (ImGui::BeginTable("MetaDataTable", 2, tableFlags, outerSize))
		{
			ImGui::TableSetupColumn("Tag", ImGuiTableColumnFlags_WidthFixed, tagWidth);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, valWidth);
			ImGui::TableSetupScrollFreeze(0, 1); // Top row fixed.
			ImGui::TableHeadersRow();

			if (!metaData)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("No Image");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(" ");
			}
			else if (!metaData->IsValid())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("No Metadata In Image");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(" ");
			}
			else
			{
				bool colHovered = (ImGui::TableGetColumnFlags(0) & ImGuiTableColumnFlags_IsHovered) || (ImGui::TableGetColumnFlags(1) & ImGuiTableColumnFlags_IsHovered);
				if (colHovered  && (ImGui::GetMousePos().y >= ImGui::GetItemRectMin().y) && (ImGui::GetMousePos().y <= ImGui::GetItemRectMax().y))
				{
					ViewerMetaData::CopyRowIndex = -1;
					if (ImGui::IsMouseReleased(1))
						ImGui::OpenPopup("CopyPopup");
				}

				for (int tagIndex = 0; tagIndex < int(tMetaTag::NumTags); tagIndex++)
				{
					tMetaTag tag = tMetaTag(tagIndex);
					tString value = metaData->GetPrettyValue(tag);
					if (!value.IsValid())
						continue;

					tString tagName = tGetMetaTagName(tag);
					tString tagDesc = tGetMetaTagDesc(tag);
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text(tagName.Chr());
					Gutil::ToolTip(tagDesc.Chr());

					ImGui::TableSetColumnIndex(1);	
					ImGui::Text(value.Chr());

					tVector2 rect = ImGui::GetItemRectSize();
					// If the value was truncated, show it all in a tooltip.
					if (rect.x > (valWidth-16.0f))
						Gutil::ToolTip(value.Chr());

					if (ImGui::IsMouseReleased(1) && colHovered  && (ImGui::GetMousePos().y >= ImGui::GetItemRectMin().y) && (ImGui::GetMousePos().y <= ImGui::GetItemRectMax().y))
					{
						ViewerMetaData::CopyRowIndex = tagIndex;
						ImGui::OpenPopup("CopyPopup");
					}
				}

				if (ImGui::BeginPopup("CopyPopup"))
				{
					if (ViewerMetaData::CopyRowIndex != -1)
					{
						tMetaTag rowTag = tMetaTag(ViewerMetaData::CopyRowIndex);
						tString rowName = tGetMetaTagName(rowTag);

						tString copyRowText = "Copy " + rowName;
						if (ImGui::MenuItem(copyRowText.Chr(), nullptr, false, true))
						{
							tString rowCopyBuf = rowName + ": " + metaData->GetPrettyValue(rowTag);
							ImGui::SetClipboardText(rowCopyBuf.Chr());
						}
					}

					if (ImGui::MenuItem("Copy All",	nullptr, false, true))
					{
						tString copyAllBuf;
						for (int tagIndex = 0; tagIndex < int(tMetaTag::NumTags); tagIndex++)
						{
							tMetaTag tag = tMetaTag(tagIndex);
							tString value = metaData->GetPrettyValue(tag);
							if (!value.IsValid())
								continue;

							tString rowName = tGetMetaTagName(tag);
							copyAllBuf += rowName + ": " + value + "\n";
						}
						ImGui::SetClipboardText(copyAllBuf.Chr());
					}
					ImGui::EndPopup();
				}
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();
}
