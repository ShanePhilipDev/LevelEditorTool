#pragma once
#include <afxdialogex.h>
#include "afxwin.h"
#include "resource.h"
#include "ToolMain.h"

class Game;

class SettingsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(SettingsDialog)

public:
	SettingsDialog(CWnd* pParent = NULL);
	virtual ~SettingsDialog();

	// Sets up initial values for sliders and game reference.
	void SetGameRef(Game* game);
protected:
	// Dialog button functions and data exchange
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void End();		
	afx_msg void Apply();

	// Game reference as we are adjusting game settings.
	Game* m_gameRef;

	// Sliders for the adjustable properties.
	CSliderCtrl m_camMoveSpeed, m_camRotSpeed, m_focusZoomSpeed, m_focusLerpSpeed;
	CSliderCtrl m_objMoveSpeed, m_objRotSpeed, m_objScaleSpeed;
	CSliderCtrl m_sculptRadius, m_sculptMagnitude;
	
	// Multipliers for slider values.
	float m_objMultiplier;
	float m_camMultiplier;
	float m_sculptMultiplier;

	DECLARE_MESSAGE_MAP();

public:
	// Init and destroy
	virtual BOOL OnInitDialog() override;
	virtual void PostNcDestroy();
};

