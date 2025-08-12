#pragma once
#include "ObjectManipulator.h"



ObjectManipulator::ObjectManipulator()
{
	// Initial values
	m_object = NULL;
	m_manipulationMode = ManipulationMode::TRANSLATE;
	m_movementRate = 1.0f;
	m_rotationRate = 5.0f;
	m_scaleRate = 0.2f;
	m_clickTimer = 0.0f;
}

ObjectManipulator::~ObjectManipulator()
{
}

void ObjectManipulator::Update(DX::StepTimer const& timer, InputCommands* input, Camera* camera)
{
	if (input->LMBDown) // when LMB is down...
	{
		if (m_object) // if there is a valid object...
		{
			if (!m_isManipulating) // if not already manipulating...
			{
				if (GetParent(GetActiveWindow()) == 0) // if clicking the main window, start manipulating and save properties
				{
					m_isManipulating = true;
					m_clickX = input->mouseX;
					m_clickY = input->mouseY;
					m_clickTimer = 0.0f;
					m_initialObject = m_sceneGraph->at(*m_currentSelection);
				}
			}
			else // if already manipulating, just increase the timer
			{
				m_clickTimer += timer.GetElapsedSeconds();
			}
		}
		else // if no valid object, stop manipulating
		{
			m_isManipulating = false;
		}
	}
	else // stop manipulating when LMB is released.
	{
		m_isManipulating = false;
	}

	// When manipulating...
	if (m_isManipulating && m_object)
	{
		// Track how far mouse has moved since last frame
		float distanceX;
		float distanceY;
		distanceX = input->mouseX - m_clickX;
		distanceY = input->mouseY - m_clickY;

		// Get forward and right vectors of the camera
		DirectX::SimpleMath::Vector3 forward = camera->GetForward();
		DirectX::SimpleMath::Vector3 right = camera->GetRight();
		forward.y = 0; // remove yaw of forward, up-down movement is handled separately
		

		// Different actions are performed based on the manipulation mode.
		switch (m_manipulationMode)
		{
		default:
		case ManipulationMode::TRANSLATE: 
			// For translation, move up and down when shift is held. Otherwise move forwards/backwards and left/right.
			if (input->shift)
			{
				if (!m_object->m_snap_to_ground)
				{
					m_object->m_position.y -= distanceY * timer.GetElapsedSeconds() * m_movementRate;
				}
			}
			else
			{
				m_object->m_position += right * distanceX * timer.GetElapsedSeconds() * m_movementRate;
				m_object->m_position -= forward * distanceY * timer.GetElapsedSeconds() * m_movementRate;
			}

			// Also update scene graph so positions can be saved.
			m_sceneGraph->at(*m_currentSelection).posX = m_object->m_position.x;
			m_sceneGraph->at(*m_currentSelection).posY = m_object->m_position.y;
			m_sceneGraph->at(*m_currentSelection).posZ = m_object->m_position.z;

			break;
		case ManipulationMode::ROTATE:
			// For rotation, control roll and pitch when shift is held. Yaw is changed when shift is not held.
			if (input->shift)
			{
				m_object->m_orientation.x += distanceX * timer.GetElapsedSeconds() * m_rotationRate;
				m_object->m_orientation.z -= distanceY * timer.GetElapsedSeconds() * m_rotationRate;
			}
			else 
			{
				m_object->m_orientation.y += distanceX * timer.GetElapsedSeconds() * m_rotationRate;
			}

			// Also update scene graph so rotation can be saved.
			m_sceneGraph->at(*m_currentSelection).rotX = m_object->m_orientation.x;
			m_sceneGraph->at(*m_currentSelection).rotY = m_object->m_orientation.y;
			m_sceneGraph->at(*m_currentSelection).rotZ = m_object->m_orientation.z;
			break;
		case ManipulationMode::SCALE:
			// For scaling, it scales equally in all directions.
			m_object->m_scale -= DirectX::SimpleMath::Vector3(1) * distanceY * timer.GetElapsedSeconds() * m_scaleRate;

			// Update scene graph.
			m_sceneGraph->at(*m_currentSelection).scaX = m_object->m_scale.x;
			m_sceneGraph->at(*m_currentSelection).scaY = m_object->m_scale.y;
			m_sceneGraph->at(*m_currentSelection).scaZ = m_object->m_scale.z;
			break;
		}

		// Reset cursor position
		SetCursorPos(m_clickX, m_clickY);
	}
}

void ObjectManipulator::SetObjectPosition(float x, float y, float z)
{
	// Make position vector then set object position
	DirectX::SimpleMath::Vector3 pos = DirectX::SimpleMath::Vector3(x, y, z);

	if (m_object)
	{
		m_object->m_position = pos;
	}
}

void ObjectManipulator::SetObjectRotation(float x, float y, float z)
{
	// Make rotation vector then set object rotation
	DirectX::SimpleMath::Vector3 rot = DirectX::SimpleMath::Vector3(x, y, z);

	if (m_object)
	{
		m_object->m_orientation = rot;
	}
}

void ObjectManipulator::SetObjectScale(float x, float y, float z)
{
	// Make scale vector then set object scale
	DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3(x, y, z);

	if (m_object)
	{
		m_object->m_scale = scale;
	}
}

void ObjectManipulator::CreateTriangles(DisplayChunk* terrain)
{
	// Creates triangles from terrain.
	// First clear vector.
	m_triangles.clear();
	
	// For every vertex point in the terrain, identify the triangles and add them to the vector.
	for (int i = 0; i < TERRAINRESOLUTION - 1; i++)
	{
		for (int j = 0; j < TERRAINRESOLUTION - 1; j++)
		{
			DirectX::SimpleMath::Vector3 vertex0 = terrain->m_terrainGeometry[i][j].position;
			DirectX::SimpleMath::Vector3 vertex1 = terrain->m_terrainGeometry[i + 1][j].position;
			DirectX::SimpleMath::Vector3 vertex2 = terrain->m_terrainGeometry[i][j + 1].position;
			DirectX::SimpleMath::Vector3 vertex3 = terrain->m_terrainGeometry[i + 1][j + 1].position;

			Triangle triangle0, triangle1;

			triangle0.vertex0 = vertex0;
			triangle0.vertex1 = vertex1;
			triangle0.vertex2 = vertex3;
			m_triangles.push_back(triangle0);

			triangle1.vertex0 = vertex0;
			triangle1.vertex1 = vertex2;
			triangle1.vertex2 = vertex3;
			m_triangles.push_back(triangle1);
		}
	}
}

void ObjectManipulator::SnapToGround(DisplayChunk* terrain)
{
	// If an object is selected
	if (m_object)
	{
		if (m_object->m_snap_to_ground) 
		{
			float traceLength = 10000; // big number for length of the trace
			DirectX::SimpleMath::Vector3 rayOrigin = m_object->m_position; // start trace at the object's position
			rayOrigin.y += traceLength / 2; // offset by half of trace length so ray traces up and down
			DirectX::SimpleMath::Vector3 rayVector = DirectX::SimpleMath::Vector3(0, -traceLength, 0); // create trace vector

			// Check if the ray intersects with each triangle. If it does, set the object's height to where the intersection occured.
			for (Triangle triangle : m_triangles)
			{
				DirectX::SimpleMath::Vector3 intersectionPoint;

				if (RayIntersectsTriangle(rayOrigin, rayVector, &triangle, intersectionPoint))
				{
					m_object->m_position.y = intersectionPoint.y;
					m_sceneGraph->at(*m_currentSelection).posY = m_object->m_position.y;
					break;
				}
			}
			
			
		}
	}
}

// Implementation of Möller–Trumbore intersection algorithm, source: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
// Adapted for use in DirectX
bool ObjectManipulator::RayIntersectsTriangle(DirectX::SimpleMath::Vector3 rayOrigin, DirectX::SimpleMath::Vector3 rayVector, Triangle* inTriangle, DirectX::SimpleMath::Vector3& outIntersectionPoint)
{
	const float EPSILON = 0.0000001;
	DirectX::SimpleMath::Vector3 vertex0 = inTriangle->vertex0;
	DirectX::SimpleMath::Vector3 vertex1 = inTriangle->vertex1;
	DirectX::SimpleMath::Vector3 vertex2 = inTriangle->vertex2;
	DirectX::SimpleMath::Vector3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = rayVector.Cross(edge2);
	a = edge1.Dot(h);

	if (a > -EPSILON && a < EPSILON)
		return false;    // This ray is parallel to this triangle.

	f = 1.0 / a;
	s = rayOrigin - vertex0;
	u = f * s.Dot(h);

	if (u < 0.0 || u > 1.0)
		return false;

	q = s.Cross(edge1);
	v = f * rayVector.Dot(q);

	if (v < 0.0 || u + v > 1.0)
		return false;

	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * edge2.Dot(q);

	if (t > EPSILON) // ray intersection
	{
		outIntersectionPoint = rayOrigin + rayVector * t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}






