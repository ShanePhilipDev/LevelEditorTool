#include "MFCMain.h"
#include "resource.h"
#include "ObjectManipulator.h"


BEGIN_MESSAGE_MAP(MFCMain, CWinApp)
	ON_COMMAND(ID_FILE_QUIT,	&MFCMain::MenuFileQuit)
	ON_COMMAND(ID_FILE_SAVETERRAIN, &MFCMain::MenuFileSaveTerrain)
	ON_COMMAND(ID_EDIT_SELECT, &MFCMain::MenuEditSelect)
	ON_COMMAND(ID_WINDOW_OBJECTDIALOG, &MFCMain::MenuWindowObject)
	ON_COMMAND(ID_BUTTON40001,	&MFCMain::ToolBarSave)
	ON_COMMAND(ID_BUTTON_TRANSLATE, &MFCMain::ToolBarTranslate)
	ON_COMMAND(ID_BUTTON_ROTATE, &MFCMain::ToolBarRotate)
	ON_COMMAND(ID_BUTTON_SCALE, &MFCMain::ToolBarScale)
	ON_COMMAND(ID_BUTTON_WIREFRAME, &MFCMain::ToolBarWireframeObjects)
	ON_COMMAND(ID_BUTTON_WIREFRAME_LANDSCAPE, &MFCMain::ToolBarWireframeLandscape)
	ON_COMMAND(ID_BUTTON_FOCUS, &MFCMain::ToolBarFocus)
	ON_COMMAND(ID_BUTTON_NEW_OBJECT, &MFCMain::ToolBarNewObject)
	ON_COMMAND(ID_BUTTON_DEL_OBJECT, &MFCMain::ToolBarDelObject)
	ON_COMMAND(ID_BUTTON_COPY, &MFCMain::ToolBarCopy)
	ON_COMMAND(ID_BUTTON_PASTE, &MFCMain::ToolBarPaste)
	ON_COMMAND(ID_BUTTON_SETTINGS, &MFCMain::ToolBarSettings)
	ON_COMMAND(ID_BUTTON_SELECT, &MFCMain::ToolBarObjectMode)
	ON_COMMAND(ID_BUTTON_SCULPT, &MFCMain::ToolBarSculptMode)
	ON_COMMAND(ID_BUTTON_RAISE, &MFCMain::ToolBarSculptRaise)
	ON_COMMAND(ID_BUTTON_CARVE, &MFCMain::ToolBarSculptLower)
	ON_COMMAND(ID_BUTTON_FLATTEN, &MFCMain::ToolBarSculptFlatten)
	ON_COMMAND(ID_BUTTON_RESET, &MFCMain::ToolBarReset)
	ON_COMMAND(ID_BUTTON_UNDO, &MFCMain::ToolBarUndo)
	ON_COMMAND(ID_BUTTON_REDO, &MFCMain::ToolBarRedo)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TOOL, &CMyFrame::OnUpdatePage)
END_MESSAGE_MAP()

BOOL MFCMain::InitInstance()
{
	//instanciate the mfc frame
	m_frame = new CMyFrame();
	m_pMainWnd = m_frame;

	m_frame->Create(	NULL,
					_T("World Of Corgcraft Editor"),
					WS_OVERLAPPEDWINDOW,
					CRect(100, 100, 1024, 768),
					NULL,
					NULL,
					0,
					NULL
				);

	

	//show and set the window to run and update. 
	m_frame->ShowWindow(SW_SHOW);
	m_frame->UpdateWindow();


	//get the rect from the MFC window so we can get its dimensions
	m_toolHandle = m_frame->m_DirXView.GetSafeHwnd();				//handle of directX child window
	m_frame->m_DirXView.GetClientRect(&WindowRECT);
	m_width		= WindowRECT.Width();
	m_height	= WindowRECT.Height();

	m_ToolSystem.onActionInitialise(m_toolHandle, m_width, m_height);

	// Initialise object dialog window
	m_ToolObjectDialog.Create(IDD_DIALOG_OBJECT);
	m_ToolObjectDialog.SetGameRef(m_ToolSystem.GetGame());
	m_ToolObjectDialog.SetWindowPos(m_pMainWnd, 1024, 100, m_ToolObjectDialog.GetRect().Width(), m_ToolObjectDialog.GetRect().Height(), NULL); // position dialogue next to main window
	m_ToolObjectDialog.ShowWindow(SW_SHOW);
	m_ToolObjectDialog.SetObjectData(&m_ToolSystem.m_sceneGraph, &m_ToolSystem.m_selectedObject);

	// Initialise settings dialog window
	m_ToolSettingsDialog.Create(IDD_DIALOG_SETTINGS);
	m_ToolSettingsDialog.SetGameRef(m_ToolSystem.GetGame());

	// Create select dialog window
	m_ToolSelectDialogue.Create(IDD_DIALOG1);	//Start up modeless

	return TRUE;
}

int MFCMain::Run()
{
	MSG msg;
	BOOL bGotMsg;

	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		if (true)
		{
			bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		}
		else
		{
			bGotMsg = (GetMessage(&msg, NULL, 0U, 0U) != 0);
		}

		if (bGotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			m_ToolSystem.UpdateInput(&msg); // pass input message to tool main class
		}
		else
		{	
			int ID = m_ToolSystem.getCurrentSelectionID();
			std::wstring statusString;
			//statusString = L"Selected Object: " + std::to_wstring(ID); // not needed anymore, object highlighting and object dialog box indicate this
			CToolBar* tb = &m_frame->m_toolBar;

			// Disable undo button if not possible to undo
			if (m_ToolSystem.GetGame()->GetUndoStack().empty())
			{
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_UNDO), TBBS_DISABLED);
			}
			else
			{
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_UNDO), TBBS_BUTTON);
			}
			
			// Disable redo button if not possible to redo
			if (m_ToolSystem.GetGame()->GetRedoStack().empty())
			{
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_REDO), TBBS_DISABLED);
			}
			else
			{
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_REDO), TBBS_BUTTON);
			}

			// When in sculpt mode...
			if (m_ToolSystem.GetGame()->GetSculptModeActive())
			{
				// Show sculpt button as pressed.
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_SCULPT), TBBS_PRESSED);
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_SELECT), TBBS_BUTTON);

				// Show current sculpt mode in status bar string.
				statusString = L"Sculpt Mode: ";
				switch (m_ToolSystem.GetGame()->GetSculptMode())
				{
				case SculptMode::RAISE:
					statusString += L"RAISE";
					break;
				case SculptMode::LOWER:
					statusString += L"LOWER";
					break;
				case SculptMode::FLATTEN:
					statusString += L"FLATTEN";
					break;
				}
			}
			else // When in object manipulation mode...
			{
				// Show object manipulator button as pressed.
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_SCULPT), TBBS_BUTTON);
				tb->SetButtonStyle(tb->CommandToIndex(ID_BUTTON_SELECT), TBBS_PRESSED);

				// Show current manipulation mode in status bar string.
				statusString = L"Control Mode: ";
				switch (m_ToolSystem.GetGame()->GetManipulationMode())
				{
				case ManipulationMode::TRANSLATE:
					statusString += L"TRANSLATE";
					break;
				case ManipulationMode::ROTATE:
					statusString += L"ROTATE";
					break;
				case ManipulationMode::SCALE:
					statusString += L"SCALE";
					break;
				}
			}
			
			
			m_ToolSystem.Tick(&msg);
			m_ToolObjectDialog.Update();

			// Update status bar string
			m_frame->m_wndStatusBar.SetPaneText(1, statusString.c_str(), 1);	
			
		}
	}

	return (int)msg.wParam;
}

// Quit
void MFCMain::MenuFileQuit()
{
	//will post message to the message thread that will exit the application normally
	PostQuitMessage(0);
}

// Save terrain
void MFCMain::MenuFileSaveTerrain()
{
	m_ToolSystem.onActionSaveTerrain();
}

// Open select dialog
void MFCMain::MenuEditSelect()
{
	//SelectDialogue m_ToolSelectDialogue(NULL, &m_ToolSystem.m_sceneGraph);		//create our dialoguebox //modal constructor
	//m_ToolSelectDialogue.DoModal();	// start it up modal

	//modeless dialogue must be declared in the class.   If we do local it will go out of scope instantly and destroy itself
	
	m_ToolSelectDialogue.ShowWindow(SW_SHOW);	//show modeless
	m_ToolSelectDialogue.ClearList();
	m_ToolSelectDialogue.SetObjectData(&m_ToolSystem.m_sceneGraph, &m_ToolSystem.m_selectedObject);
}

// Open object dialog
void MFCMain::MenuWindowObject()
{
	// Show dialog upon button press rather than re-creating it.
	m_ToolObjectDialog.ShowWindow(SW_SHOW);
	m_ToolObjectDialog.SetObjectData(&m_ToolSystem.m_sceneGraph, &m_ToolSystem.m_selectedObject);
}

// Save objects
void MFCMain::ToolBarSave()
{
	m_ToolSystem.onActionSave();
}

// Set manipulation mode
// Translate
void MFCMain::ToolBarTranslate()
{
	m_ToolSystem.GetGame()->SetManipulationMode(ManipulationMode::TRANSLATE);
}

// Rotate
void MFCMain::ToolBarRotate()
{
	m_ToolSystem.GetGame()->SetManipulationMode(ManipulationMode::ROTATE);
}

// Scale
void MFCMain::ToolBarScale()
{
	m_ToolSystem.GetGame()->SetManipulationMode(ManipulationMode::SCALE);
}

// Toggle object wireframes
void MFCMain::ToolBarWireframeObjects()
{
	m_ToolSystem.GetGame()->ToggleWireframeObjects();
}

// Toggle landscape wireframe
void MFCMain::ToolBarWireframeLandscape()
{
	m_ToolSystem.GetGame()->ToggleWireframeTerrain();
}

// Focus on selected object
void MFCMain::ToolBarFocus()
{
	m_ToolSystem.GetGame()->FocusObject(m_ToolSystem.m_selectedObject);
}

// Create new object
void MFCMain::ToolBarNewObject()
{
	m_ToolSystem.onActionNewObject();
}

// Delete object
void MFCMain::ToolBarDelObject()
{
	m_ToolSystem.onActionDelObject();	
}

// Copy object
void MFCMain::ToolBarCopy()
{
	m_ToolSystem.onActionCopy();
}

// Paste object
void MFCMain::ToolBarPaste()
{
	m_ToolSystem.onActionPaste();
}

// Show settings dialog
void MFCMain::ToolBarSettings()
{
	m_ToolSettingsDialog.ShowWindow(SW_SHOW);
}

// Activate object manipulation mode
void MFCMain::ToolBarObjectMode()
{
	m_ToolSystem.GetGame()->SetSculptModeActive(false);
}

// Activate terrain sculpting mode
void MFCMain::ToolBarSculptMode()
{
	m_ToolSystem.GetGame()->SetSculptModeActive(true);
}

// Set sculpt mode
// Raise
void MFCMain::ToolBarSculptRaise()
{
	m_ToolSystem.GetGame()->SetSculptMode(SculptMode::RAISE);
}

// Lower
void MFCMain::ToolBarSculptLower()
{
	m_ToolSystem.GetGame()->SetSculptMode(SculptMode::LOWER);
}

// Flatten
void MFCMain::ToolBarSculptFlatten()
{
	m_ToolSystem.GetGame()->SetSculptMode(SculptMode::FLATTEN);
}

// Reload everything
void MFCMain::ToolBarReset()
{
	//m_ToolSystem.GetGame()->BuildDisplayChunk(&m_ToolSystem.m_chunk);
	m_ToolSystem.onActionLoad();
	m_ToolSystem.GetGame()->ClearUndoRedo();
}

// Undo
void MFCMain::ToolBarUndo()
{
	m_ToolSystem.GetGame()->Undo();
	m_ToolObjectDialog.UpdateFromObject();
}

// Redo
void MFCMain::ToolBarRedo()
{
	m_ToolSystem.GetGame()->Redo();
	m_ToolObjectDialog.UpdateFromObject();
}


MFCMain::MFCMain()
{
	
}


MFCMain::~MFCMain()
{
}
