//------------------------------------------------------------------------------
// errors.h
//------------------------------------------------------------------------------
#pragma once

// error code constants
////////////////////////////////////////////////////////////////////////////////
namespace ERRCODE
{
	constexpr int USER_EXIT				= 0;
	constexpr int FILE_NOT_OPEN			= -1;
	constexpr int FONT_NOT_LOADED		= -2;
	constexpr int WINDOW_CLOSED			= -3;
	constexpr int IMGUI_SFML_INIT_FAIL	= -4;
}
