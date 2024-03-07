
typedef struct {
	HWND hWnd;
	BOOL bRecurse;
	BOOL bContinue;
	BOOL bDelete;
	BOOL bAllDrives;
	BOOL bCmpSubdirs;
	unsigned uFileSize;
} ThreadParams;

UINT ThreadProc(LPVOID pParam);

extern CString m_sDirectory;
extern CString m_sDupes;


///////////////////////////////////////////////////
// Mesage notification

#define USER_PROGRESS_SETUPPER	WM_USER + 0x100
#define USER_PROGRESS_INCREMENT	USER_PROGRESS_SETUPPER + 1
#define USER_STATIC_SET			USER_PROGRESS_SETUPPER + 2
#define USER_ERROR				USER_PROGRESS_SETUPPER + 3
#define USER_THREAD_DONE		USER_PROGRESS_SETUPPER + 4
#define USER_LAST_FILESIZE		USER_PROGRESS_SETUPPER + 5

