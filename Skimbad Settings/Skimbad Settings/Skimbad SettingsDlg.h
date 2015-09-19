
// Skimbad SettingsDlg.h : header file
//

#pragma once

#include "SkimbadPList.h"
#include "afxwin.h"
#include "Misc.h"

#define _WIN32_WINNT _WIN32_WINNT_WINXP

// CSkimbadSettingsDlg dialog
class CSkimbadSettingsDlg : public CDialogEx
{
	BOOL isServiceIsRunning();
	// Construction
public:
	CSkimbadSettingsDlg(CWnd* pParent = NULL);	// standard constructor
	CBrush* m_pRedBkBrush;
	CBrush* m_pGreenBkBrush;
	CBrush* m_pBlackBrush;
	bool SkimBadStatus;
	// Dialog Data
	enum { IDD = IDD_SKIMBADSETTINGS_DIALOG };

	std::vector<CString> szPName;
	DWORD dwNumCards;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedSelectprocess();
	CEdit m_editProcessName;
	CEdit m_editNumCards;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit m_editStatusSkimbadRun;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CButton m_btnSkimStart;
	CButton m_btnSkimStop;
	CButton m_btnSkimRestart;
	afx_msg void OnBnClickedBtnapply();
	afx_msg void OnBnClickedBtnskimstart();
	afx_msg void OnBnClickedBtnskimstop();
	afx_msg void OnBnClickedBtnrestart();
};
