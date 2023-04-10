#include "Inventory.h"

void Inventory::OnRenderHotbar()
{
	
}

Inventory::Inventory()
	//:m_GUIRenderer(1, "res/shaders/sprite/shader_sprite.vert", "res/shaders/sprite/shader_sprite.frag")
{
	// Hotbar
	// m_Sprite_Hotbar = Minecraft::Helper::Sprite("res/images/hud/containers.png", { 100.f, 100.f }, { 500.f, 200.f });
	//m_Sprite_Hotbar.Id = m_GUIRenderer.AddSprite("res/images/hud/crossair.png", { conf.WIN_WIDTH / 2.f - 80.f, conf.WIN_HEIGHT / 2.f - 80.f }, { 16, 16 });
}

Inventory::~Inventory()
{
}

void Inventory::OnRender()
{
	//m_GUIRenderer.Draw();
	OnRenderHotbar();
}
