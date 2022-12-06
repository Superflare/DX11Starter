#pragma once

#include <vector>
#include <memory>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "Camera.h"
#include "GameEntity.h"
#include "Lights.h"

namespace ImGuiMenus
{
	void WindowStats(int windowWidth, int windowHeight);
	void EditScene(
		std::shared_ptr<Camera> cam,
		std::vector<std::shared_ptr<GameEntity>> entities,
		std::vector<std::shared_ptr<Material>> materials,
		std::vector<Light>* lights
	);

	static bool showUiDemoWindow = false;
}
