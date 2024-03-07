#include "stdafx.h"

// comment this to use FILE* instead of CFile, for performance testing
#define USE_MFCFILES

#include "Thread.h"

CString m_sDirectory;

#include <list>
#include <string>

// Provide  operator< for std::list::sort()
class FileStruct {
public:
	unsigned size;
	std::string fullpath;
	operator< (const FileStruct& fs) { return size > fs.size; };
};

std::list<FileStruct> FileList;


int osd_WalkDir( LPCTSTR szWalkdir, /*DIRPROC(pfn), */ LPCTSTR szExt, BOOL bSubdirs ) {

	TCHAR cFile[_MAX_PATH];
	int iRet = 0;

	HANDLE hFindFile = 0;
	WIN32_FIND_DATA FileInfo;

	FileStruct fs;

	strcpy(cFile, szWalkdir);
	strcat(cFile, szExt);
//	if( szExt[0] == '*')								// use extension only
//		szExt++;

	hFindFile = ::FindFirstFile(cFile, &FileInfo);
	
	if (hFindFile != INVALID_HANDLE_VALUE) {
		iRet = 1;
		do {
			if(FileInfo.cFileName[0] != '.') {
				if((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && bSubdirs) {
					strcat( cFile, "\\" );
					osd_WalkDir( cFile, szExt, true );
				} else {
					// process the file
					ASSERT(!FileInfo.nFileSizeHigh);
					fs.size = FileInfo.nFileSizeLow;
					fs.fullpath = szWalkdir;
					fs.fullpath += FileInfo.cFileName;
					FileList.push_back (fs);
				}
			}
		} while(::FindNextFile(hFindFile, &FileInfo));

		::FindClose( hFindFile );
	}

	hFindFile = 0;
	return iRet;
}


UINT ThreadProc(LPVOID pParam) {

	CString sSearch = m_sDirectory;
	CString sExt = "*";

	ThreadParams *tp = (ThreadParams *) pParam;

	// create a list of all files
	osd_WalkDir (sSearch, sExt, (BOOL) pParam);

	if (FileList.size() < 1) {
		// error message somehow
		return 0;
	}

	// put the list in order
	FileList.sort();

	// TODO: for multiple dirs, we could have a separate compare thread

	//take files with the same filesize
	std::list<FileStruct>::iterator ref, cmp;

	ref = FileList.begin();
	do {
#ifdef USE_MFCFILES
		CFile cmpFile, refFile;
#else
		FILE *cmpFile, *refFile;
#endif

		cmp = ref;
		cmp++;
		if (cmp == FileList.end())
			break;

		if (cmp->size == ref->size) {

//#ifdef USE_MFCFILES
			// compare files
			refFile.Open (ref->fullpath.c_str(), CFile::modeRead);
			do {
				cmpFile.Open (cmp->fullpath.c_str(), CFile::modeRead);

#ifdef _DEBUG
				TRACE(cmp->fullpath.c_str());
				TRACE("\n");
				TRACE(ref->fullpath.c_str());
				TRACE("\n");
				TRACE("\n");
#endif
				
				// OS uses file buffering so I don't have to
				UINT size = cmp->size;
				BYTE cmpByte, refByte;
				do {
					if (EOF == cmpFile.Read (&cmpByte, 1))
						break;
					if (EOF == refFile.Read (&refByte, 1))
						break;
					if (cmpByte != refByte)
						break;
					size--;
				} while (size);

				cmpFile.Close();
				refFile.Rewind();

				if (!size) {
					// ask which one to delete
					sSearch.Format ("%s\n%s\nDelete the first one?", cmp->fullpath.c_str(), ref->fullpath.c_str());
					size = AfxMessageBox (sSearch, MB_YESNOCANCEL|MB_DEFBUTTON3, NULL);

					// delete it, and the iterator
					// hack: erase() returns the next iterator since current is invalid
					if (size == IDYES) {
						CFile::Remove (cmp->fullpath.c_str());
						cmp = FileList.erase(cmp);
						cmp--;
					} else if (size == IDNO) {
						CFile::Remove (ref->fullpath.c_str());
						ref = FileList.erase(ref);
						ref--;
					}
				}
				cmp++;
			} while (cmp != FileList.end() && cmp->size == ref->size);

			refFile.Close();
		}

		if (!tp->bContinue)
			break;

		ref++;
	} while (ref != FileList.end());

	// reset the list
	FileList.clear();
	return 0;
}
