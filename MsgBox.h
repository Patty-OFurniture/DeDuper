#if !defined(AFX_MSGBOX_H__7D9A2301_02C0_11D6_94A4_46029F000000__INCLUDED_)
#define AFX_MSGBOX_H__7D9A2301_02C0_11D6_94A4_46029F000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgBox.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CMsgBox dialog

class CMsgBox : public CDialog
{
// Construction
public:
	INT nRetVal;
	CMsgBox(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMsgBox)
	enum { IDD = IDD_DIALOG1 };
	CString	m_sFile1;
	CString	m_sFile2;
	CString	m_sFilePre;
	CString	m_sFilePost;
	CString	m_sFileSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMsgBox)
	afx_msg void OnFile(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGBOX_H__7D9A2301_02C0_11D6_94A4_46029F000000__INCLUDED_)
