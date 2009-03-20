// DisplaysDlg.h : header file
//

#if !defined(AFX_DISPLAYSDLG_H__94FA6F09_A060_11D1_A5BB_00AA001149F7__INCLUDED_)
#define AFX_DISPLAYSDLG_H__94FA6F09_A060_11D1_A5BB_00AA001149F7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "WaveView.h"

/////////////////////////////////////////////////////////////////////////////
// CDisplaysDlg dialog

class CDisplaysDlg : public CDialog
{
// Construction
public:
	CDisplaysDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDisplaysDlg)
	enum { IDD = IDD_DISPLAYS_DIALOG };
	CWaveView	m_viewWave;
	CProgressCtrl	m_progress9;
	CProgressCtrl	m_progress8;
	CProgressCtrl	m_progress7;
	CProgressCtrl	m_progress6;
	CProgressCtrl	m_progress5;
	CProgressCtrl	m_progress4;
	CProgressCtrl	m_progress3;
	CProgressCtrl	m_progress2;
	CProgressCtrl	m_progress1;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplaysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDisplaysDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAYSDLG_H__94FA6F09_A060_11D1_A5BB_00AA001149F7__INCLUDED_)
