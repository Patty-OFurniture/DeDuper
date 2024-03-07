// MsgBox.cpp : implementation file
//

#include "stdafx.h"
#include "DeDuper.h"
#include "MsgBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgBox dialog


CMsgBox::CMsgBox(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsgBox)
	m_sFile1 = _T("");
	m_sFile2 = _T("");
	m_sFilePre = _T("");
	m_sFilePost = _T("");
	m_sFileSize = _T("");
	//}}AFX_DATA_INIT
}


void CMsgBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if ( !pDX->m_bSaveAndValidate)
	{
		int i = this->m_sFile1.GetLength();
		for (int j = 0; j < i; j++)
		{
			if (m_sFile1[j] != m_sFile2[j])
			{
				m_sFilePre = m_sFile1.Left(j);
				break;
			}
		}
		for (int j = 1; j < i; j++)
		{
			if (m_sFile1.Right(j) != m_sFile2.Right(j))
			{
				m_sFilePost = m_sFile1.Right(j-1);
				m_sFile1 = m_sFile1.Left(m_sFile1.GetLength()-j+1);
				m_sFile2 = m_sFile2.Left(m_sFile2.GetLength()-j+1);
				break;
			}
		}
	}

	//{{AFX_DATA_MAP(CMsgBox)
	DDX_Text(pDX, IDC_FILE1, m_sFile1);
	DDX_Text(pDX, IDC_FILE2, m_sFile2);
	DDX_Text(pDX, IDC_FILE_PRE, m_sFilePre);
	DDX_Text(pDX, IDC_FILE_POST, m_sFilePost);
	DDX_Text(pDX, IDC_FSIZE, m_sFileSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMsgBox, CDialog)
	//{{AFX_MSG_MAP(CMsgBox)
	ON_COMMAND_RANGE(ID_FILE0, ID_FILEW, OnFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgBox message handlers

void CMsgBox::OnFile( UINT nID ) 
{
	EndDialog (nID);
}
