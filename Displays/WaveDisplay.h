#if !defined(AFX_WAVEDISPLAY_H__94FA6F11_A060_11D1_A5BB_00AA001149F7__INCLUDED_)
#define AFX_WAVEDISPLAY_H__94FA6F11_A060_11D1_A5BB_00AA001149F7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WaveDisplay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWaveDisplay window

class CWaveDisplay : public CWnd
{
// Construction
public:
	CWaveDisplay();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveDisplay)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWaveDisplay();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWaveDisplay)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEDISPLAY_H__94FA6F11_A060_11D1_A5BB_00AA001149F7__INCLUDED_)
