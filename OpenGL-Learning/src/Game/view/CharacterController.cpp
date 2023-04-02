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

inline bool CharacterController::BoxIntersectsBlock(Minecraft::Block_static* block, const glm::vec3& boxEdgeMin, const glm::vec3& boxEdgeMax)
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
	float height = std::ceil(m_CharBodyHeight + m_CharHeadHeight) + 1;
	m_CheckOrder.reserve((size_t)(2 * 9 + (height - 2) * 8));

	for (float i = -1; i < height; i++) {
		if((int)i == -1 || (int)i == (int)height - 1)
			m_CheckOrder.push_back({ 0.f, i, 0.f });

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

bool CharacterController::InteractWithBlock(GLFWwindow* window, Chunk* chunkArray[9])
{
	// Place or Destroy Block
	if (m_SelectedBlock) {
		static bool keyPressedRight = false;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !keyPressedRight)
		{
			keyPressedRight = true;
			glm::vec3 pos = m_SelectedBlockPosition;

			switch (m_SelectedBlockSide) {
			case Minecraft::CharacterController::SIDE::NONE:
				break;
			case Minecraft::CharacterController::SIDE::FRONT:
				pos.z++;
				break;
			case Minecraft::CharacterController::SIDE::LEFT:
				pos.x--;
				break;
			case Minecraft::CharacterController::SIDE::RIGHT:
				pos.x++;
				break;
			case Minecraft::CharacterController::SIDE::BACK:
				pos.z--;
				break;
			case Minecraft::CharacterController::SIDE::BOTTOM:
				pos.y--;
				break;
			case Minecraft::CharacterController::SIDE::TOP:
				pos.y++;
				break;
			}

			chunkArray[(const unsigned int)m_SelectedBlockPosition.w]->SetBlockUpdated(pos, 1);
			return false;
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && keyPressedRight) {
			keyPressedRight = false;
		}

		static bool keyPressedLeft = false;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !keyPressedLeft)
		{
			keyPressedLeft = true;
			chunkArray[(const unsigned int)m_SelectedBlockPosition.w]->RemoveBlock({ m_SelectedBlockPosition.x, m_SelectedBlockPosition.y, m_SelectedBlockPosition.z });
			return true;
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && keyPressedLeft) {
			keyPressedLeft = false;
		}
	}

	return false;
}

Minecraft::CharacterController::InputChangeEvent CharacterController::OnInput(GLFWwindow* window, double deltaTime, Chunk* chunkArray[9])
{
	if (!m_IsSpawned) {
		m_Camera.Position = { 0.f, -10.f, 0.f };
		return {};
	}

	Minecraft::CharacterController::InputChangeEvent event = ComputeInput(window, deltaTime, chunkArray);

	ComputeRaycast(window, deltaTime, chunkArray);

	event.threadJobs += InteractWithBlock(window, chunkArray) ? 1 : 0;
	event.chunkToBeQueued = event.threadJobs > 0 ? chunkArray[(const unsigned int)m_SelectedBlockPosition.w] : nullptr;
	
	return event;
}

glm::vec4 CharacterController::ClipChunkCoordinate(const glm::vec3& coord) const
{
	int chunkIndex = 4;
	glm::vec3 coordOut = coord;

	if (coord.x < 0 && coord.z >= 0 && coord.z < conf.CHUNK_SIZE) {
		chunkIndex = 1;
		coordOut.x = conf.CHUNK_SIZE - abs(coord.x);
	}
	else if (coord.x < 0 && coord.z < 0) {
		chunkIndex = 0;
		coordOut.x = conf.CHUNK_SIZE - abs(coord.x);
		coordOut.z = conf.CHUNK_SIZE - abs(coord.z);
	}
	else if (coord.z < 0 && coord.x >= 0 && coord.x < conf.CHUNK_SIZE) {
		chunkIndex = 3;
		coordOut.z = conf.CHUNK_SIZE - abs(coord.z);
	}
	else if (coord.x >= conf.CHUNK_SIZE && coord.z < 0) {
		chunkIndex = 6;
		coordOut.x = coord.x - conf.CHUNK_SIZE;
		coordOut.z = conf.CHUNK_SIZE - abs(coord.z);
	}
	else if (coord.x >= conf.CHUNK_SIZE && coord.z >= 0 && coord.z < conf.CHUNK_SIZE) {
		chunkIndex = 7;
		coordOut.x = coord.x - conf.CHUNK_SIZE;
	}
	else if (coord.x >= conf.CHUNK_SIZE && coord.z >= conf.CHUNK_SIZE) {
		chunkIndex = 8;
		coordOut.x = coord.x - conf.CHUNK_SIZE;
		coordOut.z = coord.z - conf.CHUNK_SIZE;
	}
	else if (coord.x >= 0 && coord.x < conf.CHUNK_SIZE && coord.z >= conf.CHUNK_SIZE) {
		chunkIndex = 5;
		coordOut.z = coord.z - conf.CHUNK_SIZE;
	}
	else if (coord.x < 0.f && coord.z >= conf.CHUNK_SIZE) {
		chunkIndex = 2;
		coordOut.x = conf.CHUNK_SIZE - abs(coord.x);
		coordOut.z = coord.z - conf.CHUNK_SIZE;
	}
	return glm::vec4(coordOut, chunkIndex);
}

void CharacterController::OnUpdate(double deltaTime)
{
	if(m_State != Minecraft::CharacterController::STATE::FLYING && !m_IsGrounded) m_FrameVelocity += glm::vec3(0.f, - conf.MOVEMENT_GRAVITATION, 0.f) * (float)deltaTime;
	if (m_FrameVelocity.y < -conf.MOVEMENT_MAX_FALL_SPEED) m_FrameVelocity.y = -conf.MOVEMENT_MAX_FALL_SPEED;
}

Minecraft::CharacterController::InputChangeEvent CharacterController::ComputeInput(GLFWwindow* window, double deltaTime, Chunk* chunkArray[9])
{
	glfwSetCursorPosCallback(window, OnMouseCallback);
	ProcessMouse();

	const float cameraSpeed = 13.f * m_MovementSpeed * (float)deltaTime * (m_IsGrounded || m_State == Minecraft::CharacterController::STATE::FLYING ? 1.f : conf.MOVEMENT_CONTROLL_AIR); // adjust accordingly
	const glm::vec2 forwardXY = glm::normalize(glm::vec2(m_Camera.Front.x, m_Camera.Front.z));

	Minecraft::CharacterController::InputChangeEvent changeEvent;

	// Input
	static bool keyPressedLShift = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !keyPressedLShift)
	{
		keyPressedLShift = true;
		toggleMode(Minecraft::CharacterController::STATE::CROUCHING);
		changeEvent.doChangeFOV = true;
		changeEvent.FOV = conf.FOV_CROUCH;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && keyPressedLShift) {
		keyPressedLShift = false;
		toggleMode(Minecraft::CharacterController::STATE::WALKING);
		changeEvent.doChangeFOV = true;
		changeEvent.FOV = conf.FOV_WALK;
	}

	static bool keyPressedLCTRL = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !keyPressedLCTRL && !keyPressedLShift)
	{
		keyPressedLCTRL = true;
		toggleMode(Minecraft::CharacterController::STATE::SPRINTING);
		changeEvent.doChangeFOV = true;
		changeEvent.FOV = conf.FOV_SPRINT;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE && keyPressedLCTRL) {
		keyPressedLCTRL = false;
		toggleMode(Minecraft::CharacterController::STATE::WALKING);
		changeEvent.doChangeFOV = true;
		changeEvent.FOV = conf.FOV_WALK;
	}

	bool moveStraight = false;
	bool moveSide = false;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		moveSide = true;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) { m_FrameVelocity += cameraSpeed * m_Camera.Front; }
		else m_FrameVelocity += cameraSpeed * glm::vec3(forwardXY.x, 0.f, forwardXY.y);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		moveSide = true;
		if (m_State == Minecraft::CharacterController::STATE::FLYING) m_FrameVelocity -= cameraSpeed * m_Camera.Front;
		else m_FrameVelocity -= cameraSpeed * glm::vec3(forwardXY.x, 0.f, forwardXY.y);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
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

	m_BlockPositionInChunk = { (int)std::floor(m_BlockPosition.x) % conf.CHUNK_SIZE, (int)std::floor(m_BlockPosition.y), ((int)std::floor(m_BlockPosition.z) % conf.CHUNK_SIZE) };

	glm::vec3 hitBoxVertices[2] = {
		{m_Camera.Position.x - m_CharWidth / 2.f, m_Camera.Position.y - m_CharBodyHeight, m_Camera.Position.z - m_CharWidth / 2.f},
		{m_Camera.Position.x + m_CharWidth / 2.f, m_Camera.Position.y + m_CharHeadHeight, m_Camera.Position.z + m_CharWidth / 2.f}
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
		glm::vec4 clipCoord = ClipChunkCoordinate(m_BlockPositionInChunk + coord);
		Minecraft::Block_static* block = chunkArray[(int)clipCoord.a] ? chunkArray[(int)clipCoord.a]->getBlock({clipCoord.x, clipCoord.y, clipCoord.z}) : nullptr;

		if (!block) continue;
		LOGC(std::to_string(block->position.y));

		if (block->subtype == Minecraft::BLOCKTYPE::STATIC_DEFAULT && BoxIntersectsBlock(block, hitBoxVertices[0] + m_FrameVelocity, hitBoxVertices[1] + m_FrameVelocity)) {
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && m_IsGrounded)
	{
		m_FrameVelocity.y = conf.MOVEMENT_JUMP_STRENGHT;
	}
	
	m_Camera.Position += m_FrameVelocity;

	return changeEvent;
}

void CharacterController::ComputeRaycast(GLFWwindow* window, double deltaTime, Chunk* chunkArray[9])
{
	float distToNearest = std::numeric_limits<float>::max();

	int faceX = m_Camera.Front.x >= 0 ? 1 : -1;
	int faceY = m_Camera.Front.y >= 0 ? 1 : -1;
	int faceZ = m_Camera.Front.z >= 0 ? 1 : -1;

	glm::vec3 cameraBlockPosition = m_BlockPositionInChunk + glm::vec3(0.f, std::floor(m_CharBodyHeight / conf.BLOCK_SIZE), 0.f);

	for (int y = 0; abs(y) < conf.BLOCK_INTERACTION_RANGE + 1; y += faceY) {
		for (int z = 0; abs(z) < conf.BLOCK_INTERACTION_RANGE + 1; z += faceZ) {
			for (int x = 0; abs(x) < conf.BLOCK_INTERACTION_RANGE + 1; x += faceX) {
				if (y == 0 && z == 0 && x == 0 || cameraBlockPosition.y + y < 0 || cameraBlockPosition.y + y >= conf.CHUNK_HEIGHT) continue;
				glm::vec4 clipCoord = ClipChunkCoordinate(cameraBlockPosition + glm::vec3(x, y, z));

				Minecraft::Block_static* block = chunkArray[(int)clipCoord.w] ? chunkArray[(int)clipCoord.w]->getBlock({ clipCoord.x, clipCoord.y, clipCoord.z }) : nullptr;
				if (!block) continue;
				Minecraft::CharacterController::SIDE side = RayIntersectsCube(m_Camera.Position, m_Camera.Front, block->position, conf.BLOCK_SIZE);
				if (side != Minecraft::CharacterController::SIDE::NONE) {
					float distance = glm::distance(m_Camera.Position, block->position + glm::vec3(conf.BLOCK_SIZE / 2.f));
					if (distance < distToNearest) {
						m_SelectedBlock = block;
						distToNearest = distance;
						m_SelectedBlockPosition = clipCoord;
						m_SelectedBlockSide = side;
					}
				}
			}
		}
	}

	if (distToNearest > conf.BLOCK_INTERACTION_RANGE + 1) m_SelectedBlock = nullptr;
}

void CharacterController::OnMouseEvent(GLFWwindow* window)
{

}

Minecraft::CharacterController::SIDE CharacterController::RayIntersectsCube(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& box, const float size)
{
	glm::vec3 min = box;
	glm::vec3 max = box + size;

	float t1 = (min.x - origin.x) / direction.x;
	float t2 = (max.x - origin.x) / direction.x;
	float t3 = (min.y - origin.y) / direction.y;
	float t4 = (max.y - origin.y) / direction.y;
	float t5 = (min.z - origin.z) / direction.z;
	float t6 = (max.z - origin.z) / direction.z;

	float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
	float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

	if (tmax < 0.f || tmin > tmax) {
		return Minecraft::CharacterController::SIDE::NONE;
	}

	if (tmin < 0.f) {
		return Minecraft::CharacterController::SIDE::INSIDE;
	}

	glm::vec3 point = origin + tmin * direction;
	glm::vec3 distanceToOrig = point - glm::vec3(box + size / 2.f);

	Minecraft::CharacterController::SIDE intersectedSide = Minecraft::CharacterController::SIDE::TOP;

	float epsilon = 0.00001f;
	
	if (distanceToOrig.x > -(size / 2.0 + epsilon) && distanceToOrig.x < -(size / 2.0 - epsilon)) intersectedSide = Minecraft::CharacterController::SIDE::LEFT;
	if (distanceToOrig.x < (size / 2.0 + epsilon) && distanceToOrig.x > (size / 2.0 - epsilon)) intersectedSide = Minecraft::CharacterController::SIDE::RIGHT;

	if (distanceToOrig.y > -(size / 2.0 + epsilon) && distanceToOrig.y < -(size / 2.0 - epsilon)) intersectedSide = Minecraft::CharacterController::SIDE::BOTTOM;
	if (distanceToOrig.y < (size / 2.0 + epsilon) && distanceToOrig.y >(size / 2.0 - epsilon)) intersectedSide = Minecraft::CharacterController::SIDE::TOP;

	if (distanceToOrig.z > -(size / 2.0 + epsilon) && distanceToOrig.z < -(size / 2.0 - epsilon)) intersectedSide = Minecraft::CharacterController::SIDE::BACK;
	if (distanceToOrig.z < (size / 2.0 + epsilon) && distanceToOrig.z >(size / 2.0 - epsilon)) intersectedSide = Minecraft::CharacterController::SIDE::FRONT;

	return intersectedSide;
}

void CharacterController::Spawn(const glm::vec3& position) {
	m_Camera.Position = { 0.f, position.y + m_CharBodyHeight, 0.f };
	m_IsSpawned = true;
}
