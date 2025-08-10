#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera()
{
	// Initial values
	m_camMoveSpeed = 10.0;
	m_camRotRate = 7.0;

	m_camPosition.x = 0.0f;
	m_camPosition.y = 3.7f;
	m_camPosition.z = -3.5f;

	m_targetPosition = m_camPosition;
	m_startPosition = m_camPosition;

	m_camOrientation.x = 0;
	m_camOrientation.y = 0;
	m_camOrientation.z = 0;

	m_camLookAt.x = 0.0f;
	m_camLookAt.y = 0.0f;
	m_camLookAt.z = 0.0f;

	m_camLookDirection.x = 0.0f;
	m_camLookDirection.y = 0.0f;
	m_camLookDirection.z = 0.0f;

	m_camRight.x = 0.0f;
	m_camRight.y = 0.0f;
	m_camRight.z = 0.0f;

	// Up vector is fixed regardless of orientation
	m_camUp.x = 0.0f;
	m_camUp.y = 1.0f;
	m_camUp.z = 0.0f;

	m_camOrientation.x = 0.0f;
	m_camOrientation.y = 0.0f;
	m_camOrientation.z = 0.0f;

	m_mouseMode = false;

	m_lerpSpeed = 4.0f;
	m_lerpTimer = 0.0f;
	m_isLerping = false;
}

Camera::~Camera()
{

}

void Camera::Update(DX::StepTimer const& timer, InputCommands* input)
{
	// Get window dimensions
	RECT rect;
	GetWindowRect(GetActiveWindow(), &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	// When right click happens, save the position of the click and set mouse mode to be active
	if (input->RMBDown && !m_mouseMode)
	{
		m_clickX = input->mouseX;
		m_clickY = input->mouseY;

		m_mouseMode = true;
	}
	else if(!input->RMBDown) // when right click isn't pressed, disable mouse mode and process keyboard rotation
	{
		m_mouseMode = false;
		// Process rotation input.
		if (input->rotRight)
		{
			m_camOrientation.y += m_camRotRate * timer.GetElapsedSeconds();
		}
		if (input->rotLeft)
		{
			m_camOrientation.y -= m_camRotRate * timer.GetElapsedSeconds();
		}
		if (input->rotUp)
		{
			m_camOrientation.z += m_camRotRate * timer.GetElapsedSeconds();

		}
		if (input->rotDown)
		{
			m_camOrientation.z -= m_camRotRate * timer.GetElapsedSeconds();

		}
	}

	// Distance of mouse click
	float distanceX;
	float distanceY;

	// When mouse mode is active, get the distance the mouse has moved in the last frame and apply to the camera's orientation
	if (m_mouseMode)
	{
		distanceX = input->mouseX - m_clickX;
		distanceY = input->mouseY - m_clickY;

		m_camOrientation.y += distanceX * timer.GetElapsedSeconds() * m_camRotRate;
		m_camOrientation.z -= distanceY * timer.GetElapsedSeconds() * m_camRotRate;

		SetCursorPos(m_clickX, m_clickY); // move cursor back to previous position
	}


	
	//create look direction from Euler angles in m_camOrientation
	/*
	m_camLookDirection.x = sin((m_camOrientation.y)*3.1415 / 180);
	m_camLookDirection.z = cos((m_camOrientation.y)*3.1415 / 180);
	m_camLookDirection.Normalize();
	*/

	// x = roll, y = yaw, z = pitch
	// lock pitch so can't look too far up or down
	if (m_camOrientation.z > 90) m_camOrientation.z = 90;
	else if (m_camOrientation.z < -90) m_camOrientation.z = -90;

	// Create look direction using parametric equation of a sphere.
	m_camLookDirection.x = cos((m_camOrientation.y) * 3.1415 / 180) * cos((m_camOrientation.z) * 3.1415 / 180);
	m_camLookDirection.y = sin((m_camOrientation.z) * 3.1415 / 180);
	m_camLookDirection.z = sin((m_camOrientation.y) * 3.1415 / 180) * cos((m_camOrientation.z) * 3.1415 / 180);
	m_camLookDirection.Normalize();

	//create right vector from look Direction
	m_camLookDirection.Cross(Vector3::UnitY, m_camRight);

	// If lerping, move camera towards desired position
	if (m_isLerping)
	{
		m_lerpTimer += timer.GetElapsedSeconds() * m_lerpSpeed;
		m_camPosition = DirectX::XMVectorLerp(m_startPosition, m_targetPosition, m_lerpTimer);
		if (m_lerpTimer > 1.0f) // lerp is between 0 and 1, when above 1 lerp has finished.
		{
			m_lerpTimer = 0.0f;
			m_camPosition = m_targetPosition;
			m_isLerping = false;
		}
	}
	else
	{
		// when not lerping, process keyboard movement
		if (input->forward)
		{
			m_camPosition += m_camLookDirection * m_camMoveSpeed * timer.GetElapsedSeconds();
		}
		if (input->back)
		{
			m_camPosition -= m_camLookDirection * m_camMoveSpeed * timer.GetElapsedSeconds();
		}
		if (input->right)
		{
			m_camPosition += m_camRight * m_camMoveSpeed * timer.GetElapsedSeconds();
		}
		if (input->left)
		{
			m_camPosition -= m_camRight * m_camMoveSpeed * timer.GetElapsedSeconds();
		}
		if (input->down)
		{
			m_camPosition -= m_camUp * m_camMoveSpeed * timer.GetElapsedSeconds();
		}
		if (input->up)
		{
			m_camPosition += m_camUp * m_camMoveSpeed * timer.GetElapsedSeconds();
		}
	}
	
	//update lookat point
	m_camLookAt = m_camPosition + m_camLookDirection;

}

void Camera::SetPosition(DirectX::SimpleMath::Vector3 pos)
{
	// Setup lerp with new position
	m_targetPosition = pos;
	m_startPosition = m_camPosition;
	m_isLerping = true;
	m_lerpTimer = 0.0f;
}

