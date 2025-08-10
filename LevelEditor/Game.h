//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SceneObject.h"
#include "DisplayObject.h"
#include "DisplayChunk.h"
#include "ChunkObject.h"
#include "InputCommands.h"
#include <vector>
#include "Camera.h"
#include "ObjectManipulator.h"
#include "DirectXMath.h"
#include "TerrainSculpter.h"
#include <stack>

// A basic game implementation that creates a D3D11 device and
// provides a game loop.

// Undo/redo actions
enum class Action
{
	MODIFY,
	ADD,
	REMOVE
};

class Game : public DX::IDeviceNotify
{
public:

	Game();
	~Game();

	// Initialization and management
	void Initialize(HWND window, int width, int height);
	void SetGridState(bool state);

	// Basic game loop
	void Tick(InputCommands * Input);
	void Render();

	// Rendering helpers
	void Clear();

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int height);

	//tool specific
	void BuildDisplayList(std::vector<SceneObject> * SceneGraph); //note vector passed by reference 
	void BuildDisplayChunk(ChunkObject *SceneChunk);
	void SaveDisplayChunk(ChunkObject *SceneChunk);	//saves geometry et al
	void ClearDisplayList();
	std::vector<DisplayObject>* GetDisplayList() { return &m_displayList; };
	float GetDeltaTime() { return m_timer.GetElapsedSeconds(); };

	// Object manipulation functions
	int MousePicking(int curID);
	void SetManipulationMode(ManipulationMode mode);
	void SetSelection(int* sel) { m_currentSelection = sel; };
	void SetManipulatorSceneGraph(std::vector<SceneObject>* sceneGraph, int* sel) { m_objectManipulator.SetSceneGraph(sceneGraph, sel); };
	ManipulationMode GetManipulationMode();
	ObjectManipulator* GetManipulator() { return &m_objectManipulator; };

	// Sculpt mode functions
	DirectX::SimpleMath::Vector3 LineTraceTerrain();
	bool GetSculptModeActive() { return m_sculptModeActive; };
	void SetSculptModeActive(bool active) { m_sculptModeActive = active; }
	SculptMode GetSculptMode() { return m_terrainSculpter.GetMode(); }
	void SetSculptMode(SculptMode mode) { m_terrainSculpter.SetMode(mode); };
	TerrainSculpter* GetSculpter() { return &m_terrainSculpter; };

	// Wireframe functions
	void ToggleWireframeObjects() { m_wireframeObjects = !m_wireframeObjects; };
	void ToggleWireframeTerrain() { m_wireframeTerrain = !m_wireframeTerrain; };

	// Highlight object getter/setter
	void SetHighlight(bool highlight) { m_highlight = highlight; };
	bool GetHighlight() { return m_highlight; };

	// Camera functions
	void FocusObject(int objID);
	void ScrollWheel(short delta);
	Camera* GetCamera() { return &m_camera; };
	float GetZoomSpeed() { return m_focusZoomSpeed; };
	void SetZoomSpeed(float s) { m_focusZoomSpeed = s; };

	// Getters for display chunk and toolbar height
	DisplayChunk* GetDisplayChunk() { return &m_displayChunk; };
	float GetToolbarHeight() { return m_toolbarHeight; };

	// Undo/redo functions
	std::stack<Action> GetUndoStack() { return m_undoStack; };
	std::stack<Action> GetRedoStack() { return m_redoStack; };
	void Undo();
	void Redo();
	void AddAction(Action action) { m_undoStack.push(action); };
	void AddToObjectStack(SceneObject object);
	void ClearUndoRedo();
	void ClearRedo();

	// Object functions
	int FindHighestID(std::vector<SceneObject>* sceneGraph);
	SceneObject* GetObjectByID(int ID, int& returnIndex);
	void PositionClashCheck(SceneObject* object);

	// Add, remove, modify objects
	void ApplyChanges(SceneObject* newObject, SceneObject oldObject);
	void DeleteSceneObject(int index);
	void AddSceneObject();
	
	// Highest ID, used for making new objects
	int m_topID;
#ifdef DXTK_AUDIO
	void NewAudioDevice();
#endif

private:
	

	void Update(DX::StepTimer const& timer);

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color);

	// Undo/redo variables
	std::stack<Action> m_undoStack;
	std::stack<Action> m_redoStack;
	std::stack<SceneObject> m_undoObjectStack;
	std::stack<SceneObject> m_redoObjectStack;
	bool m_ManipulatorUndoFlag;

	// Scene graph pointer
	std::vector<SceneObject>* m_sceneGraph;
	int* m_currentSelection;

	//tool specific
	std::vector<DisplayObject>			m_displayList;
	DisplayChunk						m_displayChunk;
	InputCommands						m_InputCommands;

	// Camera, object manipulation, and terrain editing classes
	Camera								m_camera;
	ObjectManipulator					m_objectManipulator;
	TerrainSculpter						m_terrainSculpter;

	// Toggles
	bool m_sculptModeActive;
	bool m_wireframeObjects;
	bool m_wireframeTerrain;
	bool m_highlight;

	// Window properties
	RECT m_ScreenDimensions;
	int m_toolbarHeight;

	// Sphere for terrain editing
	std::unique_ptr<DirectX::GeometricPrimitive> m_sphere;
	DirectX::SimpleMath::Vector3 m_spherePos;

	// Camera focus variables
	float m_focus;
	float m_focusMin;
	float m_focusMax;
	float m_focusZoomSpeed;
	float m_spawnDistance;

	
	//control variables
	bool m_grid;							//grid rendering on / off
	// Device resources.
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_shape;
    std::unique_ptr<DirectX::Model>                                         m_model;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture2;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;


};

std::wstring StringToWCHART(std::string s);