#pragma once
#include <afxdialogex.h>
#include "afxwin.h"
#include "resource.h"
#include "SceneObject.h"
#include "ToolMain.h"
#include "CustomCEdit.h"
#include <vector>
#include <map>

class Game; //forward declaration, all my homies hate circular dependencies

class ObjectDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ObjectDialog)

public:
	ObjectDialog(CWnd* pParent, std::vector<SceneObject>* SceneGraph);
	ObjectDialog(CWnd* pParent = NULL);
	virtual ~ObjectDialog();

	// Set pointers
	void SetObjectData(std::vector<SceneObject>* SceneGraph, int* Selection);
	void SetGameRef(Game* game) { m_gameRef = game; };

	// Get window dimensions
	CRect GetRect();

	// Main update function
	void Update();

	// Update window values from selected object
	void UpdateFromObject();

	// Apply changes from edit boxes to selected object
	void UpdateFromEditBoxes();

	// Clear all window values
	void ClearObject();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_OBJECT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void End();		//kill the dialogue

	// Functions called when checkboxes are modified
	afx_msg void CheckboxVisibility();
	afx_msg void CheckboxSnapToGround();

	// Pointers needed for modifying object properties
	Game* m_gameRef;
	std::vector<SceneObject>* m_sceneGraph;
	int* m_currentSelection;

	// Previous selected object number
	int m_previousSelection;
	
	DECLARE_MESSAGE_MAP();

	// ID text
	CStatic m_objectIDText;

	// Edit boxes, formatted to only accept floats
	CustomCEdit m_posX, m_posY, m_posZ;
	CustomCEdit m_rotX, m_rotY, m_rotZ;
	CustomCEdit m_scaleX, m_scaleY, m_scaleZ;
	std::vector<CustomCEdit*> m_editBoxes; // for easy iteration

	// Combo boxes for choosing models and textures
	CComboBox m_modelPath, m_texturePath;

	// Picture variables
	CStatic m_modelPicture, m_texturePicture; // object that holds picture
	int m_pictureSize; // size, pictures are square so just one value
	HBITMAP m_noPreview; // bitmap used when no matching image is found
	std::map<CString, HBITMAP> m_texturePreviews; // map of texture paths to bitmaps
	std::map<CString, HBITMAP> m_modelPreviews; // map of model paths to bitmaps

public:
	// Init and destroy
	virtual BOOL OnInitDialog() override;
	virtual void PostNcDestroy();
};

INT_PTR CALLBACK SelectProc(HWND   hwndDlg, UINT   uMsg, WPARAM wParam, LPARAM lParam);