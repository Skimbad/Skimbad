// SkimbadPList.cpp : implementation file
//

#include "stdafx.h"
#include "Skimbad Settings.h"
#include "SkimbadPList.h"
#include "afxdialogex.h"


// SkimbadPList dialog

IMPLEMENT_DYNAMIC(SkimbadPList, CDialog)

SkimbadPList::SkimbadPList(CWnd* pParent /*=NULL*/)
	: CDialog(SkimbadPList::IDD, pParent)
{

}

SkimbadPList::~SkimbadPList()
{
}

void SkimbadPList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESSLIST, m_SkimbadPList);
}

void SkimbadPList::RefreshProcessList()
{
	HANDLE ProcessSnap;
	PROCESSENTRY32 pe32;
	int nIndex;
	std::list <ProcessItem>  pItems;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	ProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(ProcessSnap, &pe32))
	{
		WCHAR numBuff[80];
		_itow_s(pe32.th32ProcessID, numBuff, 10);

		nIndex = m_SkimbadPList.InsertItem(0, numBuff);
		m_SkimbadPList.SetItemText(nIndex, 1, pe32.szExeFile);

		while (Process32Next(ProcessSnap, &pe32))
		{
			_itow_s(pe32.th32ProcessID, numBuff, 10);

			nIndex = m_SkimbadPList.InsertItem(0, numBuff);
			m_SkimbadPList.SetItemText(nIndex, 1, pe32.szExeFile);
		}

	}
	else
	{
		LPTSTR errorstring = NULL;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorstring, 0, NULL);
		MessageBox(errorstring, errorstring, MB_OK | MB_ICONERROR);
		free(errorstring);
		return;
	}

}

BEGIN_MESSAGE_MAP(SkimbadPList, CDialog)
	ON_BN_CLICKED(IDOK, &SkimbadPList::OnBnClickedOk)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &SkimbadPList::OnHdnItemclickProcesslist)
END_MESSAGE_MAP()


// SkimbadPList message handlers


void SkimbadPList::OnBnClickedOk()
{
	POSITION pos = m_SkimbadPList.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		while (pos)
		{
			int row = m_SkimbadPList.GetNextSelectedItem(pos);
			CString cs = m_SkimbadPList.GetItemText(row, 1);
			szPName.push_back(cs);
		}
	}
	CDialog::OnOK();
}


BOOL SkimbadPList::OnInitDialog()
{
	CDialog::OnInitDialog();

	szPName.clear();
	

	//initialize the list control
	m_SkimbadPList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);

	m_SkimbadPList.InsertColumn(0, L"PID", LVCFMT_LEFT, 90);
	m_SkimbadPList.InsertColumn(1, L"Name", LVCFMT_LEFT, 180);

	RefreshProcessList();

	return TRUE; 
}

void SkimbadPList::OnHdnItemclickProcesslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	bAscending = !bAscending;
	if (phdr->iItem == 0)
	{
		//Sort by PID
		if (bAscending)
		{
			m_SkimbadPList.SortItemsEx(PidCompareA, (LPARAM)&m_SkimbadPList);
		}
		else
		{
			m_SkimbadPList.SortItemsEx(PidCompareD, (LPARAM)&m_SkimbadPList);
		}
	}
	else
	{
		//sort by process name
		if (bAscending)
		{
			m_SkimbadPList.SortItemsEx(PNameCompareA, (LPARAM)&m_SkimbadPList);
		}
		else
		{
			m_SkimbadPList.SortItemsEx(PNameCompareD, (LPARAM)&m_SkimbadPList);
		}
	}
	*pResult = 0;
}
