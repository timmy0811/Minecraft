#include "Inventory.h"

#define highlightscale  1.045

Inventory::Inventory()
	:m_GUIRenderer(32, { 4, 5 }, "res/shaders/sprite/shader_sprite.vert", "res/shaders/sprite/shader_sprite.frag"),
	m_FontRendererHUD("res/images/text/ascii_chat_1.png", "docs/font.yaml", 128, true, 1)
{
	PushSprites();

	m_ItemsStorage[0] = new Minecraft::Item{ 0, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
	m_ItemsStorage[1] = new Minecraft::Item{ 1, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
	m_ItemsStorage[2] = new Minecraft::Item{ 2, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
	m_ItemsStorage[3] = new Minecraft::Item{ 3, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
	m_ItemsStorage[4] = new Minecraft::Item{ 4, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
	m_ItemsStorage[5] = new Minecraft::Item{ 5, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
	m_ItemsStorage[6] = new Minecraft::Item{ 6, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
	m_ItemsStorage[7] = new Minecraft::Item{ 7, 1, Minecraft::ITEM_TYPE::BLOCK_STATIC_FULL };
}

Inventory::~Inventory()
{
	for (Minecraft::Item* item : m_ItemsStorage) {
		delete item;
	}
}

unsigned int Inventory::GetHoldingItemID() const
{
	return m_ItemsStorage[m_HotbarItemPtr] ? m_ItemsStorage[m_HotbarItemPtr]->Id : 9999;
}

void Inventory::OnScrollCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (ypos < 0.f) {
		m_HotbarItemPtr++;
		MouseScrolled = true;
	}
	else if (ypos > 0.f) {
		m_HotbarItemPtr--;
		MouseScrolled = true;
	}

	if (m_HotbarItemPtr >= 9) m_HotbarItemPtr = 0;
	else if (m_HotbarItemPtr < 0) m_HotbarItemPtr = 8;
}

void Inventory::ToggleGUIState(Minecraft::GUI_STATE state, GLFWwindow* window)
{
	switch (state) {
	case Minecraft::GUI_STATE::INVENTORY:
		m_GUIRenderer.PopAll();
		m_GUIRenderer.PushSprite(m_Sprite_Inventory.Id);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case Minecraft::GUI_STATE::HOTBAR:
		m_GUIRenderer.PopAll();
		m_GUIRenderer.PushSprite(m_Sprite_Hotbar_Highlight.Id);
		m_GUIRenderer.PushSprite(m_Sprite_Hotbar.Id);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	}

	m_State = state;
}

void Inventory::PushSprites()
{
	Minecraft::Helper::Vec2_4 v;

	// Hotbar_Highlight
	v.u0 = { 0.001894, 0.017937 };	// 3
	v.u1 = { 0.043561, 0.017937 };	// 2
	v.u2 = { 0.043561, 0.071749 };	// 1
	v.u3 = { 0.001894, 0.071749 };	// 0

	float w = conf.WIN_WIDTH * 0.45f * conf.GUI_SCALE;
	float h = w / 8.691f;

	float scaleOffset = (h * highlightscale - h) / 2.f;
	m_Sprite_Hotbar_Highlight = Minecraft::Helper::Sprite("res/images/hud/containers.png", { conf.WIN_WIDTH / 2.f - w / 2.f - scaleOffset , -scaleOffset }, { h * highlightscale , h * highlightscale }, v, true);
	m_Sprite_Hotbar_Highlight.Id = m_GUIRenderer.PushSprite(m_Sprite_Hotbar_Highlight);

	// Hotbar
	v.u0 = { 0.f, 0.071749 };		// left lower
	v.u1 = { 0.344697, 0.071749 };	// right lower
	v.u2 = { 0.344697, 0.121076 };	// right upper
	v.u3 = { 0.f, 0.121076 };		// left upper

	m_Sprite_Hotbar = Minecraft::Helper::Sprite("res/images/hud/containers.png", { conf.WIN_WIDTH / 2.f - w / 2.f, 0 }, { w , h }, v, true);
	m_Sprite_Hotbar.Id = m_GUIRenderer.PushSprite(m_Sprite_Hotbar);

	// Inventory
	v.u0 = { 0.f, 0.627803f };		// left lower
	v.u1 = { 0.33333f, 0.627803f };	// right lower
	v.u2 = { 0.33333f, 1.f };	// right upper
	v.u3 = { 0.f, 1.f };		// left upper

	h = conf.WIN_HEIGHT * 0.75f * conf.GUI_SCALE;
	w = h;

	m_Sprite_Inventory = Minecraft::Helper::Sprite("res/images/hud/containers.png", { conf.WIN_WIDTH / 2.f - w / 2.f, (conf.WIN_HEIGHT - h) / 2.f }, { w , h }, v, true);
	m_Sprite_Inventory.Id = m_GUIRenderer.PushSprite(m_Sprite_Inventory);
	m_GUIRenderer.PopSprite(m_Sprite_Inventory.Id);
}

void Inventory::OnRender()
{
	m_GUIRenderer.Draw();
	m_FontRendererHUD.Draw();
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

void Inventory::OnInput(GLFWwindow* window)
{
	glfwSetScrollCallback(window, OnScrollCallback);

	if (MouseScrolled) {
		float w = conf.WIN_WIDTH * 0.45f * conf.GUI_SCALE;
		float h = w / 8.691f;

		float scaleOffset = (h * highlightscale - h) / 2.f;
		m_GUIRenderer.SetSpritePosition({ conf.WIN_WIDTH / 2.f - w / 2.f - scaleOffset + (w / 9.f) * m_HotbarItemPtr , -scaleOffset }, m_Sprite_Hotbar_Highlight.Id);

		MouseScrolled = false;
	}

	static bool keyPressedE = false;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !keyPressedE)
	{
		keyPressedE = true;
		if (m_State == Minecraft::GUI_STATE::HOTBAR) ToggleGUIState(Minecraft::GUI_STATE::INVENTORY, window);
		else ToggleGUIState(Minecraft::GUI_STATE::HOTBAR, window);
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE && keyPressedE) keyPressedE = false;
}