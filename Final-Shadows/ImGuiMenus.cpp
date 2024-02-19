#include <DirectXMath.h>
#include "ImGuiMenus.h"
#include "Helpers.h"
using namespace DirectX;

// ------------------------------------------------------------------
// Dislpay the program status in a small window
// ------------------------------------------------------------------
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

// ------------------------------------------------------------------
// Provide runtime tools to edit the precreated rendered scene
// ------------------------------------------------------------------
void ImGuiMenus::EditScene(
	const std::shared_ptr<Camera>& cam,
	std::vector<std::shared_ptr<GameEntity>>& entities,
	const std::vector<std::shared_ptr<Material>>& materials,
	std::vector<Light>* lights,
	const Microsoft::WRL::ComPtr<ID3D11Device>& device,
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context
	)
{
	ImGui::Begin("Edit Scene");

	if (ImGui::BeginTabBar("Scene Components"))
	{
		// Give camera-specific editing options
		if (ImGui::BeginTabItem("Scene Camera"))
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

		// Allow for transformation changes of game entities within the world
		if (ImGui::BeginTabItem("Entities"))
		{
			// Create new entities from a list of basic shapes
			if (ImGui::Button("Add New.."))
				ImGui::OpenPopup("new entity popup");
			
			if (ImGui::BeginPopup("new entity popup"))
			{
				ImGui::SeparatorText("Basic Shapes");
				ImGui::SeparatorText("3D");

				if (ImGui::Selectable("Cube"))
				{
					std::shared_ptr<Mesh> m = std::make_shared<Mesh>("../../Assets/Models/cube.obj", device, context);
					entities.push_back(std::make_shared<GameEntity>(m, materials[0]));
				}

				if (ImGui::Selectable("Sphere"))
				{
					std::shared_ptr<Mesh> m = std::make_shared<Mesh>("../../Assets/Models/sphere.obj", device, context);
					entities.push_back(std::make_shared<GameEntity>(m, materials[0]));
				}

				if (ImGui::Selectable("Cylinder"))
				{
					std::shared_ptr<Mesh> m = std::make_shared<Mesh>("../../Assets/Models/cylinder.obj", device, context);
					entities.push_back(std::make_shared<GameEntity>(m, materials[0]));
				}

				if (ImGui::Selectable("Torus"))
				{
					std::shared_ptr<Mesh> m = std::make_shared<Mesh>("../../Assets/Models/torus.obj", device, context);
					entities.push_back(std::make_shared<GameEntity>(m, materials[0]));
				}

				if (ImGui::Selectable("Helix"))
				{
					std::shared_ptr<Mesh> m = std::make_shared<Mesh>("../../Assets/Models/helix.obj", device, context);
					entities.push_back(std::make_shared<GameEntity>(m, materials[0]));
				}

				ImGui::SeparatorText("2D");

				if (ImGui::Selectable("Quad"))
				{
					std::shared_ptr<Mesh> m = std::make_shared<Mesh>("../../Assets/Models/quad.obj", device, context);
					entities.push_back(std::make_shared<GameEntity>(m, materials[0]));
				}

				if (ImGui::Selectable("Quad Double Sided"))
				{
					std::shared_ptr<Mesh> m = std::make_shared<Mesh>("../../Assets/Models/quad_double_sided.obj", device, context);
					entities.push_back(std::make_shared<GameEntity>(m, materials[0]));
				}

				ImGui::EndPopup();
			}

			ImGui::Spacing();

			// Manipulate entities already in the scene
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

					// Delete entity
					ImGui::Spacing();
					if (ImGui::Button("Delete"))
						entities.erase(entities.begin() + i);
					ImGui::Spacing();

					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			ImGui::EndTabItem();
		}

		// Display materials used by entities in the scene and allow for PBR property changes
		if (ImGui::BeginTabItem("Materials"))
		{
			ImGui::Spacing();

			for (int i = 0; i < materials.size(); i++)
			{
				ImGui::PushID(i);
				if (ImGui::TreeNode("Material Node", materials[i]->GetName() == "" ? "Material %d" : materials[i]->GetName(), i))
				{
					XMFLOAT4 colorTint = materials[i]->GetColorTint();
					float roughness = materials[i]->GetRoughness();
					float metallic = materials[i]->GetMetallic();
					float texScale = materials[i]->GetTextureScale();
					XMFLOAT2 texOffset = materials[i]->GetTextureOffset();

					// Color Tint
					if (ImGui::ColorPicker3("Color Tint", &colorTint.x))
					{
						materials[i]->SetColorTint(colorTint);
					}

					ImGui::Spacing();


					// Roughness
					if (roughness == -1)
						ImGui::BeginDisabled();
					
					if (ImGui::DragFloat("Roughness", &roughness, 0.005f, 0, 1))
					{
						materials[i]->SetRoughness(roughness);
					}

					if (roughness == -1)
						ImGui::EndDisabled();

					// Metallic
					if (metallic == -1)
						ImGui::BeginDisabled();

					if (ImGui::DragFloat("Metallic", &metallic, 0.005f, 0, 1))
					{
						materials[i]->SetMetallic(metallic);
					}

					if (metallic == -1)
						ImGui::EndDisabled();

					// Texture Scale
					if (ImGui::DragFloat("Texture Scale", &texScale, 0.01f, 0.01f, D3D11_FLOAT32_MAX))
					{
						materials[i]->SetTextureScale(texScale);
					}

					// Texture Offset
					if (ImGui::DragFloat2("Texture Offset", &texOffset.x, 0.01f))
					{
						materials[i]->SetTextureOffset(texOffset);
					}

					ImGui::TreePop();
				}

				ImGui::PopID();
			}

			ImGui::EndTabItem();
		}

		// Display all the lights used in the scene and edit their transforms and light properties
		// - Note: Lights' transforms can only be edited when "Cast Shadows" is turned off on that light.
		//         After a transform change, shadows can be re-enabled to work properly again
		if (ImGui::BeginTabItem("Lights"))
		{
			ImGui::Spacing();
			
			for (int i = 0; i < lights->size(); i++)
			{
				ImGui::PushID(i);
				if (ImGui::TreeNode("Light Node", "Light %d", i))
				{
					ImGui::LabelText("Type", LightTypeToString((*lights)[i].type));

					XMFLOAT3 color = (*lights)[i].color;
					XMFLOAT3 direction = (*lights)[i].direction;
					XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));
					XMFLOAT3 position = (*lights)[i].position;
					float range = (*lights)[i].range;
					float intensity = (*lights)[i].intensity;
					float spotFalloff = (*lights)[i].spotFalloff;
					bool castsShadows = (bool)(*lights)[i].castsShadows;

					ImGui::Spacing();
					if (ImGui::ColorPicker3("Color", &color.x, ImGuiColorEditFlags_PickerHueWheel))
					{
						(*lights)[i].color = color;
					}
					ImGui::Spacing();

					if (castsShadows)
						ImGui::BeginDisabled();

					// Only display the light properties that will have an effect on the selected light
					// (i.e. Point lights and spot lights have light falloff that directional lights don't have)
					if ((*lights)[i].type == LIGHT_TYPE_DIRECTIONAL || LIGHT_TYPE_SPOT)
					{
						if (ImGui::DragFloat3("Direction", &direction.x, 0.01f))
						{
							XMStoreFloat3(&(*lights)[i].direction, XMVector3Normalize(XMVectorSet(direction.x, direction.y, direction.z, 0)));
						}
					}
					
					if ((*lights)[i].type == LIGHT_TYPE_POINT || LIGHT_TYPE_SPOT)
					{
						if (ImGui::DragFloat3("Position", &position.x, 0.01f))
						{
							(*lights)[i].position = position;
						}

						if (ImGui::DragFloat("Range", &range, 0.01f, 0, D3D11_FLOAT32_MAX))
						{
							(*lights)[i].range = range;
						}
					}

					if ((*lights)[i].type == LIGHT_TYPE_SPOT)
					{
						if (ImGui::DragFloat("Spot Falloff", &spotFalloff, 0.01f))
						{
							(*lights)[i].spotFalloff = spotFalloff;
						}
					}

					if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0, D3D11_FLOAT32_MAX))
					{
						(*lights)[i].intensity = intensity;
					}

					if (castsShadows)
						ImGui::EndDisabled();

					if (ImGui::Checkbox("Casts Shadows?", &castsShadows))
					{
						(*lights)[i].castsShadows = castsShadows;
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
