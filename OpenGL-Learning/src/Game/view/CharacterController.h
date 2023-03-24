#pragma once

#include "Game/world/Chunk.h"

#include <GLFW/glfw3.h>

#include "Game/view/Camera.h"

namespace Minecraft::CharacterController {
	enum class STATE { SPRINTING, WALKING, CROUCHING, FLYING };

	struct FOVchangeEvent {
		bool doChange = false;
		float FOV = 0.f;
	};
}

class CharacterController
{
private:
	Minecraft::Camera3D m_Camera;
	Minecraft::CharacterController::STATE m_State;
	bool m_IsGrounded;
	bool m_IsSpawned;

	float m_MovementSpeed;

	glm::vec3 m_FrameVelocity;
	glm::vec3 m_BlockPosition; // Relative to Matrix World
	glm::vec3 m_BlockPositionInChunk;
	std::vector<glm::vec3> m_CheckOrder;
	Minecraft::Block_static* m_SelectedBlock;

	float m_CharBodyHeight = 1.62f;
	float m_CharHeadHeight = 0.18f;
	float m_CharWidth = 0.6f;

	// Reference Values
	unsigned int r_ChunkWidth = (unsigned int)(conf.CHUNK_SIZE * conf.BLOCK_SIZE);
	glm::vec2 r_RootPosition = { conf.WORLD_WIDTH / 2, conf.WORLD_WIDTH / 2 };
	glm::vec3 r_WorldRootPosition = { -(int)((r_RootPosition.x + 0.5) * r_ChunkWidth), 0, -(int)((r_RootPosition.y + 0.5) * r_ChunkWidth) };

	// Input
	float m_LastX = 400, m_LastY = 300;
	bool m_FirstMouseInit = true;

	inline static float s_MouseX = 0;
	inline static float s_MouseY = 0;

	Minecraft::CharacterController::FOVchangeEvent ComputeInput(GLFWwindow* window, double deltaTime, Chunk* chunkArray[9]);
	void ComputeRaycast(GLFWwindow* window, double deltaTime, Chunk* chunkArray[9]);

	static bool RayIntersectsCube(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& box, const float size);
	inline bool BoxIntersectsBlock(Minecraft::Block_static* block, const glm::vec3& boxEdgeMin, const glm::vec3& boxEdgeMax);

	glm::vec4 ClipChunkCoordinate(const glm::vec3& coord) const;
	static void OnMouseCallback(GLFWwindow* window, double xpos, double ypos);
	void ProcessMouse();

public:
	explicit CharacterController(const glm::vec3& position = {0.f, 0.f, 0.f});

	Minecraft::CharacterController::FOVchangeEvent OnInput(GLFWwindow* window, double deltaTime, Chunk* chunkArray[9]);
	void OnUpdate(double deltaTime);

	void OnMouseEvent(GLFWwindow* window);
	void Spawn(const glm::vec3& position);

	// Accessors
	inline const glm::vec3& getPosition() const { return m_Camera.Position; }
	inline const glm::vec3& getFront() const { return m_Camera.Front; }
	inline const glm::vec3& getUp() const { return m_Camera.Up; }
	inline const Minecraft::Block_static* getSelectedBlock() { return m_SelectedBlock; }

	inline const bool isOnGround() const { return m_IsGrounded; };

	inline void toggleMode(Minecraft::CharacterController::STATE mode) {
		m_State = mode;

		switch (mode) {
		case Minecraft::CharacterController::STATE::SPRINTING:
			m_MovementSpeed = conf.MOVEMENT_SPEED_SPRINT;
			break;
		case Minecraft::CharacterController::STATE::WALKING:
			m_MovementSpeed = conf.MOVEMENT_SPEED_WALK;
			break;
		case Minecraft::CharacterController::STATE::CROUCHING:
			m_MovementSpeed = conf.MOVEMENT_SPEED_CROUCH;
			break;
		case Minecraft::CharacterController::STATE::FLYING:
			m_MovementSpeed = 0.1f;
			break;
		}	
	}
};

