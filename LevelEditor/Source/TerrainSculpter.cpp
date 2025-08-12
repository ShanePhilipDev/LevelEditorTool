#include "TerrainSculpter.h"

TerrainSculpter::TerrainSculpter()
{
	// Initial values
	m_sculptMode = SculptMode::RAISE;
	m_radius = 5.0f;
	m_magnitude = 5.0f;
	m_canSculpt = true;
	m_editHeightMap = false;
}

TerrainSculpter::~TerrainSculpter()
{
}

void TerrainSculpter::Sculpt(DisplayChunk* terrain, DirectX::SimpleMath::Vector3 spherePos, DX::StepTimer const& timer)
{

	// Required conditions for sculpting: clicking main window below the toolbar, while hovering over terrain (m_canSculpt)
	if (GetParent(GetActiveWindow()) == 0 && m_inputCommands->pickerY > m_toolbarHeight && m_canSculpt) 
	{
		// For each vertex in the terrain...
		for (int i = 0; i < TERRAINRESOLUTION - 1; i++)
		{
			for (int j = 0; j < TERRAINRESOLUTION - 1; j++)
			{
				DirectX::SimpleMath::Vector3 pos1, pos2;

				// Sphere position
				pos1 = spherePos;
				pos1.y = 0; // Y position not important so ignored, works a bit smoother like this.

				// Terrain vertex position
				pos2 = terrain->m_terrainGeometry[i][j].position;
				pos2.y = 0;

				// Distance between the two points
				float distance = DirectX::SimpleMath::Vector3::Distance(pos1, pos2);

				// If the distance is lower than the radius of the sphere, that point should be sculpted.
				if (distance < m_radius)
				{
					// Heightmap is 1D array while vertex grid is 2D. This gets relevant index in heightmap.
					int index = (TERRAINRESOLUTION * i) + j;

					// Amount to extrude by. Magnitude of extrusion is half at the edge compared to the centre.
					float magnitude = MapFloat(distance, 0, m_radius, m_magnitude, m_magnitude / 2) * timer.GetElapsedSeconds(); 

					// Choose action based on sculpt mode.
					// Either raises, lowers or flattens terrain.
					// Depending on the edit heightmap toggle, it will either adjust the heightmap values or adjust the vertex position.
					switch (m_sculptMode)
					{
					case SculptMode::RAISE:
						if (m_editHeightMap)
						{
							terrain->GenerateHeightmap(index, magnitude);
						}
						else
						{
							terrain->m_terrainGeometry[i][j].position.y += magnitude;
						}
						break;
					case SculptMode::LOWER:
						if (m_editHeightMap)
						{
							terrain->GenerateHeightmap(index, -magnitude);
						}
						else
						{
							terrain->m_terrainGeometry[i][j].position.y -= magnitude;
						}
						break;
					case SculptMode::FLATTEN:
						if (m_editHeightMap)
						{
							terrain->FlattenHeightmap(index);
						}
						else
						{
							terrain->m_terrainGeometry[i][j].position.y = 0;
						}
						break;
					}

				}
			}
		}
	}
	
}

float TerrainSculpter::MapFloat(float f, float in1, float in2, float out1, float out2)
{
	return out1 + (f - in1) * (out2 - out1) / (in2 - in1);
}
