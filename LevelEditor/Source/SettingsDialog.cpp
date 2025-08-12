#include "SettingsDialog.h"

IMPLEMENT_DYNAMIC(SettingsDialog, CDialogEx)

// 2 buttons in message map - close and apply
BEGIN_MESSAGE_MAP(SettingsDialog, CDialogEx)
	ON_COMMAND(IDOK, &SettingsDialog::End)
	ON_COMMAND(IDC_BUTTON_APPLY, &SettingsDialog::Apply)
END_MESSAGE_MAP()

SettingsDialog::SettingsDialog(CWnd* pParent) : CDialogEx(IDD_DIALOG_SETTINGS, pParent)
{
	// Default values
	m_camMultiplier = 2;
	m_objMultiplier = 10;
	m_sculptMultiplier = 10;
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::SetGameRef(Game* game)
{
	// Set game ref
	m_gameRef = game;

	// Set highlight checkbox state based on highlight boolean
	if (m_gameRef->GetHighlight())
	{
		CheckDlgButton(IDC_CHECK_HIGHLIGHT, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(IDC_CHECK_HIGHLIGHT, BST_UNCHECKED);
	}

	// Set heightmap checkbox state based on heightmap boolean
	if (m_gameRef->GetSculpter()->m_editHeightMap)
	{
		CheckDlgButton(IDC_CHECK_HEIGHTMAP, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(IDC_CHECK_HEIGHTMAP, BST_UNCHECKED);
	}

	// Set camera slider positions
	m_camMoveSpeed.SetPos(m_gameRef->GetCamera()->GetMoveSpeed());
	m_camRotSpeed.SetPos(m_gameRef->GetCamera()->GetRotSpeed());
	m_focusZoomSpeed.SetPos(m_gameRef->GetZoomSpeed());
	m_focusLerpSpeed.SetPos(m_gameRef->GetCamera()->GetLerpSpeed());

	// Set object slider positions
	m_objMoveSpeed.SetPos(m_gameRef->GetManipulator()->GetMoveSpeed() * m_objMultiplier);
	m_objRotSpeed.SetPos(m_gameRef->GetManipulator()->GetRotSpeed() * m_objMultiplier);
	m_objScaleSpeed.SetPos(m_gameRef->GetManipulator()->GetScaleSpeed() * m_objMultiplier);

	// Set sculpting slider positions
	m_sculptRadius.SetPos(m_gameRef->GetSculpter()->GetRadius() * m_sculptMultiplier);
	m_sculptMagnitude.SetPos(m_gameRef->GetSculpter()->GetMagnitude() * m_sculptMultiplier);
}

void SettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	// Exchange data between dialog object and variables
	DDX_Control(pDX, IDC_SLIDER_CAMSPEED, m_camMoveSpeed);
	DDX_Control(pDX, IDC_SLIDER_CAMROT, m_camRotSpeed);
	DDX_Control(pDX, IDC_SLIDER_FOCUSZOOM, m_focusZoomSpeed);
	DDX_Control(pDX, IDC_SLIDER_FOCUSLERP, m_focusLerpSpeed);
	DDX_Control(pDX, IDC_SLIDER_OBJSPEED, m_objMoveSpeed);
	DDX_Control(pDX, IDC_SLIDER_OBJROT, m_objRotSpeed);
	DDX_Control(pDX, IDC_SLIDER_OBJSCALE, m_objScaleSpeed);
	DDX_Control(pDX, IDC_SLIDER_RADIUS, m_sculptRadius);
	DDX_Control(pDX, IDC_SLIDER_MAGNITUDE, m_sculptMagnitude);
}

void SettingsDialog::End()
{
	ShowWindow(SW_HIDE); // hide window on close instead of destroying it
}

void SettingsDialog::Apply()
{
	// Applies all changes

	// Set highlight mode based on checkbox state
	if (IsDlgButtonChecked(IDC_CHECK_HIGHLIGHT) == BST_CHECKED)
	{
		m_gameRef->SetHighlight(true);
	}
	else
	{
		m_gameRef->SetHighlight(false);
	}

	// Set heightmap mode based on checkbox state
	if (IsDlgButtonChecked(IDC_CHECK_HEIGHTMAP) == BST_CHECKED)
	{
		m_gameRef->GetSculpter()->m_editHeightMap = true;
	}
	else
	{
		m_gameRef->GetSculpter()->m_editHeightMap = false;
	}

	// Apply camera settings
	m_gameRef->GetCamera()->SetMoveSpeed(m_camMoveSpeed.GetPos());
	m_gameRef->GetCamera()->SetRotSpeed((float)m_camRotSpeed.GetPos() / m_camMultiplier);
	m_gameRef->GetCamera()->SetLerpSpeed(m_focusLerpSpeed.GetPos());
	m_gameRef->SetZoomSpeed(m_focusZoomSpeed.GetPos());

	// Apply object manipulator settings
	m_gameRef->GetManipulator()->SetMoveSpeed((float)m_objMoveSpeed.GetPos() / m_objMultiplier);
	m_gameRef->GetManipulator()->SetRotSpeed((float)m_objRotSpeed.GetPos() / m_objMultiplier);
	m_gameRef->GetManipulator()->SetScaleSpeed((float)m_objScaleSpeed.GetPos() / m_objMultiplier);

	// Apply sculpter settings
	m_gameRef->GetSculpter()->SetRadius((float)m_sculptRadius.GetPos() / m_sculptMultiplier);
	m_gameRef->GetSculpter()->SetMagnitude((float)m_sculptMagnitude.GetPos() / m_sculptMultiplier);
	
	// Hide window
	ShowWindow(SW_HIDE);
}

BOOL SettingsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialise camera setting ranges
	m_camMoveSpeed.SetRange(1, 50);
	m_camRotSpeed.SetRange(1, 40);
	m_focusZoomSpeed.SetRange(10, 50);
	m_focusLerpSpeed.SetRange(1, 20);

	// Initialise object setting ranges
	m_objMoveSpeed.SetRange(1, 50);
	m_objRotSpeed.SetRange(1, 200);
	m_objScaleSpeed.SetRange(1, 20);

	// Initialise sculpt setting ranges
	m_sculptRadius.SetRange(20, 200);
	m_sculptMagnitude.SetRange(20, 200);
	return 0;
}

void SettingsDialog::PostNcDestroy()
{
}
