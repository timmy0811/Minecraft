#include "Inventory.h"

void Inventory::OnRenderHotbar()
{
	
}

Inventory::Inventory()
	:m_GUIRenderer(1, "res/shaders/sprite/shader_sprite.vert", "res/shaders/sprite/shader_sprite.frag"),
	m_FontRendererHUD("res/images/text/ascii_chat_1.png", "docs/font.yaml", 128, true, 1)
{
	// Hotbar
	Minecraft::Helper::Vec2_4 v;
	v.u0 = { 0.f / 527.f, 1.f - 413 / 445.f};		// left lower
	v.u1 = { 182.f / 527.f, 1.f - 413 / 445.f};	// right lower
	v.u2 = { 182.f / 527.f, 1.f - 391 / 445.f};	// right upper
	v.u3 = { 0.f / 527.f, 1.f - 391 / 445.f};		// left upper

	float w = conf.WIN_WIDTH * 0.45f * conf.GUI_SCALE;
	float h = w / 8.691f;
	m_Sprite_Hotbar = Minecraft::Helper::Sprite("res/images/hud/containers.png", { conf.WIN_WIDTH / 2.f - w / 2.f, 0 }, { w , h }, v);
	m_Sprite_Hotbar.Id = m_GUIRenderer.AddSprite(m_Sprite_Hotbar);
}

Inventory::~Inventory()
{
}

void Inventory::OnRender()
{
	m_GUIRenderer.Draw();
	m_FontRendererHUD.Draw();

	OnRenderHotbar();
}

void Inventory::OnUpdate()
{
	std::string item = "Cobblestone";
	static bool init = true;
	if (init) {
		float h = (conf.WIN_WIDTH * 0.45f * conf.GUI_SCALE) / 8.691f + 4.f;
		m_FontRendererHUD.PrintMultilineText(item.c_str(), { conf.WIN_WIDTH / 2.f - conf.WIN_WIDTH * 0.45f * conf.GUI_SCALE / 2.f, h }, 3.f, { 0.3f, 0.3f, 0.3f, 0.3f });
		init = false;
	}
}
