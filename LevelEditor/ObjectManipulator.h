#pragma once
#include <vector>
#include "DisplayObject.h"
#include "DisplayChunk.h"
#include "SceneObject.h"
#include "pch.h"
#include "InputCommands.h"
#include "StepTimer.h"
#include "Camera.h"

// Manipulation mode enum
enum class ManipulationMode {
	TRANSLATE,
	ROTATE,
	SCALE
};

// Triangle struct
struct Triangle
{
	DirectX::SimpleMath::Vector3 vertex0;
	DirectX::SimpleMath::Vector3 vertex1;
	DirectX::SimpleMath::Vector3 vertex2;
};

class ObjectManipulator
{
public:
	ObjectManipulator();
	~ObjectManipulator();

	// Update
	void Update(DX::StepTimer const& timer, InputCommands* input, Camera* camera);

	// Set pointers to scene graph and current selection
	void SetSceneGraph(std::vector<SceneObject>* sceneGraph, int* sel) { m_sceneGraph = sceneGraph; m_currentSelection = sel; };
	
	// Functions relating to ground snapping
	void SnapToGround(DisplayChunk* terrain);
	static bool RayIntersectsTriangle(DirectX::SimpleMath::Vector3 rayOrigin, DirectX::SimpleMath::Vector3 rayVector, Triangle* inTriangle, DirectX::SimpleMath::Vector3& outIntersectionPoint);
	void CreateTriangles(DisplayChunk* terrain);
	
	// Getters
	bool GetActive() { return m_isManipulating; };
	ManipulationMode GetMode() { return m_manipulationMode; };
	float GetMoveSpeed() { return m_movementRate; };
	float GetRotSpeed() { return m_rotationRate; };
	float GetScaleSpeed() { return m_scaleRate; };
	DisplayObject* GetObject() { return m_object; };
	SceneObject GetInitialObject() { return m_initialObject; };
	std::vector<Triangle> GetTriangles() { return m_triangles; };
	float GetClickLength() { return m_clickTimer; };

	// Setters
	void SetMode(ManipulationMode mode) { m_manipulationMode = mode; };
	void SetMoveSpeed(float s) { m_movementRate = s; };
	void SetRotSpeed(float s) { m_rotationRate = s; };
	void SetScaleSpeed(float s) { m_scaleRate = s; };
	void SetObject(DisplayObject* obj) { m_object = obj; };
	void SetObjectPosition(float x, float y, float z);
	void SetObjectRotation(float x, float y, float z);
	void SetObjectScale(float x, float y, float z);
	
private:
	// Current mode
	ManipulationMode m_manipulationMode;

	// Selected object
	DisplayObject* m_object;

	// Current manipulation state
	bool m_isManipulating;

	// Object properties upon manipulation start
	SceneObject m_initialObject;

	// Terrain triangles
	std::vector<Triangle> m_triangles;

	// Scene graph and selection
	std::vector<SceneObject>* m_sceneGraph;
	int* m_currentSelection;

	// Click properties
	int m_clickX;
	int m_clickY;
	float m_clickTimer;

	// Transformation rates
	float m_movementRate;
	float m_rotationRate;
	float m_scaleRate;

};

