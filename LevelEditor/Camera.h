#include "pch.h"
#include "InputCommands.h"
#include "StepTimer.h"
#include <vector>

#pragma once
class Camera
{
public:
	Camera();
	~Camera();

	// Update
	void Update(DX::StepTimer const& timer, InputCommands* input);

	// Getters
	DirectX::SimpleMath::Vector3 GetPosition() { return m_camPosition; };
	DirectX::SimpleMath::Vector3 GetLookAt() { return m_camLookAt; };
	DirectX::SimpleMath::Vector3 GetRight() { return m_camRight; };
	DirectX::SimpleMath::Vector3 GetForward() { return m_camLookDirection; };
	float GetLerpSpeed() { return m_lerpSpeed; };
	float GetMoveSpeed() { return m_camMoveSpeed; };
	float GetRotSpeed() { return m_camRotRate; };

	// Setters
	void SetLerpSpeed(float s) { m_lerpSpeed = s; };
	void SetMoveSpeed(float s) { m_camMoveSpeed = s; };
	void SetRotSpeed(float s) { m_camRotRate = s; };

	// Set position (with lerp)
	void SetPosition(DirectX::SimpleMath::Vector3 pos);

private:
	// Camera properties
	DirectX::SimpleMath::Vector3		m_camPosition;
	DirectX::SimpleMath::Vector3		m_startPosition;
	DirectX::SimpleMath::Vector3		m_targetPosition;
	DirectX::SimpleMath::Vector3		m_camOrientation;
	DirectX::SimpleMath::Vector3		m_camLookAt;
	DirectX::SimpleMath::Vector3		m_camLookDirection;
	DirectX::SimpleMath::Vector3		m_camRight;
	DirectX::SimpleMath::Vector3		m_camUp;

	// Transform rates
	float m_camRotRate;
	float m_camMoveSpeed;

	// Mouse mode active
	bool m_mouseMode;

	// Track clicks
	int m_clickX;
	int m_clickY;

	// Lerp properties
	float m_lerpSpeed;
	float m_lerpTimer;
	bool m_isLerping;
};

