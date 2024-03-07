#include "stdafx.h"
#include "tchar.h"

#pragma warning(disable:4786)

#define AUTODEL 0
// #define _DEBUG_TRACE


#include "Thread.h"
#include "resource.h"
#include "MsgBox.h"
#include "crc32.h"

CString m_sDirectory;
CString m_sDupes;


#include <list>
#include <vector>
#include <string>

#ifndef MAX_FILE
#define MAX_FILE 260*2
#endif

#if defined(_UNICODE)
#define STD_STRING std::wstring
#else
#define STD_STRING std::string
#endif

#define CHECK_CANCEL() if ( tp->bContinue == FALSE ) break


// Provide operator< for std::list::sort()
// operator< returns (>) so the sort will be largest to smallest

extern unsigned crcTable[256];

class FileStruct {
public:
	unsigned size;
	size_t filehash;
	STD_STRING fullpath;
	bool operator< (const FileStruct& fs) {
		return (size > fs.size);
	};
};

std::list<FileStruct> FileList;
std::vector<STD_STRING> AlwaysDel;
std::vector<STD_STRING> NeverDel;

STD_STRING GetPath( STD_STRING& str)
{
	STD_STRING ret;
	int n = str.rfind ( '\\');

	if (n == std::string::npos)
		ret = str;
	else
		ret = str.substr(0, n+1);

	return ret;
}

#define CHECKDELETE_ALWAYS   0x0000FFFF
#define CHECKDELETE_NEVER    0xFFFF0000
#define CHECKDELETE_NOTFOUND 0

int CheckDelete ( STD_STRING& arg )
{
	STD_STRING str = GetPath(arg);

	std::vector<STD_STRING>::iterator it = NeverDel.begin();
	while (it != NeverDel.end())
	{
		if (*it++ == str)
			return CHECKDELETE_NEVER;
	}

	it = AlwaysDel.begin();
	while (it != AlwaysDel.end())
	{
		if ( *it++ == str )
			return CHECKDELETE_ALWAYS;
	}

	return CHECKDELETE_NOTFOUND;
}

void NeverDelete(STD_STRING& str)
{
	STD_STRING folder = GetPath(str);
	if (CheckDelete(str) == CHECKDELETE_NOTFOUND)
	{
		NeverDel.push_back(folder);
	}
}

void AlwaysDelete ( STD_STRING& str )
{
	STD_STRING folder = GetPath(str);
	if ( CheckDelete (str) == CHECKDELETE_NOTFOUND )
	{
		AlwaysDel.push_back(folder);
	}
}

int osd_WalkDir( LPCTSTR szWalkdir, /*DIRPROC(pfn), */ LPCTSTR szExt, BOOL bSubdirs, unsigned uFileSize ) {

	TCHAR cFile[_MAX_PATH];
	int iRet = 0;
	int iFiles = 0;
	TCHAR *p;

	HANDLE hFindFile = 0;
	WIN32_FIND_DATA FileInfo;

	FileStruct fs;

	// add the extension, then remove it for subdirs
	::lstrcpy(cFile, szWalkdir);
	p = cFile;
	while (*p) p++;
	lstrcat(p, szExt);

	memset(&FileInfo, 0, sizeof(FileInfo));
	hFindFile = ::FindFirstFile(cFile, &FileInfo);

	*p = '\0';

	if (hFindFile != INVALID_HANDLE_VALUE) {
		do {
			if(FileInfo.cFileName[0] != '.') {
				iFiles++;
				if(FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bSubdirs) {
						if ( lstrcmp (FileInfo.cFileName, _TEXT("ServicePackFiles")) &&
							 lstrcmp (FileInfo.cFileName, _TEXT("dllcache")) ) {
							 ::lstrcpy (p, FileInfo.cFileName);
							 lstrcat( p, _TEXT("\\") );
							 iRet += osd_WalkDir( cFile, szExt, true, uFileSize );
						}
					}
				} else {

					// process the file, error for files > 4GB
					// nFileSizeHigh is DWORD is unsigned long
					// ASSERT(!FileInfo.nFileSizeHigh);

					// only examine large/small files
					if ( !uFileSize || (FileInfo.nFileSizeLow <= uFileSize) )
					{
#ifdef _DEBUG
//						char *p = strrchr ( FileInfo.cFileName, '.' );
//						if (p && (tolower(p[1]) == 'j') )
#endif
						{
							fs.size = FileInfo.nFileSizeLow;
							fs.fullpath = szWalkdir;
							fs.fullpath += FileInfo.cFileName;
							fs.filehash = 0;
							FileList.push_back (fs);
							iRet++;
						}
					}
				}
			}
			memset(&FileInfo, 0, sizeof(FileInfo));
		} while(::FindNextFile(hFindFile, &FileInfo));

		::FindClose( hFindFile );
	}

	hFindFile = 0;

	// delete empty dirs
	if (!iFiles) {
		iFiles = ::RemoveDirectory ( szWalkdir );
		if ( !iFiles )
			iFiles = ::GetLastError ();
	}

	return iRet;
}


FILE* __cdecl wrap_tfopen(
	_In_z_ wchar_t const* _FileName,
	_In_z_ wchar_t const* _Mode
)
{
	FILE* fp = _tfopen(_FileName, _Mode);
	if (fp == NULL)
	{
		STD_STRING fileName = TEXT("\\\\?\\");
		fileName.append(_FileName);
		fp = _tfopen(fileName.c_str(), _Mode);
	}
	return fp;
}

CString ExtractFileName(CString FileName, BOOL bCmpSubdirs)
{
	CString Path;
	int i=-1; 

	i = FileName.ReverseFind( '\\' );
	if (bCmpSubdirs)
	{
		if (i)
			i = FileName.Left(i).ReverseFind( '\\' );
	}

	if (i==-1)
		i = FileName.ReverseFind( ':' );
	if (i==-1)
		i = FileName.ReverseFind( '/' );
	if (i>=0)
		Path = FileName.Mid (i);
	else
		Path = FileName; 

	return Path;
}

UINT GetClusterSize(STD_STRING filepath)
{
	DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;
	// this should be per-drive, that's ignored for now
	static DWORD ClusterSize = 0;

	if (!ClusterSize)
	{
		TCHAR cDrive[4];
		_tcsncpy(cDrive, filepath.c_str(), 3);
		cDrive[3] = '\0';

		::GetDiskFreeSpace(cDrive, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters);
		ClusterSize = BytesPerSector * SectorsPerCluster;
	}
	return ClusterSize;
}

UINT CalcSystemBytes(std::list<FileStruct>::iterator itFile ) {

	UINT systemBytes = 0;
	DWORD ClusterSize;

	ClusterSize = GetClusterSize(itFile->fullpath);

	systemBytes = itFile->size / ClusterSize;
	systemBytes *= ClusterSize;

	if ( itFile->size != systemBytes )
		systemBytes += ClusterSize;

	return systemBytes;
}

UINT DeleteOrRecycleFile(LPCTSTR pszPath, BOOL bDelete) {

	UINT ret = FALSE;

	if (bDelete)
		ret = ::DeleteFile(pszPath);
	else {
		SHFILEOPSTRUCT lpFileOp;
		memset(&lpFileOp, 0, sizeof(SHFILEOPSTRUCT));

		int pathLength = _tcslen(pszPath);

		// Copy pathname to double-NULL-terminated string.
		TCHAR* buf = new TCHAR[pathLength + 2]; // +1 for extra null, +1 for safety

		_tcscpy(buf, pszPath);    // copy caller's path name
		buf[pathLength + 1] = 0;    // need two NULLs at end

		lpFileOp.fFlags = FOF_SILENT | FOF_ALLOWUNDO | FOF_NOCONFIRMATION;

		lpFileOp.wFunc = FO_DELETE;                   // REQUIRED: delete operation
		lpFileOp.pFrom = buf;                         // REQUIRED: which file(s)
		lpFileOp.pTo = NULL;                          // MUST be NULL
		if (bDelete) {                                // if delete requested..
			lpFileOp.fFlags &= ~FOF_ALLOWUNDO;        // ..don't use Recycle Bin
		}
		else {                                        // otherwise..
			lpFileOp.fFlags |= FOF_ALLOWUNDO;         // ..send to Recycle Bin
		}
		return SHFileOperation(&lpFileOp);             // Make it so
	}

	return ret;
}

UINT RemoveFile(LPCTSTR pszPath, LPCTSTR pszDupes, BOOL bDelete) {

	UINT ret = FALSE;

	if (pszDupes != NULL) {

		TCHAR pszNewPath[MAX_FILE];
		LPCTSTR p = _tcsrchr(pszPath, '\\');
		ASSERT(p);

		::lstrcpy(pszNewPath, pszDupes);
		::lstrcat(pszNewPath, &p[1]);
		ret = ::MoveFile(pszPath, pszNewPath);
	}
	else
	{
		//ret = ::DeleteFile (pszPath);
		ret = DeleteOrRecycleFile(pszPath, bDelete);
		if (ret != 0)
		{
			DWORD fa = GetFileAttributes(pszPath);
			if (fa & FILE_ATTRIBUTE_READONLY)
			{
				SetFileAttributes(pszPath, fa & ~FILE_ATTRIBUTE_READONLY);
				ret = DeleteOrRecycleFile(pszPath, bDelete);
				if (ret != 0)
				{
					CString str;
					str.Format(_TEXT("Could not delete %s"), pszPath);
					AfxMessageBox(str, MB_OK, NULL);
				}
			}
		}
		// logic was inverted
		if (ret == 0)
			ret = 1;
		else
			ret = 0;
	}
	return ret;
}


TCHAR *nformat (unsigned n, float *amt)
{
	TCHAR *cUnits[] =
	{
		_TEXT("Bytes"), _TEXT("KB"), _TEXT("MB"), _TEXT("GB"), _TEXT("TB"), _TEXT("PB"), _TEXT("ERR"), NULL
	};

	int unit;
	*amt = (float) n;

	for (unit = 0; cUnits[unit]; unit++)
	{
		if (*amt/1024 < 1)
			break;
		*amt /= 1024;
	}

	return cUnits[unit];
}

void GetBlockkHash(size_t maxBlockSize, std::list<FileStruct>::iterator cmp) {
	return;

	auto cmpFile = wrap_tfopen(cmp->fullpath.c_str(), _TEXT("rb"));

	if (cmpFile)
	{
		if (maxBlockSize > cmp->size)
			maxBlockSize = cmp->size;

		char* bytes = new char[maxBlockSize];

		fread(bytes, 1, maxBlockSize, cmpFile);

		// platform independent 0xffffffff
		size_t tempHash = (size_t)(0 - 1); // precondition
		for(size_t x = 0; x < maxBlockSize; x++)
		{
			tempHash = ((tempHash >> 8) & 0x00FFFFFF) ^ crcTable[(tempHash^bytes[x]) & 0xFF];
		}
		// postcondition
		cmp->filehash = tempHash ^ 0xFFFFFFFF;

		delete bytes;

		fclose(cmpFile);
	}
}

UINT ThreadProc(LPVOID pParam) {

	CString sSearch = m_sDirectory;
	CString sExt = "*";

	// reporting total numbers
	unsigned nFiles = 0;
	unsigned nBytes = 0;
	unsigned nSystemBytes = 0;

	crcgen();

	ThreadParams *tp = (ThreadParams *) pParam;

	::SendMessage (tp->hWnd, USER_STATIC_SET, (WPARAM) _TEXT("Searching for files..."), 0);

	// create a list of all files
	if ( tp->bAllDrives )
	{
		TCHAR cDrives[1024];
		TCHAR *p = &cDrives[0];
		DWORD len = GetLogicalDriveStrings ( sizeof(cDrives)-1, cDrives );

		while (*p) 
		{
			if ( DRIVE_FIXED == GetDriveType(p) )
			{
				nFiles += osd_WalkDir (p, sExt, tp->bRecurse, tp->uFileSize);
			}
			while (*p++)
				;
		}

	} else {
		nFiles = osd_WalkDir (sSearch, sExt, tp->bRecurse, tp->uFileSize);
	}

	if (FileList.size() < 1) {
		// error message somehow
		return 0;
	}

	// update UI
	::SendMessage(tp->hWnd, USER_PROGRESS_SETUPPER, (WPARAM) nFiles, 0);
	::SendMessage(tp->hWnd, USER_STATIC_SET, (WPARAM) _TEXT("Checking files..."), 0);

	nFiles = 0;

#ifdef _DEBUG
	//  VC++ 6.0 requires Dinkumware STL Patches.  Detect when sort() loses data
	auto FileList_size = FileList.size();
#endif

	// put the list in order
	FileList.sort();

#ifdef _DEBUG
	//  VC++ 6.0 requires Dinkumware STL Patches.  Detect when sort() loses data
	if ( FileList_size != FileList.size() )
	{
		::DebugBreak();
		FileList_size = FileList.size();
	}
#endif

	//take files with the same filesize
	std::list<FileStruct>::iterator ref, cmp;
	ref = FileList.begin();

	ASSERT (!nFiles);	// counter for deleted files

	UINT ClusterSize = GetClusterSize(ref->fullpath);

	do {
		FILE *cmpFile, *refFile;

		cmp = ref;
		cmp++;
		if (cmp == FileList.end())
			break;


		if (cmp->size == ref->size) {

#ifdef _DEBUG_TRACE
			char szFileSize[1024];
			sprintf(&szFileSize[0], "%d\r\n", ref->size);
			TRACE(szFileSize);
#endif
			// compare files

			if (!ref->filehash)
				GetBlockkHash(ClusterSize, ref);

			refFile = wrap_tfopen(ref->fullpath.c_str(), _TEXT("rb") );
			if (!refFile) {
#ifdef _DEBUG_TRACE
				TRACE(ref->fullpath.c_str());
				TRACE(_TEXT("\n"));
#endif
				::SendMessage(tp->hWnd, USER_LAST_FILESIZE, (WPARAM)(ref->size / 1024) + 1, 0);
				sSearch.Format ( _TEXT("Can't open file %s"), ref->fullpath.c_str() );
				AfxMessageBox ( sSearch, MB_OK, NULL);

			} else do {

				CHECK_CANCEL();

				if (!cmp->filehash)
					GetBlockkHash(ClusterSize, cmp);

				if (cmp->filehash != ref->filehash)
				{
					cmp++;
					continue;
				}

				cmpFile = wrap_tfopen(cmp->fullpath.c_str(), _TEXT("rb") );

				if (!cmpFile) {
#ifdef _DEBUG_TRACE
					TRACE(cmp->fullpath.c_str());
					TRACE(_TEXT("\n"));
#endif
					::SendMessage(tp->hWnd, USER_LAST_FILESIZE, (WPARAM)(cmp->size / 1024) + 1, 0);
					sSearch.Format ( _TEXT("Can't open file %s"), cmp->fullpath.c_str());
					AfxMessageBox ( sSearch, MB_OK, NULL);

					// remove it so we don't try again with ref when we get there
					cmp = FileList.erase(cmp);
					continue;
				}

				// OS uses file buffering so I don't have to
				// + 1 for eof - one file having EOF early will make them different
				UINT size = cmp->size + 1;
				int x;

				while ((x = fgetc(cmpFile)) == fgetc(refFile)) {
					size--;

					if (feof(refFile))
						break;
				}

				fclose (cmpFile);
				rewind (refFile);

				if (!size) {

					// ask which one to delete
					CMsgBox box;
					CString str;
					float f;
					TCHAR *p;

					p = nformat (cmp->size, &f);
					if (cmp->size >= 1024)
						box.m_sFileSize.Format ( _TEXT("%.2f %s"), f, p);
					else
						box.m_sFileSize.Format ( _TEXT("%.0f %s"), f, p);

					if ( AUTODEL )
					{

						box.m_sFile1 = ExtractFileName ( ref->fullpath.c_str(), tp->bCmpSubdirs );
						box.m_sFile2 = ExtractFileName ( cmp->fullpath.c_str(), tp->bCmpSubdirs );

//						box.m_sFile1.MakeUpper();
//						box.m_sFile2.MakeUpper();

						if ( box.m_sFile1.CompareNoCase ( box.m_sFile2 )) {
							if (0) {
								box.m_sFile1 = ref->fullpath.c_str() ;
								box.m_sFile2 = cmp->fullpath.c_str() ;
							}
							size = box.DoModal ();
						} else {
							size = ID_FILE2;
						}
					} else {

						int resultref = CheckDelete(ref->fullpath);
						int resultdel = CheckDelete(cmp->fullpath);
						if (resultref == CHECKDELETE_ALWAYS)
							size = ID_FILE1;
						else if (resultdel == CHECKDELETE_ALWAYS)
							size = ID_FILE2;
						else if (resultref == CHECKDELETE_NEVER)
						{
							// never delete, don't ask
							// and if neither was on the "always" list
							// just ignore it
							size = ID_FILEV;
						}
						else if (resultdel == CHECKDELETE_NEVER)
						{
							// never delete, don't ask
							// and if neither was on the "always" list
							// just ignore it
							size = ID_FILEW;
						}
						else
						{

							box.m_sFile1 = ref->fullpath.c_str();
							box.m_sFile2 = cmp->fullpath.c_str();
							size = box.DoModal ();
						}
					}


					// delete it, and the iterator
					// hack: erase() returns the next iterator since current is invalid
					//       but we have cmp++ and ref++ below

					// TODO: mark ->bSaved so we know which ones were selected?
					// UI should let us know this
					// algo should prevent deleting the last dupe, since deleted files are removed.

					if (size == ID_FILEA)
					{
						size = ID_FILE1;
						AlwaysDelete ( ref->fullpath );
					}
					else if (size == ID_FILEB)
					{
						size = ID_FILE2;
						AlwaysDelete ( cmp->fullpath );
					}
					else if (size == ID_FILEV)
					{
						NeverDelete(ref->fullpath);
					}
					else if (size == ID_FILEW)
					{
						NeverDelete(cmp->fullpath);
					}

					if (size == ID_FILE2) {

						if (RemoveFile (cmp->fullpath.c_str(), NULL, tp->bDelete)) {
							nFiles++;
							nBytes += cmp->size;
							nSystemBytes += CalcSystemBytes (cmp);

							float amt;
							TCHAR* p = nformat(nBytes, &amt);

							sSearch.Format(_TEXT("Checking files... %.2f %s"), amt, p);

							::SendMessage(tp->hWnd, USER_STATIC_SET, (WPARAM)sSearch.GetString(), 0);
							::SendMessage(tp->hWnd, USER_LAST_FILESIZE, (WPARAM)(cmp->size / 1024) + 1, 0);
						}

						cmp = FileList.erase(cmp);
						cmp--;

					} else if (size == ID_FILE1) {

						// have to close it before deleting it
						fclose (refFile);
						refFile = NULL;

						if (RemoveFile (ref->fullpath.c_str(), NULL, tp->bDelete)) {
							nFiles++;
							nBytes += ref->size;
							nSystemBytes += CalcSystemBytes(cmp);

							float amt;
							TCHAR* p = nformat(nBytes, &amt);

							sSearch.Format(_TEXT("Checking files... %.2f %s"), amt, p);

							::SendMessage(tp->hWnd, USER_STATIC_SET, (WPARAM)sSearch.GetString(), 0);
							::SendMessage(tp->hWnd, USER_LAST_FILESIZE, (WPARAM)(cmp->size / 1024) + 1, 0);
						}

						ref = FileList.erase(ref);

						// if the first file is erased, don't decrement
						// TODO: ref++ below causes problems?
						// "cannot decrement begin list iterator"
						// for now, don't delete the first file
						if (ref != FileList.begin())
							ref--;

						break;
					} else if (size == ID_FILECAN) {

						// stop comparing and exit
						tp->bContinue = FALSE;
						break;
					}
				}
				cmp++;
				CHECK_CANCEL();
			} while (cmp != FileList.end() && cmp->size == ref->size);

			if (refFile != NULL)
				fclose (refFile);
		}

		CHECK_CANCEL();

		// UI update
		::SendMessage (tp->hWnd, USER_PROGRESS_INCREMENT, 0, 0);
		ref++;

	} while (ref != FileList.end());

	// reset the list
	FileList.clear();

	// update UI and alert user
	::SendMessage(tp->hWnd, USER_STATIC_SET, (WPARAM) _TEXT("Idle"), 0);
	::SendMessage(tp->hWnd, USER_PROGRESS_SETUPPER, (WPARAM) nFiles, 0);

	// reset size to 0 if this wasn't cancelled
	if (tp->bContinue)
		::SendMessage(tp->hWnd, USER_LAST_FILESIZE, (WPARAM)(0), 0);


	if (nBytes)
	{

		float amt, sys;
		TCHAR *p = nformat (nBytes, &amt);
		TCHAR *q = nformat (nSystemBytes, &sys);

		sSearch.Format ( _TEXT("Done - removed %.2f %s in %u file%c\n%.2f %s including FS overhead") , amt, p, nFiles, nFiles==1?' ':'s', sys, q );

	} else {
		sSearch.Format ( _TEXT("Done - examined %u file%c"), nFiles, nFiles==1?' ':'s' );
	}

	AfxMessageBox (sSearch, MB_OK, 0);

	::PostMessage (tp->hWnd, USER_THREAD_DONE, 0, 0);

	return 0;
}
