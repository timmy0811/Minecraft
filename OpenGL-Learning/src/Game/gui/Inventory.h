#pragma once

#include <vector>
#include "Game/render/CustomRenderer.h"

namespace Minecraft {
	enum class ITEM_TYPE { BLOCK_STATIC_FULL, BLOCK_STATIC_HALF, BLOCK_LIQUID, FOOD, RESSOURCE };

	struct Item {
		unsigned int Id;
		ITEM_TYPE Type;
	};
}

class Inventory
{
public:
	explicit Inventory();
	~Inventory();

	void OnRender();
	void OnUpdate();

private:
	void OnRenderHotbar();

private:
	std::vector<Minecraft::Item*> m_ItemsStorage{ 4 * 9 };
	Minecraft::Helper::SpriteRenderer m_GUIRenderer;
	Minecraft::Helper::FontRenderer m_FontRendererHUD;

	// Sprites
	Minecraft::Helper::Sprite m_Sprite_Hotbar;
};