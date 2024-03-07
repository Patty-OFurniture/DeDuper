// DeDuper.h : main header file for the DEDUPER application
//

#if !defined(AFX_DEDUPER_H__833FF314_019E_11D6_94A4_46029F000000__INCLUDED_)
#define AFX_DEDUPER_H__833FF314_019E_11D6_94A4_46029F000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDeDuperApp:
// See DeDuper.cpp for the implementation of this class
//

class CDeDuperApp : public CWinApp
{
public:
	CDeDuperApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeDuperApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDeDuperApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEDUPER_H__833FF314_019E_11D6_94A4_46029F000000__INCLUDED_)
