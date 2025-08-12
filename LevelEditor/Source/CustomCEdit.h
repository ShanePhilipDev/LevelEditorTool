#pragma once
#include <afxwin.h>

//CEdit class that only accepts float inputs
class CustomCEdit : public CEdit
{
	DECLARE_DYNAMIC(CustomCEdit)

public:
	CustomCEdit();
	virtual ~CustomCEdit();

	// flag to signal enter key has been pressed
	bool updateFlag;

protected:
	DECLARE_MESSAGE_MAP();

	// input validation is done for each character
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

