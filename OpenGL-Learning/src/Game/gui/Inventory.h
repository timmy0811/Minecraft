#pragma once

#include <vector>

#include "Game/render/CustomRenderer.h"
#include "Game/render/SpriteRenderer.h"

#include <GLFW/glfw3.h>

namespace Minecraft {
	enum class ITEM_TYPE { BLOCK_STATIC_FULL, BLOCK_STATIC_HALF, BLOCK_LIQUID, FOOD, RESSOURCE };

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

	void OnRender();
	void OnUpdate();
	void OnInput(GLFWwindow* window);

private:
	static void OnScrollCallback(GLFWwindow* window, double xpos, double ypos);

private:
	std::vector<Minecraft::Item*> m_ItemsStorage{ 4 * 9 };
	Minecraft::Helper::SpriteRenderer m_GUIRenderer;
	Minecraft::Helper::FontRenderer m_FontRendererHUD;

	inline static int m_HotbarItemPtr = 0;
	inline static bool MouseScrolled = false;

	// Sprites
	Minecraft::Helper::Sprite m_Sprite_Hotbar;
	Minecraft::Helper::Sprite m_Sprite_Hotbar_Highlight;
};