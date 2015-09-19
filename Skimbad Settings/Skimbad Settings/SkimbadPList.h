#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <list>
#include "afxcmn.h"
#include "Misc.h"
#include <vector>

// SkimbadPList dialog

class SkimbadPList : public CDialog
{
	DECLARE_DYNAMIC(SkimbadPList)

public:
	SkimbadPList(CWnd* pParent = NULL);   // standard constructor
	virtual ~SkimbadPList();

	typedef struct
	{
		DWORD pid;
		WCHAR* szpid;
		WCHAR* name;
	} ProcessItem;

	std::vector<CString> szPName;

	bool bAscending = 0;

// Dialog Data
	enum { IDD = IDD_SKIMBADPLIST_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void SkimbadPList::RefreshProcessList();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_SkimbadPList;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnHdnItemclickProcesslist(NMHDR *pNMHDR, LRESULT *pResult);
};
