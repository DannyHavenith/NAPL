// WaveDisplay.cpp : implementation file
//

#include "stdafx.h"
#include "Displays.h"
#include "WaveDisplay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaveDisplay

CWaveDisplay::CWaveDisplay()
{
}

CWaveDisplay::~CWaveDisplay()
{
}


BEGIN_MESSAGE_MAP(CWaveDisplay, CWnd)
	//{{AFX_MSG_MAP(CWaveDisplay)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWaveDisplay message handlers

void CWaveDisplay::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect viewBox;

	dc.SelectStockObject( BLACK_PEN);
	dc.SelectStockObject( BLACK_BRUSH);

	dc.Rectangle( 10, 10, 50, 50);
	
	// Do not call CWnd::OnPaint() for painting messages
}
