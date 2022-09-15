#pragma once
#include "imgui.h"

namespace stw
{
	constexpr std::size_t GRID_SIZE = 3;
	constexpr float DEFAULT_GRID_DRAW_SIZE = 500.0f;
	constexpr float LINE_THICKNESS = 3.0f;

	constexpr unsigned short SERVER_PORT = 8008;
	constexpr std::size_t PORT_BUFFER_SIZE = 10;
	constexpr std::size_t IP_BUFFER_SIZE = 45;
	constexpr ImGuiWindowFlags WINDOW_FULLSCREEN_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
}
