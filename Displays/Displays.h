// Displays.h : main header file for the DISPLAYS application
//

#if !defined(AFX_DISPLAYS_H__94FA6F07_A060_11D1_A5BB_00AA001149F7__INCLUDED_)
#define AFX_DISPLAYS_H__94FA6F07_A060_11D1_A5BB_00AA001149F7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDisplaysApp:
// See Displays.cpp for the implementation of this class
//

class CDisplaysApp : public CWinApp
{
public:
	CDisplaysApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplaysApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDisplaysApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAYS_H__94FA6F07_A060_11D1_A5BB_00AA001149F7__INCLUDED_)
