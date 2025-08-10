#pragma once

#include <afxwin.h> 
#include <afxext.h>
#include <afx.h>
#include "pch.h"
#include "Game.h"
#include "ToolMain.h"
#include "resource.h"
#include "MFCFrame.h"
#include "SelectDialogue.h"
#include "ObjectDialog.h"
#include "SettingsDialog.h"


class MFCMain : public CWinApp 
{
public:
	MFCMain();
	~MFCMain();
	BOOL InitInstance();
	int  Run();

private:

	CMyFrame * m_frame;	//handle to the frame where all our UI is
	HWND m_toolHandle;	//Handle to the MFC window
	ToolMain m_ToolSystem;	//Instance of Tool System that we interface to. 
	CRect WindowRECT;	//Window area rectangle. 
	SelectDialogue m_ToolSelectDialogue;			//for modeless dialogue, declare it here
	ObjectDialog m_ToolObjectDialog; // object dialog window
	SettingsDialog m_ToolSettingsDialog; // settings dialog window

	int m_width;		
	int m_height;

	//Interface funtions for menu and toolbar
	afx_msg void MenuFileQuit();
	afx_msg void MenuFileSaveTerrain();
	afx_msg void MenuEditSelect();
	afx_msg void MenuWindowObject();
	afx_msg	void ToolBarSave();
	afx_msg void ToolBarTranslate();
	afx_msg void ToolBarRotate();
	afx_msg void ToolBarScale();
	afx_msg void ToolBarWireframeObjects();
	afx_msg void ToolBarWireframeLandscape();
	afx_msg void ToolBarFocus();
	afx_msg void ToolBarNewObject();
	afx_msg void ToolBarDelObject();
	afx_msg void ToolBarCopy();
	afx_msg void ToolBarPaste();
	afx_msg void ToolBarSettings();
	afx_msg void ToolBarObjectMode();
	afx_msg void ToolBarSculptMode();
	afx_msg void ToolBarSculptRaise();
	afx_msg void ToolBarSculptLower();
	afx_msg void ToolBarSculptFlatten();
	afx_msg void ToolBarReset();
	afx_msg void ToolBarUndo();
	afx_msg void ToolBarRedo();

	DECLARE_MESSAGE_MAP()	// required macro for message map functionality  One per class
};
