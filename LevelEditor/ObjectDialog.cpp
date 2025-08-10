
#include "ObjectDialog.h"
#include "atlstr.h"
#include <atlimage.h>

IMPLEMENT_DYNAMIC(ObjectDialog, CDialogEx)

BEGIN_MESSAGE_MAP(ObjectDialog, CDialogEx)
	ON_COMMAND(IDOK, &ObjectDialog::End)
	ON_COMMAND(IDC_BUTTON_APPLY, &ObjectDialog::UpdateFromEditBoxes)
	ON_COMMAND(IDC_CHECK_VISIBILITY, &ObjectDialog::CheckboxVisibility)
	ON_COMMAND(IDC_CHECK_SNAP, &ObjectDialog::CheckboxSnapToGround)
END_MESSAGE_MAP()

ObjectDialog::ObjectDialog(CWnd* pParent, std::vector<SceneObject>* SceneGraph) : CDialogEx(IDD_DIALOG_OBJECT, pParent)
{
	// construct with scene graph, set values
	m_sceneGraph = SceneGraph;
	m_previousSelection = 1;
}

ObjectDialog::ObjectDialog(CWnd* pParent) : CDialogEx(IDD_DIALOG_OBJECT, pParent)
{
	// construct without scene graph, set values
	m_previousSelection = 1;
}

ObjectDialog::~ObjectDialog()
{
}

void ObjectDialog::SetObjectData(std::vector<SceneObject>* SceneGraph, int* Selection)
{
	// Set pointers needed for getting object
	m_sceneGraph = SceneGraph;
	m_currentSelection = Selection;

	// Update window values
	UpdateFromObject();
}

CRect ObjectDialog::GetRect()
{
	// Get window dimensions and return it
	CRect DialogWindowRect;
	GetWindowRect(DialogWindowRect);
	return DialogWindowRect;
}

void ObjectDialog::Update()
{
	// Check if edit boxes have flag set to update the window. If they have, update from the edit box values and reset flag.
	for (int i = 0; i < m_editBoxes.size(); i++)
	{
		if (m_editBoxes[i]->updateFlag)
		{
			UpdateFromEditBoxes();
			m_editBoxes[i]->updateFlag = false;
		}
	}

	// If selection has changed...
	if (*m_currentSelection != m_previousSelection)
	{
		if (*m_currentSelection == -1) // clear if no object is selected
		{
			ClearObject();
		}
		else // otherwise update window with new object's values
		{
			UpdateFromObject();
		}

		// Set new previous selection
		m_previousSelection = *m_currentSelection;
	}
	else
	{
		// When the user is manipulating an object, update values while they change the object
		if (m_gameRef->GetManipulator()->GetActive())
		{
			UpdateFromObject();
		}
	}

	// Snapping to ground sets edit box for changing height to be read only
	if (*m_currentSelection != -1)
	{
		if (m_sceneGraph->at(*m_currentSelection).snapToGround)
		{
			m_posY.SetReadOnly(TRUE);
		}
		else
		{
			m_posY.SetReadOnly(FALSE);
		}
	}
	else
	{
		m_posY.SetReadOnly(FALSE);
	}

	// Get selection from combo boxes
	int texSelection = m_texturePath.GetCurSel();
	int modSelection = m_modelPath.GetCurSel();

	// Texture preview
	if (texSelection == -1) // if nothing is selected in combo box, use no preview image
	{
		m_texturePicture.SetBitmap(m_noPreview);
	}
	else
	{
		// Get path from combo box
		CString path;
		m_texturePath.GetLBText(texSelection, path);

		// If the path matches a texture, use that texture's image. Otherwise use no preview.
		if (m_texturePreviews[path])
		{
			m_texturePicture.SetBitmap(m_texturePreviews[path]);
		}
		else
		{
			m_texturePicture.SetBitmap(m_noPreview);
		}
		
	}
	
	// Model preview
	if (modSelection == -1)
	{
		m_modelPicture.SetBitmap(m_noPreview);  // if nothing is selected in combo box, use no preview image
	}
	else
	{
		// Get path from combo box
		CString path;
		m_modelPath.GetLBText(modSelection, path);

		// If the path matches a model, use that model's image. Otherwise use no preview.
		if (m_modelPreviews[path])
		{
			m_modelPicture.SetBitmap(m_modelPreviews[path]);
		}
		else
		{
			m_modelPicture.SetBitmap(m_noPreview);
		}
	}
}



void ObjectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	// Exchange data between dialog object and variables
	DDX_Control(pDX, IDC_STATIC_SELECTED, m_objectIDText);
	DDX_Control(pDX, IDC_EDIT_POS_X, m_posX);
	DDX_Control(pDX, IDC_EDIT_POS_Y, m_posY);
	DDX_Control(pDX, IDC_EDIT_POS_Z, m_posZ);
	DDX_Control(pDX, IDC_EDIT_ROT_X, m_rotX);
	DDX_Control(pDX, IDC_EDIT_ROT_Y, m_rotY);
	DDX_Control(pDX, IDC_EDIT_ROT_Z, m_rotZ);
	DDX_Control(pDX, IDC_EDIT_SCALE_X, m_scaleX);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y, m_scaleY);
	DDX_Control(pDX, IDC_EDIT_SCALE_Z, m_scaleZ);
	DDX_Control(pDX, IDC_COMBO_MODEL, m_modelPath);
	DDX_Control(pDX, IDC_COMBO_TEXTURE, m_texturePath);
	DDX_Control(pDX, IDC_STATIC_MODEL, m_modelPicture);
	DDX_Control(pDX, IDC_STATIC_TEXTURE, m_texturePicture);
}

void ObjectDialog::End()
{
	DestroyWindow(); // destroy window
}

void ObjectDialog::CheckboxVisibility()
{
	// Occurs when visibility checkbox is pressed.
	
	// If an object is selected...
	if (*m_currentSelection != -1)
	{
		// Add to undo stacks
		m_gameRef->AddAction(Action::MODIFY);
		m_gameRef->AddToObjectStack(m_sceneGraph->at(*m_currentSelection));

		// Set object render state based on new value of checkbox
		if (IsDlgButtonChecked(IDC_CHECK_VISIBILITY) == BST_CHECKED)
		{
			m_sceneGraph->at(*m_currentSelection).editor_render = true;
		}
		else
		{
			m_sceneGraph->at(*m_currentSelection).editor_render = false;
		}

		// Rebuild display list to reflect change in visibility
		if (m_gameRef)
		{
			m_gameRef->BuildDisplayList(m_sceneGraph);
		}
	}
}

void ObjectDialog::CheckboxSnapToGround()
{
	// Occurs when snap to ground checkbox is pressed.

	// If an object is selected...
	if (*m_currentSelection != -1)
	{
		// Add to undo stacks
		m_gameRef->AddAction(Action::MODIFY);
		m_gameRef->AddToObjectStack(m_sceneGraph->at(*m_currentSelection));

		// Set object snap to ground based on new value of checkbox
		if (IsDlgButtonChecked(IDC_CHECK_SNAP) == BST_CHECKED)
		{
			m_sceneGraph->at(*m_currentSelection).snapToGround = true;
		}
		else
		{
			m_sceneGraph->at(*m_currentSelection).snapToGround = false;
		}

		// Rebuild display list to reflect change
		if (m_gameRef)
		{
			m_gameRef->BuildDisplayList(m_sceneGraph);
			
		}
	}
}

void ObjectDialog::UpdateFromObject()
{
	// If an object is selected...
	if (*m_currentSelection != -1)
	{
		// Get object
		SceneObject* Object = &m_sceneGraph->at(*m_currentSelection);

		// Set window's ID string
		std::wstring IDString = std::to_wstring(Object->ID);
		m_objectIDText.SetWindowTextW(IDString.c_str());

		// Set window's position box values
		std::wstring PosX, PosY, PosZ;
		PosX = std::to_wstring(Object->posX);
		PosY = std::to_wstring(Object->posY);
		PosZ = std::to_wstring(Object->posZ);

		m_posX.SetWindowTextW(PosX.c_str());
		m_posY.SetWindowTextW(PosY.c_str());
		m_posZ.SetWindowTextW(PosZ.c_str());

		// Set window's rotation box values
		std::wstring RotX, RotY, RotZ;
		RotX = std::to_wstring(Object->rotX);
		RotY = std::to_wstring(Object->rotY);
		RotZ = std::to_wstring(Object->rotZ);

		m_rotX.SetWindowTextW(RotX.c_str());
		m_rotY.SetWindowTextW(RotY.c_str());
		m_rotZ.SetWindowTextW(RotZ.c_str());

		// Set window's scale box values
		std::wstring ScaleX, ScaleY, ScaleZ;
		ScaleX = std::to_wstring(Object->scaX);
		ScaleY = std::to_wstring(Object->scaY);
		ScaleZ = std::to_wstring(Object->scaZ);

		m_scaleX.SetWindowTextW(ScaleX.c_str());
		m_scaleY.SetWindowTextW(ScaleY.c_str());
		m_scaleZ.SetWindowTextW(ScaleZ.c_str());

		// Set visibility checkbox state
		if (Object->editor_render)
		{
			CheckDlgButton(IDC_CHECK_VISIBILITY, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(IDC_CHECK_VISIBILITY, BST_UNCHECKED);
		}

		// Set snap to ground checkbox state
		if (Object->snapToGround)
		{
			CheckDlgButton(IDC_CHECK_SNAP, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(IDC_CHECK_SNAP, BST_UNCHECKED);
		}

		// Set model and texture combo boxes to correct paths
		std::wstring modelPath = std::wstring(Object->model_path.begin(), Object->model_path.end());
		std::wstring texturePath = std::wstring(Object->tex_diffuse_path.begin(), Object->tex_diffuse_path.end());

		int modelIndex = m_modelPath.FindString(0, modelPath.c_str());
		int textureIndex = m_texturePath.FindString(0, texturePath.c_str());

		m_modelPath.SetCurSel(modelIndex);
		m_texturePath.SetCurSel(textureIndex);
	}
}

void ObjectDialog::UpdateFromEditBoxes()
{
	// If an object is selected...
	if (*m_currentSelection != -1)
	{
		// Add to undo stacks.
		m_gameRef->AddAction(Action::MODIFY);
		m_gameRef->AddToObjectStack(m_sceneGraph->at(*m_currentSelection));

		// Get object to edit.
		SceneObject* Object = &m_sceneGraph->at(*m_currentSelection);

		// Temp string for holding values.
		CString temp; 

		// Get position string, convert to float, then apply to object.
		m_posX.GetWindowText(temp);
		Object->posX = _ttof(temp);
		m_posY.GetWindowText(temp);
		Object->posY = _ttof(temp);
		m_posZ.GetWindowText(temp);
		Object->posZ = _ttof(temp);

		// Get rotation string, convert to float, then apply to object.
		m_rotX.GetWindowText(temp);
		Object->rotX = _ttof(temp);
		m_rotY.GetWindowText(temp);
		Object->rotY = _ttof(temp);
		m_rotZ.GetWindowText(temp);
		Object->rotZ = _ttof(temp);

		// Get scale string, convert to float, then apply to object.
		m_scaleX.GetWindowText(temp);
		Object->scaX = _ttof(temp);
		m_scaleY.GetWindowText(temp);
		Object->scaY = _ttof(temp);
		m_scaleZ.GetWindowText(temp);
		Object->scaZ = _ttof(temp);

		// Apply changes to manipulator's display object to avoid having to rebuild the display list
		m_gameRef->GetManipulator()->SetObjectPosition(Object->posX, Object->posY, Object->posZ);
		m_gameRef->GetManipulator()->SetObjectRotation(Object->rotX, Object->rotY, Object->rotZ);
		m_gameRef->GetManipulator()->SetObjectScale(Object->scaX, Object->scaY, Object->scaZ);
		
		// If model changes, display list need to be rebuilt
		bool rebuildDisplayList = false;
		std::wstring modelPath = std::wstring(Object->model_path.begin(), Object->model_path.end());
		std::wstring texturePath = std::wstring(Object->tex_diffuse_path.begin(), Object->tex_diffuse_path.end());
		
		// If texture/model has changed, rebuild display list with new model/texture
		CString boxPath;
		m_modelPath.GetLBText(m_modelPath.GetCurSel(), boxPath);
		if (modelPath != boxPath.GetString())
		{
			rebuildDisplayList = true;
			Object->model_path = CW2A(boxPath.GetString());
		}

		m_texturePath.GetLBText(m_texturePath.GetCurSel(), boxPath);
		if (texturePath != boxPath.GetString())
		{
			rebuildDisplayList = true;
			Object->tex_diffuse_path = CW2A(boxPath.GetString());
		}

		if (rebuildDisplayList)
		{
			m_gameRef->BuildDisplayList(m_sceneGraph);
		}

		// Update window from object to re-format floats in edit box
		UpdateFromObject();
	}
}

void ObjectDialog::ClearObject()
{
	// Clear ID
	m_objectIDText.SetWindowTextW(L"NONE");

	// Clear positions
	m_posX.SetWindowTextW(L"");
	m_posY.SetWindowTextW(L"");
	m_posZ.SetWindowTextW(L"");

	// Clear rotations
	m_rotX.SetWindowTextW(L"");
	m_rotY.SetWindowTextW(L"");
	m_rotZ.SetWindowTextW(L"");

	// Clear scale
	m_scaleX.SetWindowTextW(L"");
	m_scaleY.SetWindowTextW(L"");
	m_scaleZ.SetWindowTextW(L"");

	// Uncheck checkboxes
	CheckDlgButton(IDC_CHECK_VISIBILITY, BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_SNAP, BST_UNCHECKED);

	// Select nothing in combo box
	m_modelPath.SetCurSel(-1);
	m_texturePath.SetCurSel(-1);
}

BOOL ObjectDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Initialise picture size
	m_pictureSize = 112;

	// List of model names and texture names
	std::vector<CString> modelNames{ L"bedroll", L"campfire", L"corgi", L"doghouse", L"crate", L"placeholder", L"pug", L"thrall" };
	std::vector<CString> textureNames{ L"bedroll", L"campfire", L"corgi", L"doghouse", L"crate", L"placeholder", L"pug", L"thrall", L"blank"};

	// For each model...
	for (int i = 0; i < modelNames.size(); i++)
	{
		// Get model path and preview image path
		CString path = L"database/data/" + modelNames[i] + L".cmo";
		CString previewPath = L"database/data/" + modelNames[i] + L"_preview.bmp";

		// Get image from preview path
		HBITMAP image = (HBITMAP)LoadImage(NULL, previewPath, IMAGE_BITMAP, m_pictureSize, m_pictureSize, LR_LOADFROMFILE);

		// Add to combo box
		m_modelPath.AddString(path);

		// If image exists, add to model preview list with corresponding path
		if (image)
		{
			m_modelPreviews.emplace(path, image);
		}
	}

	m_modelPath.SetCurSel(-1);

	// For each texture...
	for (int i = 0; i < textureNames.size(); i++)
	{
		// Get texture path and preview image path
		CString path = L"database/data/" + textureNames[i] + L".dds";
		CString previewPath = L"database/data/" + textureNames[i] + L"_tex.bmp";

		// Get image from preview path
		HBITMAP image = (HBITMAP)LoadImage(NULL, previewPath, IMAGE_BITMAP, m_pictureSize, m_pictureSize, LR_LOADFROMFILE);

		// Add to combo box
		m_texturePath.AddString(path);

		// If image exists, add to texture preview list with corresponding path
		if (image)
		{
			m_texturePreviews.emplace(path, image);
		}
	}

	m_texturePath.SetCurSel(-1);
	
	// Set no preview image
	m_noPreview = (HBITMAP)LoadImage(NULL, L"database/data/no_preview.bmp", IMAGE_BITMAP, m_pictureSize, m_pictureSize, LR_LOADFROMFILE);
	m_texturePicture.SetBitmap(m_noPreview);
	m_modelPicture.SetBitmap(m_noPreview);

	// Add edit boxes to vector for iteration
	m_editBoxes.push_back(&m_posX);
	m_editBoxes.push_back(&m_posY);
	m_editBoxes.push_back(&m_posZ);
	m_editBoxes.push_back(&m_rotX);
	m_editBoxes.push_back(&m_rotY);
	m_editBoxes.push_back(&m_rotZ);
	m_editBoxes.push_back(&m_scaleX);
	m_editBoxes.push_back(&m_scaleY);
	m_editBoxes.push_back(&m_scaleZ);

	return TRUE;
}

void ObjectDialog::PostNcDestroy()
{
}
