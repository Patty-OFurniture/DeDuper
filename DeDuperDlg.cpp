// DeDuperDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeDuper.h"
#include "DeDuperDlg.h"

#include "Thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeDuperDlg dialog

CDeDuperDlg::CDeDuperDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeDuperDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeDuperDlg)
	m_bRecurse = TRUE;
	m_bAllDrives = FALSE;
	m_uFileSize = 0;
	m_bFileSize = TRUE;
	thread = NULL;
	m_bCmpSubdirs = TRUE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDeDuperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeDuperDlg)
	DDX_Control(pDX, IDC_DIR, m_cDir);
	DDX_Control(pDX, IDC_STATUS, m_cStatic);
	DDX_Control(pDX, IDC_PROGRESS1, m_cProgress);
	DDX_Check(pDX, IDC_SUBDIRS, m_bRecurse);
	DDX_Check(pDX, IDC_ALLDRIVES, m_bAllDrives);
	DDX_Text(pDX, IDC_FILESIZELIMIT, m_uFileSize);
	DDX_Check(pDX, IDC_FILESIZE, m_bFileSize);
	DDX_Check(pDX, IDC_CMPSUBDIRS, m_bCmpSubdirs);
	//}}AFX_DATA_MAP
	
	// custom handling
	if ( !m_bFileSize )
		m_uFileSize = 0;
}

BEGIN_MESSAGE_MAP(CDeDuperDlg, CDialog)
	//{{AFX_MSG_MAP(CDeDuperDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDB_START, OnStart)
	ON_BN_CLICKED(IDB_STOP, OnStop)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
//	ON_BN_CLICKED(IDC_SELDIR, OnSeldir)
	ON_MESSAGE(USER_PROGRESS_SETUPPER, OnProgressSetupper)
	ON_MESSAGE(USER_PROGRESS_INCREMENT, OnProgressIncrement)
	ON_MESSAGE(USER_STATIC_SET, OnStaticSet)
	ON_MESSAGE(USER_ERROR, OnError)
	ON_MESSAGE(USER_THREAD_DONE, OnThreadDone)
	ON_MESSAGE(USER_LAST_FILESIZE, OnLastFilesize)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeDuperDlg message handlers

BOOL CDeDuperDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (!InitialDirectory.IsEmpty())
	{
		m_cDir.SetWindowText(InitialDirectory.GetString());
	}
// ====================================

	if (0)
	{
		SYSTEMTIME st;
		FILETIME ft;
		WORD date, time;
	
		st.wYear = 1996;
		st.wMonth = 12;
		st.wDay = 24;
		st.wDayOfWeek = -1;
		st.wHour = 23;
		st.wMinute = 32;
		st.wSecond = 0;
		st.wMilliseconds = 0;

		SystemTimeToFileTime ( &st, &ft );
		FileTimeToDosDateTime ( &ft, &date, &time );

		return FALSE;
	}

// ====================================

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		BOOL loaded = strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (loaded && !strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDeDuperDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDeDuperDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDeDuperDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/////////////////////////////////////////////////////////////////////////////
// Worker thread start and stop


// only one thread now, so make this static
ThreadParams tp;


void CDeDuperDlg::OnStart() 
{
//	if (thread != NULL)
//		return;

	UpdateData (TRUE);

	// verify directory exists

	m_cDir.GetWindowText (m_sDirectory);

	// create thread params
	tp.bContinue = TRUE;
	tp.bRecurse = m_bRecurse;
	tp.hWnd = m_hWnd;
	tp.bAllDrives = m_bAllDrives;
	tp.uFileSize = m_bFileSize ? m_uFileSize*1024 : 0;
	tp.bCmpSubdirs = m_bCmpSubdirs;

	// TODO
	tp.bDelete = FALSE;

	if (m_sDirectory.Right (1) != "\\")
		m_sDirectory += "\\";
//	if (m_bMove && m_sDupes.Right (1) != "\\")
//		m_sDupes += "\\";

	// begin thread
	thread = AfxBeginThread (ThreadProc, &tp, THREAD_PRIORITY_BELOW_NORMAL);
}


void CDeDuperDlg::OnStop() 
{
	tp.bContinue = FALSE;	
}

/*
void CDeDuperDlg::OnSeldir(UINT uID)
{
	CString m_sDir;
	if (uID == IDC_SELDIR)
		m_cDir.GetWindowText (m_sDir);
	else
		m_cDupes.GetWindowText (m_sDir);

	SelectDir (m_sDir, m_sDir);

	if (uID == IDC_SELDIR)
		m_cDir.SetWindowText (m_sDir);
	else
		m_cDupes.SetWindowText (m_sDir);
}
*/

int CALLBACK SetSelProc( HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	if (uMsg==BFFM_INITIALIZED) {

		TCHAR *ch = (TCHAR *) lpData;
		while(*ch) ch++;
		ch--;
		if(*ch == '/' || *ch == '\\')
			*ch = '\0';

		SendMessage( hWnd, BFFM_SETSELECTION, TRUE, lpData );
	}
	return 0;
}


bool CDeDuperDlg::SelectDir(CString &str, LPCTSTR szStartDir)
{
	int iRet = 0;

// BEGIN code directly from MSDN
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	LPMALLOC pMalloc;

	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		ZeroMemory(&bi,sizeof(bi));
		bi.hwndOwner = NULL;
		bi.pszDisplayName = 0;
		bi.pidlRoot = 0;
		bi.lpfn = SetSelProc;
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
		bi.lParam = (LPARAM) szStartDir;

		if (pidl = SHBrowseForFolder(&bi)) {
			iRet = SHGetPathFromIDList(pidl, str.GetBuffer(MAX_PATH));
			str.ReleaseBuffer ();
			pMalloc->Free(pidl);
			pMalloc->Release();
			m_sSelectedDir = str;
		}
	}
// END code directly from MSDN

	if(iRet)
		str.Format (_T("%s\\\0"), m_sSelectedDir.GetString());

	return iRet ? 1 : 0;

}


int CDeDuperDlg::LogMsg(LPCTSTR szFormat, ...)
{
	
	va_list arg;
	va_start( arg, szFormat );

	CString string;
	string.FormatV( szFormat, arg );

	//TODO: m_cListBox.AddString (string);

	va_end( arg );
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Message Notification from thread

LONG CDeDuperDlg::OnProgressSetupper(WPARAM wParam, LPARAM lParam)
{
	m_cProgress.SetRange32 (0, wParam);
	m_cProgress.SetPos (0);
	m_cProgress.SetStep (1);
	return 0;
}

LONG CDeDuperDlg::OnProgressIncrement(WPARAM wParam, LPARAM lParam)
{
	m_cProgress.StepIt();
	return 0;
}

LONG CDeDuperDlg::OnStaticSet(WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	LPCTSTR t = (LPCTSTR) wParam;
	m_cStatic.SetWindowText ( t );
	return 0;
#else
	m_cStatic.SetWindowText ( (LPCTSTR) wParam);
	return 0;
#endif
}

LONG CDeDuperDlg::OnError(WPARAM wParam, LPARAM lParam)
{
	if(thread) {
		WaitForSingleObject (thread->m_hThread, INFINITE);
//		delete thread;
		thread = NULL;
	}
	return 0;
}

LONG CDeDuperDlg::OnLastFilesize(WPARAM wParam, LPARAM lParam)
{
	// ref USER_STATIC_SET
	// this actually could be the LPARAM for OnStaticSet
	m_uFileSize = (UINT) wParam;
	m_bFileSize = TRUE;
	UpdateData(FALSE);
	return 0;
}

LONG CDeDuperDlg::OnThreadDone(WPARAM wParam, LPARAM lParam)
{
/*
?OnThreadDone@CDeDuperDlg@@IAEJIJ@Z:
004019A0   push        esi
004019A1   mov         esi,ecx
004019A3   mov         eax,dword ptr [esi+12Ch]
004019A9   test        eax,eax
004019AB   je          CDeDuperDlg::OnThreadDone+23h (004019c3)
004019AD   mov         eax,dword ptr [eax+28h]
004019B0   push        0FFh
004019B2   push        eax
004019B3   call        dword ptr [__imp__WaitForSingleObject@8 (0042f348)]
004019B9   mov         dword ptr [esi+12Ch],0
004019C3   push        offset string "Finished" (0043a0dc)
004019C8   lea         ecx,[esi+98h]
004019CE   call        CWnd::SetWindowTextA (00421917)
004019D3   xor         eax,eax
004019D5   pop         esi
004019D6   ret         8
004019D9   nop
004019DA   nop

0x0000182d
0x000019ad
*/
	if(thread != NULL) {
		WaitForSingleObject (thread->m_hThread, INFINITE);
//		delete thread;
		thread = NULL;
	}

//	if(m_bExitOnDone)
//		this->PostMessage (WM_QUIT);

	m_cStatic.SetWindowText ( TEXT("Finished"));
	return 0;
}

void CDeDuperDlg::OnClose() 
{

	if (thread != NULL) {
		tp.bContinue = FALSE;
		WaitForSingleObject (thread->m_hThread, INFINITE);
		thread = NULL;
	}

	CDialog::OnClose();
}
