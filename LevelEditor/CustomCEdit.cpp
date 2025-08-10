#include "stdafx.h"
#include "CustomCEdit.h"

IMPLEMENT_DYNAMIC(CustomCEdit, CEdit)

BEGIN_MESSAGE_MAP(CustomCEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

CustomCEdit::CustomCEdit()
{

}

CustomCEdit::~CustomCEdit()
{
}

void CustomCEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Get current string in the edit box
	CString CurString; 
	GetWindowText(CurString);

	// Get start and end position of selection
	int nStart;
	int nEnd;
	CEdit::GetSel(nStart, nEnd);

	// If char is a number or backspace, accept input
	if (nChar >= '0' && nChar <= '9' || nChar == '\b')
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
	else if (nChar == '.') // If char is a period, accept input if there is not already a period in the string
	{
		if (CurString.Find('.') == -1)
		{
			CEdit::OnChar(nChar, nRepCnt, nFlags);
		}
	}
	else if (nChar == '-') // If char is a negative sign, accept input if there is not already one and the selection is at the start of the string
	{
		if (CurString.Find('-') == -1 && nStart == 0)
		{
			CEdit::OnChar(nChar, nRepCnt, nFlags);
		}
	}	
	else if (nChar == '\r') // Set update flag when return key is pressed
	{
		updateFlag = true;
	}
}

