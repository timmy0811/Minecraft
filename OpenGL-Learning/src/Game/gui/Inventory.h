#pragma once

#include <vector>

#include "Game/render/CustomRenderer.h"
#include "Game/render/SpriteRenderer.h"

#include <GLFW/glfw3.h>

namespace Minecraft {
	enum class ITEM_TYPE { BLOCK_STATIC_FULL, BLOCK_STATIC_HALF, BLOCK_LIQUID, FOOD, RESSOURCE };
	enum class GUI_STATE { HOTBAR, INVENTORY };

	struct Item {
		unsigned int Id;
		unsigned int count;
		ITEM_TYPE Type;
	};
}

class Inventory
{
public:
	explicit Inventory();
	~Inventory();

	unsigned int GetHoldingItemID() const;
	inline const Minecraft::GUI_STATE GetGUIState() const { return m_State; }

	void OnRender();
	void OnUpdate();
	void OnInput(GLFWwindow* window);

private:
	static void OnScrollCallback(GLFWwindow* window, double xpos, double ypos);
	void ToggleGUIState(Minecraft::GUI_STATE state, GLFWwindow* window);
	void PushSprites();

private:
	std::vector<Minecraft::Item*> m_ItemsStorage{ 4 * 9 };
	Minecraft::Helper::SpriteRenderer m_GUIRenderer;
	Minecraft::Helper::FontRenderer m_FontRendererHUD;

	Minecraft::GUI_STATE m_State = Minecraft::GUI_STATE::HOTBAR;

	inline static int m_HotbarItemPtr = 0;
	inline static bool MouseScrolled = false;

	// Sprites
	Minecraft::Helper::Sprite m_Sprite_Hotbar;
	Minecraft::Helper::Sprite m_Sprite_Inventory;
	Minecraft::Helper::Sprite m_Sprite_Hotbar_Highlight;
};