#include "ToolMain.h"
#include "resource.h"
#include <vector>
#include <sstream>

//
//ToolMain Class
ToolMain::ToolMain()
{

	m_currentChunk = 0;		//default value
	m_selectedObject = 0;	//initial selection ID
	m_sceneGraph.clear();	//clear the vector for the scenegraph
	m_databaseConnection = NULL;
	m_d3dRenderer.SetSelection(&m_selectedObject);

	ZeroMemory(&m_toolInputCommands, sizeof(InputCommands)); // initialise struct to zero

	// initial values
	m_haveCopiedObject = false;
	m_leftClickTimer = 0;
	m_actionCooldown = 0.25;
	m_actionCooldownTimer = 0;
}


ToolMain::~ToolMain()
{
	sqlite3_close(m_databaseConnection);		//close the database connection
}


int ToolMain::getCurrentSelectionID()
{
	return m_selectedObject;
}

void ToolMain::onActionInitialise(HWND handle, int width, int height)
{
	//window size, handle etc for directX
	m_width		= width;
	m_height	= height;
	
	m_d3dRenderer.Initialize(handle, m_width, m_height);
	

	//database connection establish
	int rc;
	rc = sqlite3_open_v2("database/test.db",&m_databaseConnection, SQLITE_OPEN_READWRITE, NULL);

	if (rc) 
	{
		TRACE("Can't open database");
		//if the database cant open. Perhaps a more catastrophic error would be better here
	}
	else 
	{
		TRACE("Opened database successfully");
	}

	onActionLoad();
}

void ToolMain::onActionLoad()
{
	//load current chunk and objects into lists
	if (!m_sceneGraph.empty())		//is the vector empty
	{
		m_sceneGraph.clear();		//if not, empty it
	}

	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	sqlite3_stmt *pResultsChunk;

	//OBJECTS IN THE WORLD
	//prepare SQL Text
	sqlCommand = "SELECT * from Objects";				//sql command which will return all records from the objects table.
	//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0 );
	
	//loop for each row in results until there are no more rows.  ie for every row in the results. We create and object
	while (sqlite3_step(pResults) == SQLITE_ROW)
	{	
		SceneObject newSceneObject;
		newSceneObject.ID = sqlite3_column_int(pResults, 0);
		newSceneObject.chunk_ID = sqlite3_column_int(pResults, 1);
		newSceneObject.model_path		= reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		newSceneObject.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		newSceneObject.posX = sqlite3_column_double(pResults, 4);
		newSceneObject.posY = sqlite3_column_double(pResults, 5);
		newSceneObject.posZ = sqlite3_column_double(pResults, 6);
		newSceneObject.rotX = sqlite3_column_double(pResults, 7);
		newSceneObject.rotY = sqlite3_column_double(pResults, 8);
		newSceneObject.rotZ = sqlite3_column_double(pResults, 9);
		newSceneObject.scaX = sqlite3_column_double(pResults, 10);
		newSceneObject.scaY = sqlite3_column_double(pResults, 11);
		newSceneObject.scaZ = sqlite3_column_double(pResults, 12);
		newSceneObject.render = sqlite3_column_int(pResults, 13);
		newSceneObject.collision = sqlite3_column_int(pResults, 14);
		newSceneObject.collision_mesh = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 15));
		newSceneObject.collectable = sqlite3_column_int(pResults, 16);
		newSceneObject.destructable = sqlite3_column_int(pResults, 17);
		newSceneObject.health_amount = sqlite3_column_int(pResults, 18);
		newSceneObject.editor_render = sqlite3_column_int(pResults, 19);
		newSceneObject.editor_texture_vis = sqlite3_column_int(pResults, 20);
		newSceneObject.editor_normals_vis = sqlite3_column_int(pResults, 21);
		newSceneObject.editor_collision_vis = sqlite3_column_int(pResults, 22);
		newSceneObject.editor_pivot_vis = sqlite3_column_int(pResults, 23);
		newSceneObject.pivotX = sqlite3_column_double(pResults, 24);
		newSceneObject.pivotY = sqlite3_column_double(pResults, 25);
		newSceneObject.pivotZ = sqlite3_column_double(pResults, 26);
		newSceneObject.snapToGround = sqlite3_column_int(pResults, 27);
		newSceneObject.AINode = sqlite3_column_int(pResults, 28);
		newSceneObject.audio_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 29));
		newSceneObject.volume = sqlite3_column_double(pResults, 30);
		newSceneObject.pitch = sqlite3_column_double(pResults, 31);
		newSceneObject.pan = sqlite3_column_int(pResults, 32);
		newSceneObject.one_shot = sqlite3_column_int(pResults, 33);
		newSceneObject.play_on_init = sqlite3_column_int(pResults, 34);
		newSceneObject.play_in_editor = sqlite3_column_int(pResults, 35);
		newSceneObject.min_dist = sqlite3_column_double(pResults, 36);
		newSceneObject.max_dist = sqlite3_column_double(pResults, 37);
		newSceneObject.camera = sqlite3_column_int(pResults, 38);
		newSceneObject.path_node = sqlite3_column_int(pResults, 39);
		newSceneObject.path_node_start = sqlite3_column_int(pResults, 40);
		newSceneObject.path_node_end = sqlite3_column_int(pResults, 41);
		newSceneObject.parent_id = sqlite3_column_int(pResults, 42);
		newSceneObject.editor_wireframe = sqlite3_column_int(pResults, 43);
		newSceneObject.name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));

		newSceneObject.light_type = sqlite3_column_int(pResults, 45);
		newSceneObject.light_diffuse_r = sqlite3_column_double(pResults, 46);
		newSceneObject.light_diffuse_g = sqlite3_column_double(pResults, 47);
		newSceneObject.light_diffuse_b = sqlite3_column_double(pResults, 48);
		newSceneObject.light_specular_r = sqlite3_column_double(pResults, 49);
		newSceneObject.light_specular_g = sqlite3_column_double(pResults, 50);
		newSceneObject.light_specular_b = sqlite3_column_double(pResults, 51);
		newSceneObject.light_spot_cutoff = sqlite3_column_double(pResults, 52);
		newSceneObject.light_constant = sqlite3_column_double(pResults, 53);
		newSceneObject.light_linear = sqlite3_column_double(pResults, 54);
		newSceneObject.light_quadratic = sqlite3_column_double(pResults, 55);
	

		//send completed object to scenegraph
		m_sceneGraph.push_back(newSceneObject);
	}

	//THE WORLD CHUNK
	//prepare SQL Text
	sqlCommand = "SELECT * from Chunks";				//sql command which will return all records from  chunks table. There is only one tho.
														//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResultsChunk, 0);


	sqlite3_step(pResultsChunk);
	m_chunk.ID = sqlite3_column_int(pResultsChunk, 0);
	m_chunk.name = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 1));
	m_chunk.chunk_x_size_metres = sqlite3_column_int(pResultsChunk, 2);
	m_chunk.chunk_y_size_metres = sqlite3_column_int(pResultsChunk, 3);
	m_chunk.chunk_base_resolution = sqlite3_column_int(pResultsChunk, 4);
	m_chunk.heightmap_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 5));
	m_chunk.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 6));
	m_chunk.tex_splat_alpha_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 7));
	m_chunk.tex_splat_1_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 8));
	m_chunk.tex_splat_2_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 9));
	m_chunk.tex_splat_3_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 10));
	m_chunk.tex_splat_4_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 11));
	m_chunk.render_wireframe = sqlite3_column_int(pResultsChunk, 12);
	m_chunk.render_normals = sqlite3_column_int(pResultsChunk, 13);
	m_chunk.tex_diffuse_tiling = sqlite3_column_int(pResultsChunk, 14);
	m_chunk.tex_splat_1_tiling = sqlite3_column_int(pResultsChunk, 15);
	m_chunk.tex_splat_2_tiling = sqlite3_column_int(pResultsChunk, 16);
	m_chunk.tex_splat_3_tiling = sqlite3_column_int(pResultsChunk, 17);
	m_chunk.tex_splat_4_tiling = sqlite3_column_int(pResultsChunk, 18);


	//Process REsults into renderable
	m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	//build the renderable chunk 
	m_d3dRenderer.BuildDisplayChunk(&m_chunk);

	//setup for object manipulation
	m_d3dRenderer.SetManipulatorSceneGraph(&m_sceneGraph, &m_selectedObject);
}

void ToolMain::onActionSave()
{
	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	

	//OBJECTS IN THE WORLD Delete them all
	//prepare SQL Text
	sqlCommand = "DELETE FROM Objects";	 //will delete the whole object table.   Slightly risky but hey.
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);
	sqlite3_step(pResults);

	//Populate with our new objects
	std::wstring sqlCommand2;
	int numObjects = m_sceneGraph.size();	//Loop thru the scengraph.

	for (int i = 0; i < numObjects; i++)
	{
		std::stringstream command;
		command << "INSERT INTO Objects " 
			<<"VALUES(" << m_sceneGraph.at(i).ID << ","
			<< m_sceneGraph.at(i).chunk_ID  << ","
			<< "'" << m_sceneGraph.at(i).model_path <<"'" << ","
			<< "'" << m_sceneGraph.at(i).tex_diffuse_path << "'" << ","
			<< m_sceneGraph.at(i).posX << ","
			<< m_sceneGraph.at(i).posY << ","
			<< m_sceneGraph.at(i).posZ << ","
			<< m_sceneGraph.at(i).rotX << ","
			<< m_sceneGraph.at(i).rotY << ","
			<< m_sceneGraph.at(i).rotZ << ","
			<< m_sceneGraph.at(i).scaX << ","
			<< m_sceneGraph.at(i).scaY << ","
			<< m_sceneGraph.at(i).scaZ << ","
			<< m_sceneGraph.at(i).render << ","
			<< m_sceneGraph.at(i).collision << ","
			<< "'" << m_sceneGraph.at(i).collision_mesh << "'" << ","
			<< m_sceneGraph.at(i).collectable << ","
			<< m_sceneGraph.at(i).destructable << ","
			<< m_sceneGraph.at(i).health_amount << ","
			<< m_sceneGraph.at(i).editor_render << ","
			<< m_sceneGraph.at(i).editor_texture_vis << ","
			<< m_sceneGraph.at(i).editor_normals_vis << ","
			<< m_sceneGraph.at(i).editor_collision_vis << ","
			<< m_sceneGraph.at(i).editor_pivot_vis << ","
			<< m_sceneGraph.at(i).pivotX << ","
			<< m_sceneGraph.at(i).pivotY << ","
			<< m_sceneGraph.at(i).pivotZ << ","
			<< m_sceneGraph.at(i).snapToGround << ","
			<< m_sceneGraph.at(i).AINode << ","
			<< "'" << m_sceneGraph.at(i).audio_path << "'" << ","
			<< m_sceneGraph.at(i).volume << ","
			<< m_sceneGraph.at(i).pitch << ","
			<< m_sceneGraph.at(i).pan << ","
			<< m_sceneGraph.at(i).one_shot << ","
			<< m_sceneGraph.at(i).play_on_init << ","
			<< m_sceneGraph.at(i).play_in_editor << ","
			<< m_sceneGraph.at(i).min_dist << ","
			<< m_sceneGraph.at(i).max_dist << ","
			<< m_sceneGraph.at(i).camera << ","
			<< m_sceneGraph.at(i).path_node << ","
			<< m_sceneGraph.at(i).path_node_start << ","
			<< m_sceneGraph.at(i).path_node_end << ","
			<< m_sceneGraph.at(i).parent_id << ","
			<< m_sceneGraph.at(i).editor_wireframe << ","
			<< "'" << m_sceneGraph.at(i).name << "'" << ","

			<< m_sceneGraph.at(i).light_type << ","
			<< m_sceneGraph.at(i).light_diffuse_r << ","
			<< m_sceneGraph.at(i).light_diffuse_g << ","
			<< m_sceneGraph.at(i).light_diffuse_b << ","
			<< m_sceneGraph.at(i).light_specular_r << ","
			<< m_sceneGraph.at(i).light_specular_g << ","
			<< m_sceneGraph.at(i).light_specular_b << ","
			<< m_sceneGraph.at(i).light_spot_cutoff << ","
			<< m_sceneGraph.at(i).light_constant << ","
			<< m_sceneGraph.at(i).light_linear << ","
			<< m_sceneGraph.at(i).light_quadratic

			<< ")";
		std::string sqlCommand2 = command.str();
		rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand2.c_str(), -1, &pResults, 0);
		sqlite3_step(pResults);	
	}
	
	m_d3dRenderer.GetDisplayChunk()->SaveHeightMap(); // also save height map

	MessageBox(NULL, L"Objects and terrain saved", L"Notification", MB_OK);
}

void ToolMain::onActionSaveTerrain()
{
	m_d3dRenderer.SaveDisplayChunk(&m_chunk);
}

void ToolMain::onActionNewObject()
{
	// Add action and object to undo stack
	m_d3dRenderer.AddAction(Action::ADD);
	m_d3dRenderer.AddSceneObject();
	m_d3dRenderer.AddToObjectStack(m_sceneGraph.back());

	//m_d3dRenderer.FocusObject(m_selectedObject);
}

void ToolMain::onActionDelObject()
{
	// if an object is selected, add action and object to undo stacks and delete the object
	if (m_selectedObject != -1)
	{
		m_d3dRenderer.AddAction(Action::REMOVE);
		m_d3dRenderer.AddToObjectStack(m_sceneGraph.at(m_selectedObject));
		m_d3dRenderer.DeleteSceneObject(m_selectedObject);
	}
}

void ToolMain::onActionCopy()
{
	// if an object is selected, copy it
	if (m_selectedObject != -1)
	{
		m_haveCopiedObject = true;
		m_copiedObject = m_sceneGraph[m_selectedObject];
	}
}

void ToolMain::onActionPaste() 
{
	// If an object has been copied...
	if (m_haveCopiedObject)
	{
		// New ID
		m_d3dRenderer.m_topID++;

		// Create new scene object with copied properties and 
		SceneObject newSceneObject;
		newSceneObject = m_copiedObject;
		newSceneObject.ID = m_d3dRenderer.m_topID++;

		// Check for clashing positions
		m_d3dRenderer.PositionClashCheck(&newSceneObject);
		
		// Add to undo stacks
		m_d3dRenderer.AddAction(Action::ADD);
		m_d3dRenderer.AddToObjectStack(newSceneObject);

		//send completed object to scenegraph
		m_sceneGraph.push_back(newSceneObject);

		// rebuild display list and select the new object
		m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
		m_selectedObject = m_d3dRenderer.GetDisplayList()->size() - 1;
		m_d3dRenderer.GetManipulator()->SetObject(&m_d3dRenderer.GetDisplayList()->back());
	}
}


void ToolMain::Tick(MSG *msg)
{
	//do we have a selection
	//do we have a mode
	//are we clicking / dragging /releasing
	//has something changed
		//update Scenegraph
		//add to scenegraph
		//resend scenegraph to Direct X renderer

	
	

	//Renderer Update Call
	m_d3dRenderer.Tick(&m_toolInputCommands);

	// increment timers
	m_leftClickTimer += m_d3dRenderer.GetDeltaTime();
	m_actionCooldownTimer += m_d3dRenderer.GetDeltaTime();
}

void ToolMain::UpdateInput(MSG * msg)
{
	// Check if shift key is down, set relevant value in input commands
	bool isShiftDown = GetKeyState(VK_LSHIFT) < 0;

	if (isShiftDown)
	{
		m_toolInputCommands.shift = true;
	}
	else
	{
		m_toolInputCommands.shift = false;
	}

	// Check if ctrl key is down, set relevant value in input commands
	bool isCtrlDown = GetKeyState(VK_LCONTROL) < 0;

	if (isCtrlDown)
	{
		m_toolInputCommands.ctrl = true;
	}
	else
	{
		m_toolInputCommands.ctrl = false;
	}

	switch (msg->message)
	{
		//Global inputs,  mouse position and keys etc
	case WM_KEYDOWN:
		m_keyArray[msg->wParam] = true;

		// Cooldown used for delete action
		if (msg->wParam == VK_DELETE)
		{
			if (GetParent(GetActiveWindow()) == 0) // only applies when used on the main window, won't trigger on dialog edit box deletions
			{
				if (m_actionCooldownTimer > m_actionCooldown) // if enough time has passed, reset timer and delete object
				{
					m_actionCooldownTimer = 0;
					onActionDelObject();
				}
			}
		}

		break;

	case WM_KEYUP:
		m_keyArray[msg->wParam] = false;
		break;

	case WM_MOUSEMOVE:
		// Used for rotation since moving mouse with SetCursorPos uses different co-ordinate space from l params
		POINT point;
		GetCursorPos(&point);
		m_toolInputCommands.mouseX = point.x;
		m_toolInputCommands.mouseY = point.y;

		// different co-ordinates used for picking
		m_toolInputCommands.pickerX = GET_X_LPARAM(msg->lParam);
		m_toolInputCommands.pickerY = GET_Y_LPARAM(msg->lParam);
		break;

	case WM_LBUTTONDOWN:	//mouse button down,  you will probably need to check when its up too
		//set some flag for the mouse button in inputcommands
		m_toolInputCommands.LMBDown = true;
		m_leftClickTimer = 0; // reset left click timer
		break;

	case WM_RBUTTONDOWN:
		m_toolInputCommands.RMBDown = true;
		break;

	case WM_LBUTTONUP:
		m_toolInputCommands.LMBDown = false;

		// only do object picking on short clicks. long clicks will do object manipulation.
		if (m_leftClickTimer < 0.2f)
		{
			if (!m_d3dRenderer.GetSculptModeActive()) // only pick objects when not in sculpt mode
			{
				m_selectedObject = m_d3dRenderer.MousePicking(m_selectedObject);
			}
		}
		
		break;

	case WM_RBUTTONUP:
		m_toolInputCommands.RMBDown = false;
		break;

	case WM_MOUSEWHEEL:
		auto wheelDelta = GET_WHEEL_DELTA_WPARAM(msg->wParam);
		m_d3dRenderer.ScrollWheel(wheelDelta); // update scroll wheel input
		break;
		
		
	}
	
	// Hiding and showing the cursor when LMB or RMB are pressed within the renderer window.
	if (GetParent(GetActiveWindow()) == 0 && m_toolInputCommands.pickerY > m_d3dRenderer.GetToolbarHeight()) // if not clicking the main window, we can ignore the click
	{
		if ((GetKeyState(VK_LBUTTON) & 0x80) != 0 || (GetKeyState(VK_RBUTTON) & 0x80) != 0)
		{
			while (ShowCursor(false) >= 0);
		}
		else
		{
			while (ShowCursor(true) <= 0);
		}
	}
	else
	{
		while (ShowCursor(true) <= 0);
	}

	//here we update all the actual app functionality that we want.  This information will either be used int toolmain, or sent down to the renderer (Camera movement etc
	//WASD movement
	if (m_keyArray['W'])
	{
		m_toolInputCommands.forward = true;
	}
	else m_toolInputCommands.forward = false;
	
	if (m_keyArray['S'])
	{
		if (m_toolInputCommands.ctrl) // save instead when ctrl key is held
		{
			onActionSave();
			m_keyArray['S'] = false;
		}
		else
		{
			m_toolInputCommands.back = true;
		}
	}
	else m_toolInputCommands.back = false;

	if (m_keyArray['A'])
	{
		m_toolInputCommands.left = true;
	}
	else m_toolInputCommands.left = false;

	if (m_keyArray['D'])
	{
		m_toolInputCommands.right = true;
	}
	else m_toolInputCommands.right = false;

	//rotation
	/*
	if (m_keyArray[39]) // 39 = right arrow
	{
		m_toolInputCommands.rotRight = true;
	}
	else m_toolInputCommands.rotRight = false;
	if (m_keyArray[37]) // 37 = left arrow
	{
		m_toolInputCommands.rotLeft = true;
	}
	else m_toolInputCommands.rotLeft = false;
	if (m_keyArray[38]) // 38 = up arrow
	{
		m_toolInputCommands.rotUp = true;
	}
	else m_toolInputCommands.rotUp = false;
	if (m_keyArray[40]) // 40 = down arrow
	{
		m_toolInputCommands.rotDown = true;
	}
	else m_toolInputCommands.rotDown = false;
	*/

	//elevation
	if (m_keyArray['Q'])
	{
		m_toolInputCommands.down = true;
	}
	else m_toolInputCommands.down = false;

	if (m_keyArray['E'])
	{
		m_toolInputCommands.up = true;
	}
	else m_toolInputCommands.up = false;

	// focus object
	if (m_keyArray['F'])
	{
		m_d3dRenderer.FocusObject(m_selectedObject);
	}

	// copy object
	if (m_keyArray['C'] && m_toolInputCommands.ctrl)
	{
		onActionCopy();
	}

	// paste object
	if (m_keyArray['V'] && m_toolInputCommands.ctrl)
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			onActionPaste();
		}
	}

	// undo 
	if (m_keyArray['Z'] && m_toolInputCommands.ctrl)
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			GetGame()->Undo();
		}
	}

	// redo
	if (m_keyArray['Y'] && m_toolInputCommands.ctrl)
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			GetGame()->Redo();
		}
	}

	// new object
	if (m_keyArray['N'])
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			onActionNewObject();
		}
	}

	// toggle wireframe for objects
	if (m_keyArray['O'])
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			m_d3dRenderer.ToggleWireframeObjects();
		}
	}
	
	// toggle wireframe for landscape
	if (m_keyArray['L'])
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			m_d3dRenderer.ToggleWireframeTerrain();
		}
	}

	// Use first sculpt/control mode
	if (m_keyArray['1'])
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			if (m_d3dRenderer.GetSculptModeActive())
			{
				m_d3dRenderer.SetSculptMode(SculptMode::RAISE);
			}
			else
			{
				m_d3dRenderer.SetManipulationMode(ManipulationMode::TRANSLATE);
			}
			
		}
	}

	// Use second sculpt/control mode
	if (m_keyArray['2'])
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			if (m_d3dRenderer.GetSculptModeActive())
			{
				m_d3dRenderer.SetSculptMode(SculptMode::LOWER);
			}
			else
			{
				m_d3dRenderer.SetManipulationMode(ManipulationMode::ROTATE);
			}
		}
	}

	// Use third sculpt/control mode
	if (m_keyArray['3'])
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			if (m_d3dRenderer.GetSculptModeActive())
			{
				m_d3dRenderer.SetSculptMode(SculptMode::FLATTEN);
			}
			else
			{
				m_d3dRenderer.SetManipulationMode(ManipulationMode::SCALE);
			}
		}
	}

	// Switch between sculpting and object manipulation
	if (m_keyArray[9]) // 9 = tab
	{
		if (m_actionCooldownTimer > m_actionCooldown)
		{
			m_actionCooldownTimer = 0;
			m_d3dRenderer.SetSculptModeActive(!m_d3dRenderer.GetSculptModeActive()); // toggle sculpt mode
		}

	}

}
