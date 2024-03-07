// DeDuperDlg.h : header file
//

#if !defined(AFX_DEDUPERDLG_H__833FF316_019E_11D6_94A4_46029F000000__INCLUDED_)
#define AFX_DEDUPERDLG_H__833FF316_019E_11D6_94A4_46029F000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDeDuperDlg dialog

class CDeDuperDlg : public CDialog
{
// Construction
public:
	int LogMsg (LPCTSTR szFormat, ...);
	CDeDuperDlg(CWnd* pParent = NULL);	// standard constructor
	bool SelectDir(CString& str, LPCTSTR szStartDir);
	CString InitialDirectory;

// Dialog Data
	//{{AFX_DATA(CDeDuperDlg)
	enum { IDD = IDD_DEDUPER_DIALOG };
	CEdit	m_cDir;
	CStatic	m_cStatic;
	CProgressCtrl	m_cProgress;
	BOOL	m_bRecurse;
	BOOL	m_bAllDrives;
	UINT	m_uFileSize;
	BOOL	m_bFileSize;
	BOOL	m_bCmpSubdirs;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeDuperDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDeDuperDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnStart();
	afx_msg void OnStop();
	afx_msg void OnClose();
	//}}AFX_MSG
	LONG OnError(WPARAM wParam, LPARAM lParam);
	LONG OnProgressIncrement(WPARAM wParam, LPARAM lParam);
	LONG OnProgressSetupper(WPARAM wParam, LPARAM lParam);
	LONG OnStaticSet(WPARAM wParam, LPARAM lParam);
	LONG OnThreadDone(WPARAM wParam, LPARAM lParam);
	LONG OnLastFilesize(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CString m_sSelectedDir;
	CWinThread *thread;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEDUPERDLG_H__833FF316_019E_11D6_94A4_46029F000000__INCLUDED_)
