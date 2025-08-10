//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "DisplayObject.h"
#include <string>


using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game()

{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
	m_displayList.clear();
	
	// Set initial values
	m_grid = false;
    m_wireframeObjects = false;
    m_wireframeTerrain = false;
    m_highlight = true;
    m_focus = 2;
    m_focusMin = 1;
    m_focusMax = 10;
    m_focusZoomSpeed = 30;
    m_sculptModeActive = false;
    m_ManipulatorUndoFlag = false;
    m_spherePos = DirectX::SimpleMath::Vector3(0,0,0);
    m_spawnDistance = 3;
    m_toolbarHeight = 16;
    m_terrainSculpter.SetInput(&m_InputCommands);
    m_terrainSculpter.SetToolbarHeight(m_toolbarHeight);
}

Game::~Game()
{

#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    GetClientRect(window, &m_ScreenDimensions);

    // Create primitive sphere
    m_sphere = GeometricPrimitive::CreateSphere(m_deviceResources->GetD3DDeviceContext());

#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

void Game::SetGridState(bool state)
{
	m_grid = state;
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick(InputCommands *Input)
{
	//copy over the input commands so we have a local version to use elsewhere.
	m_InputCommands = *Input;
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    // Update camera with inputs.
    m_camera.Update(timer, &m_InputCommands);

    // When in sculpt mode...
    if (m_sculptModeActive)
    {
        if (m_InputCommands.LMBDown) // if lmb is down, sculpt and update the triangle list for snapping objects to ground
        {
            m_terrainSculpter.Sculpt(&m_displayChunk, m_spherePos, timer);
            m_objectManipulator.CreateTriangles(&m_displayChunk);
        }
    }
    else // when in object manipulation mode, update object manipulator
    {
        m_objectManipulator.Update(timer, &m_InputCommands, &m_camera);
    }

    // Only register object manipulation inputs for undo if they last for more than 0.2s to prevent single clicks to select from being registered as actions
    if (m_objectManipulator.GetActive() && m_objectManipulator.GetClickLength() > 0.2f && !m_ManipulatorUndoFlag)
    {
        m_ManipulatorUndoFlag = true;
        AddAction(Action::MODIFY);
        AddToObjectStack(m_objectManipulator.GetInitialObject());
    }
    else if (!m_objectManipulator.GetActive())
    {
        m_ManipulatorUndoFlag = false;
    }

    // Snap object to ground if needed
    m_objectManipulator.SnapToGround(&m_displayChunk);

	//apply camera vectors
    m_view = Matrix::CreateLookAt(m_camera.GetPosition(), m_camera.GetLookAt(), Vector3::UnitY);

    m_batchEffect->SetView(m_view);
    m_batchEffect->SetWorld(Matrix::Identity);
	m_displayChunk.m_terrainEffect->SetView(m_view);
	m_displayChunk.m_terrainEffect->SetWorld(Matrix::Identity);

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

   
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    //m_batchEffect->SetFogEnabled(true);

	if (m_grid)
	{
		// Draw procedurally generated dynamic grid
		const XMVECTORF32 xaxis = { 512.f, 0.f, 0.f };
		const XMVECTORF32 yaxis = { 0.f, 0.f, 512.f };
		DrawGrid(xaxis, yaxis, g_XMZero, 512, 512, Colors::Gray);
	}
	

 
	//RENDER OBJECTS FROM SCENEGRAPH
	int numRenderObjects = m_displayList.size();
	for (int i = 0; i < numRenderObjects; i++)
	{
        if (m_displayList[i].m_render)
        {

            m_deviceResources->PIXBeginEvent(L"Draw model");


            if (m_currentSelection)
            {
                // If the current object being rendered is the selected object, and highlights are enabled. Toggle fog on.
                if (i == *m_currentSelection && m_highlight)
                {
                    m_displayList[i].m_model->UpdateEffects([&](IEffect* effect)
                        {
                            auto fog = dynamic_cast<IEffectFog*>(effect);
                    if (fog)
                    {

                        // Distance from camera to centre of object
                        float dist = DirectX::SimpleMath::Vector3(m_camera.GetPosition() - m_displayList[i].m_model->meshes[0]->boundingBox.Center).Length();

                        fog->SetFogEnabled(true);

                        // dynamically adjust the fog intensity based on distance
                        fog->SetFogStart(-dist / 2); 
                        fog->SetFogEnd(dist * 2);
                        fog->SetFogColor(Colors::HotPink);

                    }
                        });
                }
                else // Otherwise turn fog off.
                {
                    m_displayList[i].m_model->UpdateEffects([&](IEffect* effect)
                        {
                            auto fog = dynamic_cast<IEffectFog*>(effect);
                    if (fog)
                    {
                        fog->SetFogEnabled(false);

                    }
                        });
                }
            }

            const XMVECTORF32 scale = { m_displayList[i].m_scale.x, m_displayList[i].m_scale.y, m_displayList[i].m_scale.z };
            const XMVECTORF32 translate = { m_displayList[i].m_position.x, m_displayList[i].m_position.y, m_displayList[i].m_position.z };

            //convert degrees into radians for rotation matrix
            XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y * 3.1415 / 180,
                m_displayList[i].m_orientation.x * 3.1415 / 180,
                m_displayList[i].m_orientation.z * 3.1415 / 180);

            XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

            m_displayList[i].m_model->Draw(context, *m_states, local, m_view, m_projection, m_wireframeObjects);	// draw object, wireframe toggle determines how to render it
            


            m_deviceResources->PIXEndEvent();
        }

	}
    m_deviceResources->PIXEndEvent();

	//RENDER TERRAIN
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(),0);
	context->RSSetState(m_states->CullNone());
    if (m_wireframeTerrain) // render wireframe if enabled
    {
        context->RSSetState(m_states->Wireframe());		
    }
	
    

	//Render the batch,  This is handled in the Display chunk becuase it has the potential to get complex
	m_displayChunk.RenderBatch(m_deviceResources);
   
    // If in the sculpt mode, draw sphere at mouse position
    if (m_sculptModeActive)
    {
        float rad = m_terrainSculpter.GetRadius() * 2; 
        
        m_spherePos = LineTraceTerrain(); // Sphere is positioned where the deprojected mouse click line trace hits the terrain.

        // Create matrix for transforming sphere into position
        const XMVECTORF32 translate = { m_spherePos.x, m_spherePos.y, m_spherePos.z };
        XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(0, 0, 0);
        const XMVECTORF32 scale = { rad, rad, rad };
        XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

        // Semi-transparent colour
        DirectX::XMVECTOR colour = DirectX::XMVectorSet(1, 0, 1, 0.25);

        // If possible to sculpt at the mouse's position, draw the sphere
        if (m_terrainSculpter.m_canSculpt)
        {
            m_sphere->Draw(local, m_view, m_projection, colour);
        }
    }

    //CAMERA POSITION ON HUD
    m_sprites->Begin();
    //WCHAR   Buffer[256];
    std::wstring var = L"Camera - X: " + std::to_wstring(m_camera.GetPosition().x) + L", Y: " + std::to_wstring(m_camera.GetPosition().y) + L", Z: " + std::to_wstring(m_camera.GetPosition().z);
    m_font->DrawString(m_sprites.get(), var.c_str(), XMFLOAT2(100, 10), Colors::Yellow);
    m_sprites->End();

    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void XM_CALLCONV Game::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
    m_deviceResources->PIXBeginEvent(L"Draw grid");

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullCounterClockwise());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        float fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::BuildDisplayList(std::vector<SceneObject> * SceneGraph)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();
    m_sceneGraph = SceneGraph;
    m_topID = FindHighestID(SceneGraph);

	if (!m_displayList.empty())		//is the vector empty
	{
		m_displayList.clear();		//if not, empty it
	}

	//for every item in the scenegraph
	int numObjects = SceneGraph->size();
	for (int i = 0; i < numObjects; i++)
	{
		
		//create a temp display object that we will populate then append to the display list.
		DisplayObject newDisplayObject;
		
		//load model
		std::wstring modelwstr = StringToWCHART(SceneGraph->at(i).model_path);							//convect string to Wchar
        newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str(), *m_fxFactory, true);	//get DXSDK to load model "False" for LH coordinate system (maya)
     
		//Load Texture
		std::wstring texturewstr = StringToWCHART(SceneGraph->at(i).tex_diffuse_path);								//convect string to Wchar
		HRESULT rs;
		rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

		//if texture fails.  load error default
		if (rs)
		{
			CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
		}

		//apply new texture to models effect
		newDisplayObject.m_model->UpdateEffects([&](IEffect* effect) //This uses a Lambda function,  if you dont understand it: Look it up.
		{	
			auto lights = dynamic_cast<BasicEffect*>(effect);
			if (lights)
			{
				lights->SetTexture(newDisplayObject.m_texture_diffuse);			
			}
		});

		//set position
		newDisplayObject.m_position.x = SceneGraph->at(i).posX;
		newDisplayObject.m_position.y = SceneGraph->at(i).posY;
		newDisplayObject.m_position.z = SceneGraph->at(i).posZ;
		
		//setorientation
		newDisplayObject.m_orientation.x = SceneGraph->at(i).rotX;
		newDisplayObject.m_orientation.y = SceneGraph->at(i).rotY;
		newDisplayObject.m_orientation.z = SceneGraph->at(i).rotZ;

		//set scale
		newDisplayObject.m_scale.x = SceneGraph->at(i).scaX;
		newDisplayObject.m_scale.y = SceneGraph->at(i).scaY;
		newDisplayObject.m_scale.z = SceneGraph->at(i).scaZ;

		//set wireframe / render flags
		newDisplayObject.m_render		= SceneGraph->at(i).editor_render;
		newDisplayObject.m_wireframe	= SceneGraph->at(i).editor_wireframe;
        newDisplayObject.m_snap_to_ground = SceneGraph->at(i).snapToGround;

		newDisplayObject.m_light_type		= SceneGraph->at(i).light_type;
		newDisplayObject.m_light_diffuse_r	= SceneGraph->at(i).light_diffuse_r;
		newDisplayObject.m_light_diffuse_g	= SceneGraph->at(i).light_diffuse_g;
		newDisplayObject.m_light_diffuse_b	= SceneGraph->at(i).light_diffuse_b;
		newDisplayObject.m_light_specular_r = SceneGraph->at(i).light_specular_r;
		newDisplayObject.m_light_specular_g = SceneGraph->at(i).light_specular_g;
		newDisplayObject.m_light_specular_b = SceneGraph->at(i).light_specular_b;
		newDisplayObject.m_light_spot_cutoff = SceneGraph->at(i).light_spot_cutoff;
		newDisplayObject.m_light_constant	= SceneGraph->at(i).light_constant;
		newDisplayObject.m_light_linear		= SceneGraph->at(i).light_linear;
		newDisplayObject.m_light_quadratic	= SceneGraph->at(i).light_quadratic;
		
		m_displayList.push_back(newDisplayObject);
		
	}
		
    // Set object for manipulating
    if (*m_currentSelection != -1)
    {
        m_objectManipulator.SetObject(&m_displayList[*m_currentSelection]);
    }
		
}

void Game::BuildDisplayChunk(ChunkObject * SceneChunk)
{
	//populate our local DISPLAYCHUNK with all the chunk info we need from the object stored in toolmain
	//which, to be honest, is almost all of it. Its mostly rendering related info so...
	m_displayChunk.PopulateChunkData(SceneChunk);		//migrate chunk data
	m_displayChunk.LoadHeightMap(m_deviceResources);
	m_displayChunk.m_terrainEffect->SetProjection(m_projection);
	m_displayChunk.InitialiseBatch();
    m_objectManipulator.CreateTriangles(&m_displayChunk); // generate triangle data
}

void Game::SaveDisplayChunk(ChunkObject * SceneChunk)
{
	m_displayChunk.SaveHeightMap();			//save heightmap to file.
}

void Game::SetManipulationMode(ManipulationMode mode)
{
    m_objectManipulator.SetMode(mode);
}

ManipulationMode Game::GetManipulationMode()
{
    return m_objectManipulator.GetMode();
}

DirectX::SimpleMath::Vector3 Game::LineTraceTerrain()
{
    // Get terrain triangles
    std::vector<Triangle> triangles = m_objectManipulator.GetTriangles();

    // Near and far plane of click
    const XMVECTOR nearSource = XMVectorSet(m_InputCommands.pickerX, m_InputCommands.pickerY, 0.0f, 1.0f);
    const XMVECTOR farSource = XMVectorSet(m_InputCommands.pickerX, m_InputCommands.pickerY, 1.0f, 1.0f);

    GetClientRect(GetActiveWindow(), &m_ScreenDimensions);

    //Unproject the points on the near and far plane, with respect to the matrix we just created.
    XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, m_world);
    XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, m_world);

    //turn the transformed points into our picking vector. 
    XMVECTOR pickingVector = farPoint - nearPoint;
    pickingVector = XMVector3Normalize(pickingVector);

    DirectX::SimpleMath::Vector3 rayOrigin = DirectX::SimpleMath::Vector3(nearPoint);
    DirectX::SimpleMath::Vector3 rayVector = DirectX::SimpleMath::Vector3(pickingVector);
    DirectX::SimpleMath::Vector3 intersectionPoint;

    // Check if ray intersects each triangle. If it does, it is possible to sculpt the terrain.
    for (Triangle triangle : triangles)
    {
        if (ObjectManipulator::RayIntersectsTriangle(rayOrigin, rayVector, &triangle, intersectionPoint))
        {
            m_terrainSculpter.m_canSculpt = true;
            return intersectionPoint;
        }
    }

    m_terrainSculpter.m_canSculpt = false;
    return DirectX::SimpleMath::Vector3(0, 0, 0);
}

int Game::MousePicking(int curID)
{

    HWND ActiveWindow = GetActiveWindow();
    
    if (GetParent(ActiveWindow) != 0) // if not clicking the main window, we can ignore the click
    {
        return curID;
    }

    GetClientRect(ActiveWindow, &m_ScreenDimensions); // update screen dimensions if window size has changed


    int selectedID = -1;
    float pickedDistance = 0;
    float closestDistance = 9999999; // big number for initial closest distance, first object will be closer than this
    bool hit = false;

    std::vector<int> intersectedObjects;
    std::vector<float> intersectedDistances;


    if (m_InputCommands.pickerY > m_toolbarHeight) // only check if the user has clicked below the toolbar
    {

    //setup near and far planes of frustum with mouse X and mouse y passed down from Toolmain. 
    //they may look the same but note, the difference in Z
    const XMVECTOR nearSource = XMVectorSet(m_InputCommands.pickerX, m_InputCommands.pickerY, 0.0f, 1.0f);
    const XMVECTOR farSource = XMVectorSet(m_InputCommands.pickerX, m_InputCommands.pickerY, 1.0f, 1.0f);

    //Loop through entire display list of objects and pick with each in turn. 
    for (int i = 0; i < m_displayList.size(); i++)
    {
        //Get the scale factor and translation of the object
        const XMVECTORF32 scale = { m_displayList[i].m_scale.x,		m_displayList[i].m_scale.y,		m_displayList[i].m_scale.z };
        const XMVECTORF32 translate = { m_displayList[i].m_position.x,	m_displayList[i].m_position.y,	m_displayList[i].m_position.z };

        //convert euler angles into a quaternion for the rotation of the object
        XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y * 3.1415 / 180,
            m_displayList[i].m_orientation.x * 3.1415 / 180,
            m_displayList[i].m_orientation.z * 3.1415 / 180);

        //create set the matrix of the selected object in the world based on the translation, scale and rotation.
        XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

        //Unproject the points on the near and far plane, with respect to the matrix we just created.
        XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);
        XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);

        //turn the transformed points into our picking vector. 
        XMVECTOR pickingVector = farPoint - nearPoint;
        pickingVector = XMVector3Normalize(pickingVector);

        //loop through mesh list for object
        for (int y = 0; y < m_displayList[i].m_model.get()->meshes.size(); y++)
        {
            //checking for ray intersection
            if (m_displayList[i].m_model.get()->meshes[y]->boundingBox.Intersects(nearPoint, pickingVector, pickedDistance))
            {
                intersectedObjects.push_back(i);
                intersectedDistances.push_back(pickedDistance);
            }
        }
    }

    // Add models to vector and check their current IDs. If none are the current ID, select closest. If one is current ID, switch to next object.
    for (int i = 0; i < intersectedObjects.size(); i++)
    {
        if (intersectedObjects[i] == curID)
        {
            if (i == intersectedObjects.size() - 1)
            {
                selectedID = intersectedObjects[0];
            }
            else
            {
                selectedID = intersectedObjects[i + 1];
            }
           
            break;
        }
        else if (intersectedDistances[i] < closestDistance)
        {
           selectedID = intersectedObjects[i];
           closestDistance = intersectedDistances[i];
        }
    }

    // If valid object found, set the object manipulator's object
    if (selectedID >= 0)
    {
        m_objectManipulator.SetObject(&m_displayList[selectedID]);
    }
    else
    {
        m_objectManipulator.SetObject(NULL);
    }
    
    //if we got a hit.  return it.  
    return selectedID;
    }
    else // return previous id if click is out of bounds
    {
        return curID;
    }
   
}

void Game::FocusObject(int objID) // moves camera to the object
{
    if (objID >= 0) // make sure there is a selected object
    {
        // Position of object used for calculating new camera position
        DirectX::SimpleMath::Vector3 objPosition, newCamPosition;
        objPosition = m_displayList[objID].m_position;

        // Focus on centre of mesh
        objPosition += m_displayList[objID].m_model.get()->meshes[0]->boundingBox.Center * m_displayList[objID].m_scale.x;

        // Calculate focus distance
        auto bounds = m_displayList[objID].m_model.get()->meshes[0]->boundingBox.Extents;
        float dimensions[3] = { bounds.x, bounds.y, bounds.z };
        float focusDistance = *std::max_element(dimensions, dimensions + 3) * abs(m_displayList[objID].m_scale.x) * m_focus; // get largest dimension, then orbit at focus distance multiplier.

        // Update camera position, starts lerping
        newCamPosition = objPosition + m_camera.GetForward() * -focusDistance; 
        m_camera.SetPosition(newCamPosition);
    }
}

void Game::ScrollWheel(short delta)
{
    // Scroll wheel behaviour.
    if (delta > 0) // Decrease focus distance.
    {
        m_focus -= m_timer.GetElapsedSeconds() * m_focusZoomSpeed;
    }
    else // Increase focus distance.
    {
        m_focus += m_timer.GetElapsedSeconds() * m_focusZoomSpeed;
    }

    // Clamp to min and max values.
    if (m_focus < m_focusMin)
    {
        m_focus = m_focusMin;
    }
    else if (m_focus > m_focusMax)
    {
        m_focus = m_focusMax;
    }
}

void Game::AddToObjectStack(SceneObject object)
{
    ClearRedo(); // new branch so can no longer redo
    m_undoObjectStack.push(object); // add object to undo stack
}

void Game::ClearUndoRedo()
{
    // Pop all items from stacks
    while (!m_undoStack.empty())
        m_undoStack.pop();
    
    while (!m_redoStack.empty())
        m_redoStack.pop();
    
    while (!m_undoObjectStack.empty())
        m_undoObjectStack.pop();

    while (!m_redoObjectStack.empty())
        m_redoObjectStack.pop();

}

void Game::ClearRedo()
{
    // Pop all items from redo stack
    while (!m_redoStack.empty())
        m_redoStack.pop();

    while (!m_redoObjectStack.empty())
        m_redoObjectStack.pop();
}

void Game::Undo()
{
    if (!m_undoStack.empty()) // only works when there are actions to undo
    {
        // Get action and object
        Action action = m_undoStack.top();
        SceneObject oldObject = m_undoObjectStack.top();
        SceneObject* currentObject;
        int index;
        
        // Add to redo stack, pop undo stakcs
        m_redoStack.push(action);
        m_undoStack.pop();
        m_undoObjectStack.pop();

        switch (action)
        {
        case Action::ADD:
            // Delete the object, push the old one to the redo stack
            currentObject = GetObjectByID(oldObject.ID, index);
            m_redoObjectStack.push(oldObject);
            DeleteSceneObject(index);
            m_topID--;
            //MessageBox(NULL, L"Add object undone.", L"Notification", MB_OK);
            break;
        case Action::MODIFY:
            // Apply changes from old object to the current object after pushing the current object to the stack
            currentObject = GetObjectByID(oldObject.ID, index);
            m_redoObjectStack.push(*currentObject);
            ApplyChanges(currentObject, oldObject);
            //MessageBox(NULL, L"Modify object undone.", L"Notification", MB_OK);
            break;
        case Action::REMOVE:
            // Add the object back to the scene graph, push to redo stack
            m_sceneGraph->push_back(oldObject);
            currentObject = &m_sceneGraph->back();
            m_redoObjectStack.push(*currentObject);
            //MessageBox(NULL, L"Remove object undone.", L"Notification", MB_OK);
            break;
        }

        BuildDisplayList(m_sceneGraph); // rebuild display list
    }
}

void Game::Redo()
{
    if (!m_redoStack.empty()) // only works when there are actions to redo
    {
        // Get action and object
        Action action = m_redoStack.top();
        SceneObject oldObject = m_redoObjectStack.top();
        SceneObject* currentObject;
        int index;

        // add back to undo stack, pop redo stacks
        m_undoStack.push(action);
        m_redoStack.pop();
        m_redoObjectStack.pop();

        switch (action)
        {
        case Action::ADD:
            // Add object back to scene graph and undo stack
            m_sceneGraph->push_back(oldObject);
            m_undoObjectStack.push(m_sceneGraph->back());
            //MessageBox(NULL, L"Add object redone.", L"Notification", MB_OK);
            break;
        case Action::MODIFY:
            // Apply changes from old object to the current object after pushing the current object to the stack
            currentObject = GetObjectByID(oldObject.ID, index);
            m_undoObjectStack.push(*currentObject);
            ApplyChanges(currentObject, oldObject);
            //MessageBox(NULL, L"Modify object redone.", L"Notification", MB_OK);
            break;
        case Action::REMOVE:
            // Delete object from the scene graph
            currentObject = GetObjectByID(oldObject.ID, index);
            m_undoObjectStack.push(*currentObject);
            DeleteSceneObject(index);
            //MessageBox(NULL, L"Remove object redone.", L"Notification", MB_OK);
            break;
        }
        BuildDisplayList(m_sceneGraph); // rebuild display list
    }
}

SceneObject* Game::GetObjectByID(int ID, int& returnIndex)
{
    // check each objects ID, return the object and position in the scene graph if there is a match
    for (int i = 0; i < m_sceneGraph->size(); i++) 
    {
        if (m_sceneGraph->at(i).ID == ID)
        {
            returnIndex = i;
            return &m_sceneGraph->at(i);
        }
    }

    return nullptr;
}

void Game::PositionClashCheck(SceneObject* newSceneObject)
{
    // Check for position clash when adding new objects
    bool positionCheckComplete = false;
    bool clashFound = false;
    DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(newSceneObject->posX, newSceneObject->posY, newSceneObject->posZ);

    while (!positionCheckComplete)
    {
        clashFound = false;

        for (SceneObject object : *m_sceneGraph)
        {
            if (newSceneObject->posX == object.posX && newSceneObject->posY == object.posY && newSceneObject->posZ == object.posZ) // if position matches, move to the right
            {
                position += m_camera.GetRight();
                newSceneObject->posX = position.x;
                newSceneObject->posY = position.y;
                newSceneObject->posZ = position.z;
                clashFound = true;
            }
        }

        if (!clashFound)
        {
            positionCheckComplete = true;
        }
    }
}

void Game::ApplyChanges(SceneObject* newObject, SceneObject oldObject)
{
    // Assign relevant values
    newObject->posX = oldObject.posX;
    newObject->posY = oldObject.posY;
    newObject->posZ = oldObject.posZ;
    newObject->rotX = oldObject.rotX;
    newObject->rotY = oldObject.rotY;
    newObject->rotZ = oldObject.rotZ;
    newObject->scaX = oldObject.scaX;
    newObject->scaY = oldObject.scaY;
    newObject->scaZ = oldObject.scaZ;
    newObject->editor_render = oldObject.editor_render;
    newObject->snapToGround = oldObject.snapToGround;
    newObject->model_path = oldObject.model_path;
    newObject->tex_diffuse_path = oldObject.tex_diffuse_path;
}

void Game::DeleteSceneObject(int index)
{
    // Remove object from scene graph
    m_sceneGraph->erase(m_sceneGraph->begin() + index);
    *m_currentSelection = -1;
    BuildDisplayList(m_sceneGraph);
    GetManipulator()->SetObject(NULL);
}

void Game::AddSceneObject()
{
    // Add object to scene graph
    // New ID
    m_topID++;

    // Position in front of camerea
    DirectX::SimpleMath::Vector3 position = m_camera.GetPosition() + m_camera.GetForward() * m_spawnDistance;
    
    // Default values, rest set in constructor
    SceneObject newSceneObject;
    newSceneObject.ID = m_topID;
    newSceneObject.model_path = "database/data/placeholder.cmo";
    newSceneObject.tex_diffuse_path = "database/data/placeholder.dds";
    newSceneObject.posX = position.x;
    newSceneObject.posY = position.y;
    newSceneObject.posZ = position.z;
    newSceneObject.scaX = 1;
    newSceneObject.scaY = 1;
    newSceneObject.scaZ = 1;

    // Adjust object position if another object already shares its position
    PositionClashCheck(&newSceneObject);
    
    //send completed object to scenegraph
    m_sceneGraph->push_back(newSceneObject);

    BuildDisplayList(m_sceneGraph);
    *m_currentSelection = GetDisplayList()->size() - 1;
    GetManipulator()->SetObject(&GetDisplayList()->back());
   
}

int Game::FindHighestID(std::vector<SceneObject>* sceneGraph)
{
    // Returns highest ID in scene graph
    int highestID = -1;
    
    for (SceneObject object : *sceneGraph)
    {
        if (object.ID > highestID)
        {
            highestID = object.ID;
        }
    }

    return highestID;
}

#ifdef DXTK_AUDIO

void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif


#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    m_fxFactory = std::make_unique<EffectFactory>(device);
	m_fxFactory->SetDirectory(L"database/data/"); //fx Factory will look in the database directory
	m_fxFactory->SetSharing(false);	//we must set this to false otherwise it will share effects based on the initial tex loaded (When the model loads) rather than what we will change them to.

    m_sprites = std::make_unique<SpriteBatch>(context);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
                VertexPositionColor::InputElementCount,
                shaderByteCode, byteCodeLength,
                m_batchInputLayout.ReleaseAndGetAddressOf())
        );
    }

    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");

//    m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

    // SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
    m_model = Model::CreateFromSDKMESH(device, L"tiny.sdkmesh", *m_fxFactory);
	

    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
    );

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
    );

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_batchEffect->SetProjection(m_projection);
	
}

void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_font.reset();
    m_shape.reset();
    m_model.reset();
    m_texture1.Reset();
    m_texture2.Reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

std::wstring StringToWCHART(std::string s)
{

	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
