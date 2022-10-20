#include <DirectXMath.h>
#include "ImGuiMenus.h"
#include "Helpers.h"
using namespace DirectX;

void ImGuiMenus::WindowStats(int windowWidth, int windowHeight)
{
	ImGui::Begin("Window Stats");

	ImGui::Text("Frames per second: %f", ImGui::GetIO().Framerate);
	ImGui::Text("Individual frame time: %fms", 1.0f / ImGui::GetIO().Framerate * 1000.0f);
	ImGui::Text("Window size: %dx%d", windowWidth, windowHeight);

	ImGui::Spacing();

	if (ImGui::Button(ImGuiMenus::showUiDemoWindow ? "Hide ImGui demo window" : "Show ImGui demo window"))
		ImGuiMenus::showUiDemoWindow = !ImGuiMenus::showUiDemoWindow;

	if (ImGuiMenus::showUiDemoWindow)
	{
		ImGui::ShowDemoWindow();
	}

	ImGui::End();
}

void ImGuiMenus::EditScene(std::shared_ptr<Camera> cam, std::vector<std::shared_ptr<GameEntity>> entities)
{
	ImGui::Begin("Edit Scene");

	if (ImGui::BeginTabBar("Scene Components"))
	{
		if (ImGui::BeginTabItem("Cameras"))
		{
			// Transform values
			XMFLOAT3 pos = cam->GetTransform()->GetPosition();
			XMFLOAT3 rot = Rad2DegFromVector(cam->GetTransform()->GetRotationPitchYawRoll());

			if (ImGui::DragFloat3("Position", &pos.x, 0.01f))
				cam->GetTransform()->SetPosition(pos);

			// Rotation will need to be changed to using euler angles and floating point numbers aren't precise
			// enough to constantly calculate the euler angles from the current quaternion rotation
			if (ImGui::DragFloat3("Rotation (Degrees)", &rot.x, 0.6f))
			{
				// Clamp the rotation so the camera can't be rotated upside-down
				if (rot.x > 90.0f)
				{
					rot.x = 89.9f;
				}
				else if (rot.x < -90.0f)
				{
					rot.x = -89.9f;
				}
				cam->GetTransform()->SetRotation(Deg2RadFromVector(rot));
			}

			// Clip planes
			float nearClip = cam->GetNearClip();
			float farClip = cam->GetFarClip();
			if (ImGui::DragFloat("Near clip plane", &nearClip, 0.01f, 0.001f, 100.0f))
				cam->SetNearClip(nearClip);
			if (ImGui::DragFloat("Far clip plane", &farClip, 1.0f, 10.0f, 1000.0f))
				cam->SetFarClip(farClip);
			
			// Field of view
			float fov = cam->GetFov() * 180.0f / XM_PI;
			if (ImGui::SliderFloat("Field of view", &fov, 0.01f, 180.0f))
				cam->SetFov(fov * XM_PI / 180.0f);
			

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Entities"))
		{
			for (int i = 0; i < entities.size(); i++)
			{
				ImGui::PushID(i);
				if (ImGui::TreeNode("Entity Node", "Entity %d", i))
				{
					// Transform values
					Transform* transform = entities[i]->GetTransform();
					XMFLOAT3 pos = transform->GetPosition();
					XMFLOAT3 rot = Rad2DegFromVector(transform->GetRotationPitchYawRoll());
					XMFLOAT3 scale = transform->GetScale();

					if (ImGui::DragFloat3("Position", &pos.x, 0.01f))
						transform->SetPosition(pos);

					if (ImGui::DragFloat3("Rotation (Degrees)", &rot.x, 0.6f))
					{
						// Clamp the rotation so the calculations of the euler angles work properly
						if (rot.x > 90.0f)
						{
							rot.x = 89.9f;
						}
						else if (rot.x < -90.0f)
						{
							rot.x = -89.9f;
						}
						transform->SetRotation(Deg2RadFromVector(rot));
					}

					if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
						transform->SetScale(scale);

					// Mesh details
					ImGui::Spacing();
					ImGui::Text("Mesh index count: %d", entities[i]->GetMesh()->GetIndexCount());


					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}
