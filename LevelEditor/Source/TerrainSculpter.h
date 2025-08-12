#pragma once
#include "DisplayChunk.h"
#include "StepTimer.h"
#include "InputCommands.h"

// Enum for different modes
enum class SculptMode {
	RAISE,
	LOWER,
	FLATTEN
};

class TerrainSculpter
{
public:
	TerrainSculpter();
	~TerrainSculpter();

	// Function that sculpts terrain given a pointer to the chunk and a position.
	void Sculpt(DisplayChunk* terrain, DirectX::SimpleMath::Vector3 spherePos, DX::StepTimer const& timer);

	// Getters
	SculptMode GetMode() { return m_sculptMode; };
	float GetRadius() { return m_radius; };
	float GetMagnitude() { return m_magnitude; };

	// Setters
	void SetRadius(float r) { m_radius = r; };
	void SetMagnitude(float m) { m_magnitude = m; };
	void SetMode(SculptMode mode) { m_sculptMode = mode; };
	void SetInput(InputCommands* input) { m_inputCommands = input; };
	void SetToolbarHeight(int height) { m_toolbarHeight = height; }

	// Function to map floats from 1 range to another
	static float MapFloat(float f, float in1, float in2, float out1, float out2);

	// Status properties
	bool m_canSculpt;
	bool m_editHeightMap;

protected:
	// Inpout pointer for mouse position
	InputCommands* m_inputCommands;

	// Current mode
	SculptMode m_sculptMode;

	// Properties of the sculpt
	float m_radius;
	float m_magnitude;

	// Toolbar height
	int m_toolbarHeight;
};

