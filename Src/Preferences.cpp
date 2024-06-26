// Preferences.h
//
// Preferences window.
//
// Copyright (c) 2019-2024 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "Preferences.h"
#include "Config.h"
#include "Image.h"
#include "TacentView.h"
#include "GuiUtil.h"
#include "ThumbnailView.h"
#include "Version.cmake.h"
using namespace tMath;


void Viewer::DoCopyPastePreferences(bool reducedWidth)
{
	Config::ProfileData& profile	= Config::GetProfileData();
	float comboWidth				= reducedWidth ? Gutil::GetUIParamScaled(64.0f, 2.5f) : Gutil::GetUIParamScaled(100.0f, 2.5f);

	const char* fillColourPresetItems[] = { "User", "Black", "White", "Trans" };
	ImGui::SetNextItemWidth(comboWidth);
	int presetIndex = 0;
	if (profile.ClipboardCopyFillColour == tColour4b::black)
		presetIndex = 1;
	else if (profile.ClipboardCopyFillColour == tColour4b::white)
		presetIndex = 2;
	else if (profile.ClipboardCopyFillColour == tColour4b::transparent)
		presetIndex = 3;
	if (ImGui::Combo("Copy Fill", &presetIndex, fillColourPresetItems, tNumElements(fillColourPresetItems)))
	{
		switch (presetIndex)
		{
			case 1:		profile.ClipboardCopyFillColour = tColour4b::black;			break;
			case 2:		profile.ClipboardCopyFillColour = tColour4b::white;			break;
			case 3:		profile.ClipboardCopyFillColour = tColour4b::transparent;	break;
		}
	}

	ImGui::SameLine();
	tColour4f copyColour(profile.ClipboardCopyFillColour);
	if (ImGui::ColorEdit4("##CopyFillColour", copyColour.E, ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview))
	{
		profile.ClipboardCopyFillColour.Set(copyColour);
	}

	ImGui::SameLine();
	Gutil::HelpMark
	(
		"The copy fill colour is used when copying to the clipboard. Unselected\n"
		"channels will be filled with the corresponding component of this RGBA colour.\n"
		"If channel-intensity is selected in the intensity filter dialog, this fill colour\n"
		"is not used since the chosen intensity channel is spread into the RGB of\n"
		"the opaque clipboard image. The intensity channel may be one of R,G,B, or A."
	);

	ImGui::Checkbox("Paste Creates Image",		&profile.ClipboardPasteCreatesImage);
	ImGui::SameLine();
	Gutil::HelpMark
	(
		"If true a new image will be created when pasting from the clipboard.\n"
		"If false the clipboard contents will be pasted into the current image.\n"
		"When this is false only the selected channel filters are pasted. If\n"
		"intensity is selected, the intensity of the pasted image is copied into\n"
		"the single selected intensity channel which may be one of R, G, B, or A."
	);

	if (!profile.ClipboardPasteCreatesImage)
	{
		const char* pasteAnchorItems[] =
		{
			"TopL", "TopM", "TopR",
			"MidL", "MidM", "MidR",
			"BotL", "BotM", "BotR"
		};
		ImGui::SetNextItemWidth(comboWidth);
		ImGui::Combo("Paste Anchor", &profile.ClipboardPasteAnchor, pasteAnchorItems, tNumElements(pasteAnchorItems), tNumElements(pasteAnchorItems));
		ImGui::SameLine();
		Gutil::HelpMark
		(
			"This specifies where a pasted clipboard image will be pasted into the.\n"
			"current image in cases where the image dimensions don't match. You may\n"
			"choose one of 9 possible anchor positions. The most common choices are\n"
			"top-left (TopL), middle (MidM), and bottom-left (BotL)."
		);
	}
}


void Viewer::ShowPreferencesWindow(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	tVector2 windowPos = Gutil::GetDialogOrigin(Gutil::DialogID::Preferences);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	tString title;
	tsPrintf(title, "Preferences (%s Profile)", Config::GetProfileName());
	if (!ImGui::Begin(title.Chr(), popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	Config::ProfileData& profile = Config::GetProfileData();
	float buttonWidth	= Gutil::GetUIParamScaled(100.0f, 2.5f);
	float rightButtons	= Gutil::GetUIParamExtent(169.0f, 437.0f);

	bool tab = false;
	uint32 category = Config::Category_None;
	if (ImGui::BeginTabBar("PreferencesTabBar", ImGuiTabBarFlags_None))
	{
		tab = ImGui::BeginTabItem("Interface", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_Interface;
			float itemWidth					= Gutil::GetUIParamScaled(110.0f, 2.5f);
			float presetColourComboWidth	= Gutil::GetUIParamScaled(100.0f, 2.5f);
			ImGui::NewLine();

			ImGui::Checkbox("Always Show Filename", &profile.ShowNavFilenameAlways);
			ImGui::SameLine();
			Gutil::HelpMark
			(
				"When false the filename is only shown at the right of the nav-bar in\n"
				"fullscreen mode because the window title-bar is not visible. When set\n"
				"to true the filename is displayed there all the time."
			);

			ImGui::Checkbox("Transparent Work Area", &PendingTransparentWorkArea);
			#ifndef PACKAGE_SNAP
			if (PendingTransparentWorkArea != Config::Global.TransparentWorkArea)
			{
				ImGui::SameLine();
				ImGui::Text("(Restart)");
			}
			#else
			if (PendingTransparentWorkArea)
			{
				ImGui::SameLine();
				ImGui::Text("(No Snap Support)");
			}
			#endif

			ImGui::Checkbox("Background Extend", &profile.BackgroundExtend);

			const char* frameBufferBPCItems[] = { "8 BPC", "10 BPC", "12 BPC", "16 BPC" };
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::Combo("Frame Buffer", &PendingFrameBufferBPC, frameBufferBPCItems, tNumElements(frameBufferBPCItems));
			if (PendingFrameBufferBPC != Config::Global.FrameBufferBPC)
			{
				ImGui::SameLine();
				ImGui::Text("(Restart)");
			}

			ImGui::SameLine();
			Gutil::HelpMark
			(
				"Frame buffer bits per component. Requires restart to take effect.\n"
				"Generally to display HDR 10-bits or more is required. This value affects\n"
				"the number of available colours, not the gamut. The setting may also be\n"
				"used with SDR images. Requires restart to take effect.\n"
				"\n"
				"8 BPC : Also known as truecolor or 24 bit colour. 16.77 million colours.\n"
				"\n"
				"10 BPC : Also known as 30 bit colour. Good HDR monitors support this\n"
				"without frame-rate-control. FRC is a method of flashing different colours\n"
				"on an 8-BPC monitor to emulate additional shades. 1.07 billion colours.\n"
				"\n"
				"12 BPC : Also known as 36 bit colour. Only high-end monitors.\n"
				"\n"
				"16 BPC : Not supported directly by any display at this time.\n"
				"\n"
				"Check the output log to determine the achieved framebuffer bit-depth. The\n"
				"result will depend on whether your GPU supports the requested BPC.\n"
			);

			const char* onScreenControlsItems[] = { "Auto", "Always", "Never" };
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::Combo("On-Screen Controls", &profile.OnScreenControls, onScreenControlsItems, tNumElements(onScreenControlsItems));
			ImGui::SameLine();
			Gutil::HelpMark
			(
				"In auto mode the on-screen controls will appear when the mouse is\n"
				"moved and remain if the mouse is near or over a control. If the\n"
				"mouse is not moved for a period of time, the controls will auto-hide."
			);
			
			if (!Config::Global.TransparentWorkArea)
			{
				const char* backgroundItems[] = { "None", "Checker", "Solid" };
				ImGui::SetNextItemWidth(itemWidth);
				ImGui::Combo("Background Style", &profile.BackgroundStyle, backgroundItems, tNumElements(backgroundItems));

				if (profile.GetBackgroundStyle() == Config::ProfileData::BackgroundStyleEnum::SolidColour)
				{
					tColour4f floatCol(profile.BackgroundColour);
					if (ImGui::ColorEdit3("Solid Colour", floatCol.E, ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueBar))
					{
						profile.BackgroundColour.Set(floatCol);
						profile.BackgroundColour.A = 0xFF;
					}

					int preset = 0;
					if (profile.BackgroundColour == tColour4b::black)
						preset = 1;
					else if (profile.BackgroundColour == tColour4b::lightgrey)
						preset = 2;
					else if (profile.BackgroundColour == tColour4b::white)
						preset = 3;

					ImGui::SameLine();
					const char* presetColours[] = { "Custom", "Black", "Grey", "White" };
					ImGui::SetNextItemWidth(presetColourComboWidth);
					if (ImGui::Combo("Preset", &preset, presetColours, tNumElements(presetColours)))
					{
						switch (preset)
						{
							case 1:		profile.BackgroundColour = tColour4b::black;		break;
							case 2:		profile.BackgroundColour = tColour4b::lightgrey;	break;
							case 3:		profile.BackgroundColour = tColour4b::white;		break;
						}
					}
				}

				if (profile.GetBackgroundStyle() == Config::ProfileData::BackgroundStyleEnum::Checkerboard)
				{
					ImGui::PushItemWidth(itemWidth);
					ImGui::InputInt("Checker Size", &profile.BackgroundCheckerboxSize);
					ImGui::PopItemWidth();
					tMath::tiClamp(profile.BackgroundCheckerboxSize, 2, 256);
				}
			}

			// Reticle mode.
			const char* reticleModeItems[] = { "Always Hidden", "Always Visible", "On Select", "Auto Hide" };
			ImGui::SetNextItemWidth(itemWidth);
			ImGui::Combo("Reticle Mode", &profile.ReticleMode, reticleModeItems, tNumElements(reticleModeItems));
			ImGui::SameLine();
			Gutil::HelpMark
			(
				"Controls when the cursor reticle is visible.\n"
				"Always Hidden: Never display reticle. Driving blind.\n"
				"Always Visible: Never hide the reticle.\n"
				"On Select: Visible when click mouse. Hides when switch image or click outside image.\n"
				"Auto Hide: Hides after inactivity timeout."
			);

			const char* uiSizeItems[] = { "Auto", "Nano", "Tiny", "Small", "Moderate", "Medium", "Large", "Huge", "Massive" };
			tStaticAssert(tNumElements(uiSizeItems) == int(Config::ProfileData::UISizeEnum::NumSizes)+1);

			ImGui::SetNextItemWidth(itemWidth);
			int sizeInt = profile.UISize + 1;

			if (ImGui::Combo("UI Size", &sizeInt, uiSizeItems, tNumElements(uiSizeItems), tNumElements(uiSizeItems)))
			{
				profile.UISize = sizeInt - 1;
				Viewer::UpdateDesiredUISize();
			}

			if (sizeInt == 0)
			{
				ImGui::SameLine();
				tString currSizeStr;
				tsPrintf(currSizeStr, "(%s)", uiSizeItems[int(Viewer::CurrentUISize)+1]);
				ImGui::Text(currSizeStr.Chr());
			}

			ImGui::SameLine();
			Gutil::HelpMark("Overall size of UI widgets and font.\nIf set to 'auto' uses the OS scale setting.");

			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Slideshow", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_Slideshow;
			float inputWidth	= Gutil::GetUIParamScaled(110.0f, 2.5f);

			ImGui::NewLine();
			ImGui::SetNextItemWidth(inputWidth);
			if (ImGui::InputDouble("Period (s)", &profile.SlideshowPeriod, 0.001f, 1.0f, "%.3f"))
			{
				tiClampMin(profile.SlideshowPeriod, 1.0/60.0);
				Viewer::SlideshowCountdown = profile.SlideshowPeriod;
			}
			if (ImGui::Button("8s"))
			{
				profile.SlideshowPeriod = 8.0;
				Viewer::SlideshowCountdown = profile.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("4s"))
			{
				profile.SlideshowPeriod = 4.0;
				Viewer::SlideshowCountdown = profile.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("1s"))
			{
				profile.SlideshowPeriod = 1.0;
				Viewer::SlideshowCountdown = profile.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("10fps"))
			{
				profile.SlideshowPeriod = 1.0/10.0;
				Viewer::SlideshowCountdown = profile.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("30fps"))
			{
				profile.SlideshowPeriod = 1.0/30.0;
				Viewer::SlideshowCountdown = profile.SlideshowPeriod;
			}
			ImGui::SameLine();
			if (ImGui::Button("60fps"))
			{
				profile.SlideshowPeriod = 1.0/60.0;
				Viewer::SlideshowCountdown = profile.SlideshowPeriod;
			}

			ImGui::Checkbox("Countdown Indicator", &profile.SlideshowProgressArc);
			ImGui::SameLine();
			Gutil::HelpMark("Display a time remaining indicator when slideshow active.");

			ImGui::Checkbox("Auto Start", &profile.SlideshowAutoStart);
			ImGui::SameLine();
			Gutil::HelpMark("Should slideshow start automatically on launch.");

			ImGui::Checkbox("Looping", &profile.SlideshowLooping);
			ImGui::SameLine();
			Gutil::HelpMark("Should slideshow loop after completion.");

			Gutil::Separator();

			Viewer::DoSortParameters(false);
			ImGui::Checkbox("Auto Reshuffle", &profile.SlideshowAutoReshuffle);
			ImGui::SameLine();
			Gutil::HelpMark("If sort set to shuffle, reshuffle automatically after every loop.");

			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("System", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_System;
			float itemWidth			= Gutil::GetUIParamScaled(100.0f, 2.5f);
			float mipFiltWidth		= Gutil::GetUIParamScaled(144.0f, 2.5f);
			float sysButtonWidth	= Gutil::GetUIParamScaled(126.0f, 2.5f);
			ImGui::NewLine();

			ImGui::SetNextItemWidth(itemWidth);
			ImGui::InputInt("Max Undo Steps", &profile.MaxUndoSteps); ImGui::SameLine();
			Gutil::HelpMark("Maximum number of undo steps.");
			tMath::tiClamp(profile.MaxUndoSteps, 1, 32);

			ImGui::SetNextItemWidth(itemWidth);
			ImGui::InputInt("Max Mem (MB)", &profile.MaxImageMemMB); ImGui::SameLine();
			Gutil::HelpMark("Approx memory use limit of this app. Minimum 256 MB.");
			tMath::tiClampMin(profile.MaxImageMemMB, 256);

			ImGui::SetNextItemWidth(itemWidth);
			ImGui::InputInt("Max Cache Files", &profile.MaxCacheFiles); ImGui::SameLine();
			Gutil::HelpMark("Maximum number of cache files that may be created. Minimum 200.");
			tMath::tiClampMin(profile.MaxCacheFiles, 200);
			if (!DeleteAllCacheFilesOnExit)
			{
				if (ImGui::Button("Clear Cache On Exit", tVector2(sysButtonWidth, 0.0f)))
					DeleteAllCacheFilesOnExit = true;
				ImGui::SameLine(); Gutil::HelpMark("Cache will be cleared on exit.");
			}
			else
			{
				if (ImGui::Button("Cancel Clear Cache", tVector2(sysButtonWidth, 0.0f)))
					DeleteAllCacheFilesOnExit = false;
				ImGui::SameLine(); Gutil::HelpMark("Cache will no longer be cleared on exit.");
			}

			if (ImGui::Button("Reset Bookmarks", tVector2(sysButtonWidth, 0.0f)))
				tFileDialog::Reset();
			ImGui::SameLine(); Gutil::HelpMark("Reset File Dialog Bookmarks.");

			Gutil::Separator();

			ImGui::SetNextItemWidth(itemWidth);
			ImGui::InputFloat("Gamma##Monitor", &profile.MonitorGamma, 0.01f, 0.1f, "%.3f");
			ImGui::SameLine();
			Gutil::HelpMark("Some image property windows allow gamma correction and the gamma to be specified (eg. HDR DDS files).\nThis setting allows you to set a custom value for what the gamma will be reset to in those dialogs.\nResetting this tab always chooses the industry-standard gamm of 2.2");

			ImGui::Checkbox("Strict Loading", &profile.StrictLoading); ImGui::SameLine();
			Gutil::HelpMark
			(
				"Some image files are ill-formed. If strict is true these files are not loaded.\n"
				"Ill-formed jpg and dds files have been found in the wild that are ill-formed\n"
				"but still loadable. If strict is false, these files will still load."
			);

			// If the orient loading value changes we need to reload any images that have the Orientation tag set in their meta-data.
			// If the current image ends up not being unloaded, the 'Load' call exits immediately, so it's fast (i.e. it knows).
			if (ImGui::Checkbox("Meta Data Orient Loading", &profile.MetaDataOrientLoading))
			{
				for (Image* i = Images.First(); i; i = i->Next())
				{
					if (i->Filetype == tSystem::tFileType::JPG)
					{
						if (i->Cached_MetaData.IsValid() && i->Cached_MetaData[tImage::tMetaTag::Orientation].IsSet())
							i->Unload(true);
					}
					else
					{
						// This is not efficient but forces changes to the orient loading to be displayed correctly live
						// for types other than jpg (currently pvr needs this).
						for (Image* i = Images.First(); i; i = i->Next())
							i->Unload(true);
					}
				}

				CurrImage->Load();
			}
			ImGui::SameLine();
			Gutil::HelpMark("If Exif or other meta-data contains orientation information this will take it into account\nwhen loading and displays the image correctly oriented/flipped. Affects jpg/pvr files.");

			ImGui::Checkbox("Detect APNG Inside PNG", &profile.DetectAPNGInsidePNG); ImGui::SameLine();
			Gutil::HelpMark("Some png image files are really apng files. If detecton is true these png files will be displayed animated.");

			ImGui::Checkbox("Mipmap Chaining", &profile.MipmapChaining); ImGui::SameLine();
			Gutil::HelpMark("Chaining generates mipmaps faster. No chaining gives slightly\nbetter results at cost of large generation time.");

			ImGui::SetNextItemWidth(mipFiltWidth);
			ImGui::Combo("Mip Filter", &profile.MipmapFilter, tImage::tResampleFilterNames, 1+int(tImage::tResampleFilter::NumFilters), 1+int(tImage::tResampleFilter::NumFilters));
			ImGui::SameLine();
			Gutil::HelpMark("Filtering method to use when generating minification mipmaps.\nUse None for no mipmapping.");
	
			ImGui::EndTabItem();
		}

		tab = ImGui::BeginTabItem("Behaviour", nullptr, ImGuiTabItemFlags_NoTooltip);
		if (tab)
		{
			category = Config::Category_Behaviour;
			float itemWidth			= Gutil::GetUIParamScaled(100.0f, 2.5f);
			float comboWidth		= Gutil::GetUIParamScaled(120.0f, 2.5f);
			ImGui::NewLine();
			ImGui::Checkbox("Confirm Deletes",			&profile.ConfirmDeletes);
			ImGui::Checkbox("Confirm File Overwrites",	&profile.ConfirmFileOverwrites);
			ImGui::Checkbox("Auto Property Window",		&profile.AutoPropertyWindow);
			ImGui::Checkbox("Auto Play Anims",			&profile.AutoPlayAnimatedImages);
			ImGui::Checkbox("Zoom Per Image",			&profile.ZoomPerImage);

			Gutil::Separator();

			DoCopyPastePreferences();

			if (profile.ClipboardPasteCreatesImage)
			{
				tString pasteTypeName = profile.ClipboardPasteFileType;
				tSystem::tFileType pasteType = tSystem::tGetFileTypeFromName(pasteTypeName);
				ImGui::SetNextItemWidth(itemWidth);
				if (ImGui::BeginCombo("Paste Type", pasteTypeName.Chr()))
				{
					for (tSystem::tFileTypes::tFileTypeItem* item = FileTypes_ClipboardPaste.First(); item; item = item->Next())
					{
						tSystem::tFileType ft = item->FileType;
						bool selected = (ft == pasteType);

						tString ftName = tGetFileTypeName(ft);
						if (ImGui::Selectable(ftName.Chr(), &selected))
							profile.ClipboardPasteFileType = ftName;

						if (selected)
							ImGui::SetItemDefaultFocus();
					}				
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				Gutil::HelpMark
				(
					"When an image is pasted from the clipboard it creates a new image of this type.\n"
					"Valid types are ones that are lossless or support lossless encoding like webp.\n"
					"Pasted images support alpha channel. If no alpha it saves the image without it." 
				);
			}

			ImGui::SetNextItemWidth(itemWidth);
			int roll[2] = { profile.ClipboardPasteRollH, profile.ClipboardPasteRollV };
			if (ImGui::InputInt2("Paste Roll", roll))
			{
				tiClamp(roll[0], -16383, 16384);
				tiClamp(roll[1], -16383, 16384);
				profile.ClipboardPasteRollH = roll[0];
				profile.ClipboardPasteRollV = roll[1];
			}
			ImGui::SameLine();
			Gutil::HelpMark
			(
				"This may be used if when pasting an image the pixels are not\n"
				"aligned properly. The first integer rolls the image horizontally\n"
				"when pasting, the second rolls vertically. Negatives are allowed."
			);

			if (!profile.ZoomPerImage)
			{
				Gutil::Separator();
				const char* zoomModes[] = { "User", "Fit", "Downscale", "OneToOne" };
				ImGui::PushItemWidth(comboWidth);
				int zoomMode = int(GetZoomMode());
				if (ImGui::Combo("Zoom Mode", &zoomMode, zoomModes, tNumElements(zoomModes)))
				{
					switch (Config::ProfileData::ZoomModeEnum(zoomMode))
					{
						case Config::ProfileData::ZoomModeEnum::Fit:
						case Config::ProfileData::ZoomModeEnum::DownscaleOnly:
							ResetPan();
							break;
						case Config::ProfileData::ZoomModeEnum::OneToOne:
							SetZoomPercent(100.0f);
							ResetPan();
							break;
					}
					SetZoomMode( Config::ProfileData::ZoomModeEnum(zoomMode) );
				}
				ImGui::PopItemWidth();
				ImGui::SameLine();
				Gutil::HelpMark
				(
					"Controls what zoom to use when displaying images.\n"
					"User: User-specified. This mode is automatically turned on when zooming in/out.\n"
					"Fit: Image is zoomed to fit display area no matter its size.\n"
					"Downscale: Shows it at 100% zoom unless image is too big and needs downscaling.\n"
					"  This is the default. It keeps the full image always visible.\n"
					"OneToOne: One image pixel takes up one screen pixel."
				);

				float zoom = Viewer::GetZoomPercent();
				ImGui::PushItemWidth(comboWidth);
				if (ImGui::InputFloat("Zoom Percent", &zoom, 0.01f, 0.1f, "%.3f"))
					Viewer::SetZoomPercent(zoom);
				ImGui::PopItemWidth();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	Gutil::Separator();

	if (ImGui::Button("Reset Profile", tVector2(buttonWidth, 0.0f)))
	{
		Config::ResetProfile(Config::Category_AllNoBindings);
		Viewer::UpdateDesiredUISize();
		SlideshowCountdown = profile.SlideshowPeriod;
	}
	Gutil::ToolTip
	(
		"Resets the current profile (excluding key-bindings) to defaults.\n"
		"Key-bindings may be reset from the Key Bindings window."
	);

	ImGui::SameLine();
	ImGui::SetCursorPosX(rightButtons);
	if (ImGui::Button("Reset Tab", tVector2(buttonWidth, 0.0f)))
	{
		Config::ResetProfile(category);
		if (category == Config::Category_Interface)
			Viewer::UpdateDesiredUISize();
		SlideshowCountdown = profile.SlideshowPeriod;
	}
	Gutil::ToolTip("Resets the current tab/category for the current profile (what you see above).");

	if (ImGui::Button("Reset All", tVector2(buttonWidth, 0.0f)))
	{
		Config::ResetAllProfiles(Config::Category_AllNoBindings);
		Config::Global.Reset();
		Config::SetProfile(Profile::Main);
		Viewer::UpdateDesiredUISize();

		// @todo These two are global. Reset tab does not work properly with them.
		// If the global reset turns transparent work area off (the default) we can always safely clear the pending.
		if (!Config::Global.TransparentWorkArea)
			PendingTransparentWorkArea = false;

		// Similarly for framebuffer BPC.
		if (Config::Global.GetFrameBufferBPC() == Config::GlobalData::FrameBufferBPCEnum::BPC_Default)
			PendingFrameBufferBPC = int(Config::GlobalData::FrameBufferBPCEnum::BPC_Default);

		SlideshowCountdown = profile.SlideshowPeriod;
		ChangeScreenMode(profile.FullscreenMode, true);
	}
	Gutil::ToolTip
	(
		"Resets all profiles (excluding key bindings) to their default settings and switches\n"
		"to the main profile. Keybindings may be reset from the Key Bindings window."
	);

	ImGui::SameLine();
	ImGui::SetCursorPosX(rightButtons);
	if (ImGui::Button("Close", tVector2(buttonWidth, 0.0f)))
	{
		if (popen)
			*popen = false;
	}
	ImGui::End();
}
