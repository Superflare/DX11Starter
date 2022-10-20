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

void ImGuiMenus::EditScene(std::shared_ptr<Camera> cam, std::vector<std::shared_ptr<GameEntity>> entities,
	std::vector<Light>* lights, DirectX::XMFLOAT3* ambientColor)
{
	ImGui::Begin("Edit Scene");

	if (ImGui::BeginTabBar("Scene Components"))
	{
		if (ImGui::BeginTabItem("Cameras"))
		{
			ImGui::Spacing();

			// Transform values
			XMFLOAT3 pos = cam->GetTransform()->GetPosition();
			XMFLOAT3 rot = Rad2DegFromVector(cam->GetTransform()->GetRotationPitchYawRoll());

			if (ImGui::DragFloat3("Position", &pos.x, 0.01f))
				cam->GetTransform()->SetPosition(pos);
			
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
			ImGui::Spacing();

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

		if (ImGui::BeginTabItem("Lights"))
		{
			ImGui::Spacing();

			if (ImGui::TreeNode("Ambient"))
			{
				ImGui::ColorPicker3("Ambient Color", &ambientColor->x, ImGuiColorEditFlags_PickerHueWheel);

				ImGui::TreePop();
			}

			ImGui::Spacing();
			for (int i = 0; i < lights->size(); i++)
			{
				ImGui::PushID(i);
				if (ImGui::TreeNode("Light Node", "Light %d", i))
				{
					ImGui::LabelText("Type", LightTypeToString((*lights)[i].type));

					XMFLOAT3 color = (*lights)[i].color;
					XMFLOAT3 direction = (*lights)[i].direction;
					XMFLOAT3 position = (*lights)[i].position;
					float range = (*lights)[i].range;
					float intensity = (*lights)[i].intensity;
					float spotFalloff = (*lights)[i].spotFalloff;

					ImGui::Spacing();
					if (ImGui::ColorPicker3("Color", &color.x, ImGuiColorEditFlags_PickerHueWheel))
					{
						(*lights)[i].color = color;
					}
					ImGui::Spacing();

					if ((*lights)[i].type == LIGHT_TYPE_DIRECTIONAL)
					{
						if (ImGui::DragFloat3("Direction", &direction.x, 0.01f))
						{
							XMStoreFloat3(&(*lights)[i].direction, XMVector3Normalize(XMVectorSet(direction.x, direction.y, direction.z, 0)));
						}
					}
					else if ((*lights)[i].type == LIGHT_TYPE_POINT)
					{
						if (ImGui::DragFloat3("Position", &position.x, 0.01f))
						{
							(*lights)[i].position = position;
						}

						if (ImGui::DragFloat("Range", &range, 0.01f, 0, ImGuiSliderFlags_ClampOnInput))
						{
							(*lights)[i].range = range;
						}
					}
					else if ((*lights)[i].type == LIGHT_TYPE_POINT)
					{
						if (ImGui::DragFloat("Spot Falloff", &spotFalloff, 0.01f))
						{
							(*lights)[i].spotFalloff = spotFalloff;
						}
					}

					if (ImGui::DragFloat("Intensity", &intensity, 0.01f))
					{
						(*lights)[i].intensity = intensity;
					}

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
