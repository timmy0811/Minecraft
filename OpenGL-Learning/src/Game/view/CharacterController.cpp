#include "CharacterController.h"

void CharacterController::ProcessMouse()
{
	if (m_FirstMouseInit) {
		m_LastX = s_MouseX;
		m_LastY = s_MouseY;
		m_FirstMouseInit = false;
	}

	float xoffset = s_MouseX - m_LastX;
	float yoffset = m_LastY - s_MouseY; // reversed since y-coordinates range from bottom to top
	m_LastX = s_MouseX;
	m_LastY = s_MouseY;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	m_Camera.yaw += xoffset;
	m_Camera.pitch += yoffset;

	if (m_Camera.pitch > 89.99f)
		m_Camera.pitch = 89.99f;
	if (m_Camera.pitch < -89.99f)
		m_Camera.pitch = -89.99f;

	glm::vec3 direction;

	direction.x = cos(glm::radians(m_Camera.yaw)) * cos(glm::radians(m_Camera.pitch));
	direction.y = sin(glm::radians(m_Camera.pitch));
	direction.z = sin(glm::radians(m_Camera.yaw)) * cos(glm::radians(m_Camera.pitch));
	m_Camera.Front = glm::normalize(direction);
}

inline bool CharacterController::CheckForBoxIntersection(Minecraft::Block_static* block, const glm::vec3& boxEdgeMin, const glm::vec3& boxEdgeMax)
{
	if (block->position.x + conf.BLOCK_SIZE < boxEdgeMin.x || boxEdgeMax.x < block->position.x) return false;
	if (block->position.y + conf.BLOCK_SIZE < boxEdgeMin.y || boxEdgeMax.y < block->position.y) return false;
	if (block->position.z + conf.BLOCK_SIZE < boxEdgeMin.z || boxEdgeMax.z < block->position.z) return false;
	return true;
}

void CharacterController::OnMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	CharacterController::s_MouseX = float(xpos);
	CharacterController::s_MouseY = float(ypos);
}

CharacterController::CharacterController(const glm::vec3& position)
	:m_State(Minecraft::CharacterController::STATE::WALKING)
{
	m_Camera.Position = position;

	// Setup Check Order
	float height = std::ceil(m_CharBodyHeight + m_CharHeadHeight) + 2;
	m_CheckOrder.reserve((size_t)(2 * 9 + (height - 2) * 8));

	for (float i = -1; i < height; i++) {
		if(i == -1 || i == height - 1.f) m_CheckOrder.push_back({ 0.f, i, 0.f });

		// Inner
		m_CheckOrder.push_back({ 0.f, i, -1.f });
		m_CheckOrder.push_back({ -1.f, i, 0.f });
		m_CheckOrder.push_back({ 0.f, i, 1.f });
		m_CheckOrder.push_back({ 1.f, i, 0.f });

		// Outer
		m_CheckOrder.push_back({ -1.f, i, -1.f });
		m_CheckOrder.push_back({ -1.f, i, 1.f });
		m_CheckOrder.push_back({ 1.f, i, 1.f });
		m_CheckOrder.push_back({ 1.f, i, -1.f });
	}

	toggleMode(Minecraft::CharacterController::STATE::WALKING);
}

Minecraft::CharacterController::FOVchangeEvent CharacterController::OnInput(GLFWwindow* window, double deltaTime, Chunk* chunkArray[9])
{
	if (!m_IsSpawned) {
		m_Camera.Position = { 0.f, -10.f, 0.f };
		return {};
	}

	glfwSetCursorPosCallback(window, OnMouseCallback);
	ProcessMouse();

	const float cameraSpeed = 13.f * m_MovementSpeed * (float)deltaTime * (m_IsGrounded || m_State == Minecraft::CharacterController::STATE::FLYING ? 1.f : conf.MOVEMENT_CONTROLL_AIR); // adjust accordingly
	const glm::vec2 forwardXY = glm::normalize(glm::vec2(m_Camera.Front.x, m_Camera.Front.z));

	Minecraft::CharacterController::FOVchangeEvent changeEvent;

	// Input
	static bool keyPressedLShift = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !keyPressedLShift)
	{
		keyPressedLShift = true;
		toggleMode(Minecraft::CharacterController::STATE::CROUCHING);
		changeEvent.doChange = true;
		changeEvent.FOV = conf.FOV_CROUCH;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && keyPressedLShift) {
		keyPressedLShift = false;
		toggleMode(Minecraft::CharacterController::STATE::WALKING);
		changeEvent.doChange = true;
		changeEvent.FOV = conf.FOV_WALK;
	}

	static bool keyPressedLCTRL = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !keyPressedLCTRL && !keyPressedLShift)
	{
		keyPressedLCTRL = true;
		toggleMode(Minecraft::CharacterController::STATE::SPRINTING);
		changeEvent.doChange = true;
		changeEvent.FOV = conf.FOV_SPRINT;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE && keyPressedLCTRL) {
		keyPressedLCTRL = false;
		toggleMode(Minecraft::CharacterController::STATE::WALKING);
		changeEvent.doChange = true;
		changeEvent.FOV = conf.FOV_WALK;
	}

	bool moveStraight = false;
	bool moveSide = false;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		moveSide = true;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) { m_FrameVelocity += cameraSpeed * m_Camera.Front; }
		else m_FrameVelocity += cameraSpeed * glm::vec3(forwardXY.x, 0.f, forwardXY.y);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		moveSide = true;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) m_FrameVelocity -= cameraSpeed * m_Camera.Front;
		else m_FrameVelocity -= cameraSpeed * glm::vec3(forwardXY.x, 0.f, forwardXY.y);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		moveStraight = true;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) m_FrameVelocity -= glm::normalize(glm::cross(m_Camera.Front, m_Camera.Up)) * cameraSpeed;
		else m_FrameVelocity -= glm::normalize(glm::cross(glm::vec3(forwardXY.x, 0.f, forwardXY.y), glm::vec3(0.f, 1.f, 0.f))) * cameraSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		moveStraight = true;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) m_FrameVelocity += glm::normalize(glm::cross(m_Camera.Front, m_Camera.Up)) * cameraSpeed;
		else m_FrameVelocity += glm::normalize(glm::cross(glm::vec3(forwardXY.x, 0.f, forwardXY.y), glm::vec3(0.f, 1.f, 0.f))) * cameraSpeed;
	}

	// Movement
	if (!moveStraight && !moveSide) {
		float drag;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) {
			drag = conf.MOVEMENT_PLAYER_DRAG * 0.35f;

			// Drag on Y
			if (m_FrameVelocity.y > 0.005) m_FrameVelocity.y -= abs(m_FrameVelocity.y) * drag;
			else if (m_FrameVelocity.y < -0.005) m_FrameVelocity.y += abs(m_FrameVelocity.y) * drag;
			else m_FrameVelocity.y = 0.f;
		}
		else {
			drag = m_IsGrounded ? conf.MOVEMENT_PLAYER_DRAG : conf.MOVEMENT_PLAYER_DRAG_AIR;
		}

		// Drag on X
		if (m_FrameVelocity.x > 0.005) m_FrameVelocity.x -= abs(m_FrameVelocity.x) * drag;
		else if (m_FrameVelocity.x < -0.005) m_FrameVelocity.x += abs(m_FrameVelocity.x) * drag;
		else m_FrameVelocity.x = 0.f;

		// Drag on Z
		if (m_FrameVelocity.z > 0.005) m_FrameVelocity.z -= abs(m_FrameVelocity.z) * drag;
		else if (m_FrameVelocity.z < -0.005) m_FrameVelocity.z += abs(m_FrameVelocity.z) * drag;
		else m_FrameVelocity.z = 0.f;
	}

	m_BlockPosition = { std::floor(m_Camera.Position.x - r_WorldRootPosition.x),
						std::floor(m_Camera.Position.y - m_CharBodyHeight),
						std::floor(m_Camera.Position.z - r_WorldRootPosition.z) };

	glm::vec3 blockPositionInChunk = { (int)std::floor(m_BlockPosition.x) % conf.CHUNK_SIZE, (int)std::floor(m_BlockPosition.y), ((int)std::floor(m_BlockPosition.z) % conf.CHUNK_SIZE) + 1 };

	glm::vec3 hitBoxVertices[2] = {
		{m_Camera.Position.x - m_CharWidth / 2.f, m_Camera.Position.y - m_CharBodyHeight, m_Camera.Position.z - m_CharWidth / 2.f + 1},
		{m_Camera.Position.x + m_CharWidth / 2.f, m_Camera.Position.y + m_CharHeadHeight, m_Camera.Position.z + m_CharWidth / 2.f + 1}
	};

	// Limit Movement Speed
	const float stepDistance = glm::length(glm::vec2(m_FrameVelocity.x, m_FrameVelocity.z));
	const float cuttageFactor = m_MovementSpeed / stepDistance;
	if (stepDistance > m_MovementSpeed) {
		m_FrameVelocity.x *= cuttageFactor;
		m_FrameVelocity.z *= cuttageFactor;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) m_FrameVelocity.y *= cuttageFactor;
	}

	// Check for Collision
	m_IsGrounded = false;
	bool skipFloor = false;

	for (const glm::vec3& coord : m_CheckOrder) {
		if (skipFloor && coord.y == -1) continue;
		
		static glm::vec3 coordInChunk;
		coordInChunk = blockPositionInChunk + coord;
		int chunkIndex = 4;

		if (coordInChunk.x < 0 && coordInChunk.z >= 0 && coordInChunk.z < conf.CHUNK_SIZE) {
			chunkIndex = 1;
			coordInChunk.x = conf.CHUNK_SIZE - 1.f;
		}
		else if (coordInChunk.x < 0 && coordInChunk.z < 0) {
			chunkIndex = 0;
			coordInChunk.x = conf.CHUNK_SIZE - 1.f;
			coordInChunk.z = conf.CHUNK_SIZE - 1.f;
		}
		else if (coordInChunk.z < 0 && coordInChunk.x >= 0 && coordInChunk.x < conf.CHUNK_SIZE) {
			chunkIndex = 3;
			coordInChunk.z = conf.CHUNK_SIZE - 1.f;
		}
		else if (coordInChunk.x >= conf.CHUNK_SIZE && coordInChunk.z < 0) {
			chunkIndex = 6;
			coordInChunk.x = 0.f;
			coordInChunk.z = conf.CHUNK_SIZE - 1.f;
		}
		else if (coordInChunk.x >= conf.CHUNK_SIZE && coordInChunk.z >= 0 && coordInChunk.z < conf.CHUNK_SIZE) {
			chunkIndex = 7;
			coordInChunk.x = 0.f;
		}
		else if (coordInChunk.x >= conf.CHUNK_SIZE && coordInChunk.z >= conf.CHUNK_SIZE) {
			chunkIndex = 8;
			coordInChunk.x = 0.f;
			coordInChunk.z = 0.f;
		}
		else if (coordInChunk.x >= 0 && coordInChunk.x < conf.CHUNK_SIZE && coordInChunk.z >= conf.CHUNK_SIZE) {
			chunkIndex = 5;
			coordInChunk.z = 0.f;
		}
		else if (coordInChunk.x < 0.f && coordInChunk.z >= conf.CHUNK_SIZE) {
			chunkIndex = 2;
			coordInChunk.x = conf.CHUNK_SIZE - 1.f;
			coordInChunk.z = 0.f;
		}

		Minecraft::Block_static* block = chunkArray[chunkIndex] ? chunkArray[chunkIndex]->getBlock(coordInChunk) : nullptr;

		if (!block) continue;
		if (block->subtype == Minecraft::BLOCKTYPE::STATIC_DEFAULT && CheckForBoxIntersection(block, hitBoxVertices[0] + m_FrameVelocity, hitBoxVertices[1] + m_FrameVelocity)) {
			// Floor
			if (coord.y == -1) {
				m_FrameVelocity.y = -((m_Camera.Position.y - m_CharBodyHeight) - floor(m_Camera.Position.y - m_CharBodyHeight));
				m_IsGrounded = true;
				skipFloor = true;
			}
			// Walls
			else {
				// Along X Axis
				if (hitBoxVertices[0].x > block->position.x + conf.BLOCK_SIZE && m_FrameVelocity.x < 0.f ||
					hitBoxVertices[1].x < block->position.x && m_FrameVelocity.x > 0.f) {
					m_FrameVelocity.x = 0.f;
				}
				// Along Z Axis
				if (hitBoxVertices[0].z > block->position.z + conf.BLOCK_SIZE && m_FrameVelocity.z < 0.f ||
					hitBoxVertices[1].z < block->position.z && m_FrameVelocity.z > 0.f) {
					m_FrameVelocity.z = 0.f;
				}

				// Along Y Axis
				if (coord.y == std::ceil(m_CharBodyHeight + m_CharHeadHeight) && hitBoxVertices[1].y < block->position.y && m_FrameVelocity.y > 0.f) {
					m_FrameVelocity.y = 0.f;
				}
			}
		}
	}

	// Jumping
	static bool keyPressedSpace = false;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !keyPressedSpace)
	{
		keyPressedSpace = true;
		if (m_IsGrounded) {
			m_FrameVelocity.y = conf.MOVEMENT_JUMP_STRENGHT;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && keyPressedSpace) keyPressedSpace = false;

	m_Camera.Position += m_FrameVelocity;
	
	return changeEvent;
}

void CharacterController::OnUpdate(double deltaTime)
{
	if(m_State != Minecraft::CharacterController::STATE::FLYING && !m_IsGrounded) m_FrameVelocity += glm::vec3(0.f, - conf.MOVEMENT_GRAVITATION, 0.f) * (float)deltaTime;
	if (m_FrameVelocity.y < -conf.MOVEMENT_MAX_FALL_SPEED) m_FrameVelocity.y = -conf.MOVEMENT_MAX_FALL_SPEED;
}

void CharacterController::OnMouseEvent(GLFWwindow* window)
{
}

void CharacterController::Spawn(const glm::vec3& position) {
	m_Camera.Position = { 0.f, position.y + m_CharBodyHeight, 0.f };
	m_IsSpawned = true;
}
