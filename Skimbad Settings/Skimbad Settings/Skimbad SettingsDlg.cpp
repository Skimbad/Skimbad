
// Skimbad SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Skimbad Settings.h"
#include "Skimbad SettingsDlg.h"
#include "afxdialogex.h"
#include <winsvc.h>

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif


// CSkimbadSettingsDlg dialog

BOOL CSkimbadSettingsDlg::isServiceIsRunning()
{
	BOOL result = FALSE;
	SERVICE_STATUS Status;
	SC_HANDLE schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	if (schSCManager)
	{
		SC_HANDLE SHandle = OpenService(schSCManager, L"SkimbadRunService", SC_MANAGER_ALL_ACCESS);
		if (SHandle == NULL)
		{
			result = FALSE;
		}
		QueryServiceStatus(SHandle, &Status);
		switch (Status.dwCurrentState)
		{
		case SERVICE_RUNNING:
			result = TRUE;
			break;
		case SERVICE_STOPPED:
		default:
			result = FALSE;
			break;
		}
		CloseServiceHandle(SHandle);
	}
	CloseServiceHandle(schSCManager);
	return result;
}

CSkimbadSettingsDlg::CSkimbadSettingsDlg(CWnd* pParent)
	: CDialogEx(CSkimbadSettingsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pRedBkBrush = new CBrush(RGB(255, 0, 0));
	m_pGreenBkBrush = new CBrush(RGB(0, 255, 0));
	m_pBlackBrush = new CBrush(RGB(0, 0, 0));
}

void CSkimbadSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESSNAME, m_editProcessName);
	DDX_Control(pDX, IDC_NUMCARDS, m_editNumCards);
	DDX_Control(pDX, IDC_STATUSSKIMBADRUN, m_editStatusSkimbadRun);
	DDX_Control(pDX, IDC_BTNSKIMSTART, m_btnSkimStart);
	DDX_Control(pDX, IDC_BTNSKIMSTOP, m_btnSkimStop);
	DDX_Control(pDX, IDC_BTNRESTART, m_btnSkimRestart);
}

BEGIN_MESSAGE_MAP(CSkimbadSettingsDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSkimbadSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SELECTPROCESS, &CSkimbadSettingsDlg::OnBnClickedSelectprocess)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTNAPPLY, &CSkimbadSettingsDlg::OnBnClickedBtnapply)
	ON_BN_CLICKED(IDC_BTNSKIMSTART, &CSkimbadSettingsDlg::OnBnClickedBtnskimstart)
	ON_BN_CLICKED(IDC_BTNSKIMSTOP, &CSkimbadSettingsDlg::OnBnClickedBtnskimstop)
	ON_BN_CLICKED(IDC_BTNRESTART, &CSkimbadSettingsDlg::OnBnClickedBtnrestart)
END_MESSAGE_MAP()


// CSkimbadSettingsDlg message handlers

BOOL CSkimbadSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	//Load registry settings
	HKEY hkey;
	LSTATUS stat;
	DWORD created;

	stat = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Skimbad", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_READ, NULL, &hkey, &created);

	if (stat == ERROR_SUCCESS)
	{
		DWORD dwSize = MAX_PATH;
		for (std::vector<CString>::const_iterator it = szPName.begin(); it != szPName.end(); ++it)
		{
			WCHAR szPNameTMP[MAX_PATH];
			StrCpy(szPNameTMP, (*it));
			RegQueryValueEx(hkey, L"ProcessName", NULL, NULL, (BYTE *)szPNameTMP, &dwSize);
		}
		

		dwSize = sizeof(DWORD);
		RegQueryValueEx(hkey, L"NumCards", NULL, NULL, (BYTE *) &dwNumCards, &dwSize);

		if (dwNumCards == 0 && dwNumCards>100)
		{
			dwNumCards = 100;
		}

		WCHAR buffer[100];
		_itow_s(dwNumCards, buffer, 10);
		m_editNumCards.SetWindowTextW(buffer);

	}
	else
	{
		//Failed to open key, set default values
		
	}

	RegCloseKey(hkey);

	//Setup up monitoring timer
	SetTimer(NULL, 500, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSkimbadSettingsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSkimbadSettingsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSkimbadSettingsDlg::OnBnClickedOk()
{
	HKEY hkey;
	LSTATUS stat;
	DWORD created;
	WCHAR buffer[100];

	m_editNumCards.GetWindowTextW(buffer, 100);
	dwNumCards = _wtoi(buffer);

	stat = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Skimbad", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, &created);

	if (stat == ERROR_SUCCESS)
	{
		for (std::vector<CString>::const_iterator it = szPName.begin(); it != szPName.end(); ++it)
		{
			WCHAR szPNameTMP[MAX_PATH];
			StrCpy(szPNameTMP, (*it));
			RegSetValueEx(hkey, L"ProcessName", NULL, REG_SZ, (BYTE *)szPNameTMP, wcslen(szPNameTMP) * sizeof(WCHAR));
			RegSetValueEx(hkey, L"NumCards", NULL, REG_DWORD, (BYTE *)&dwNumCards, sizeof(DWORD));
		}
		
		
	}
	else
	{
		//Failed to open key try
	}

	RegCloseKey(hkey);
	
	CDialogEx::OnOK();
}


void CSkimbadSettingsDlg::OnBnClickedSelectprocess()
{
	SkimbadPList pl;
	pl.DoModal();
	CString selected_processes;
	for (std::vector<CString>::const_iterator it = pl.szPName.begin(); it != pl.szPName.end(); ++it)
	{
		WCHAR szPNameTMP[MAX_PATH];
		StrCpy(szPNameTMP, (*it));
		if (wcscmp(L"", szPNameTMP))
		{
			szPName.push_back((*it));
		}
		selected_processes += (*it);
		selected_processes +=	"; ";
	}
	if (!pl.szPName.empty())
		m_editProcessName.SetWindowTextW(selected_processes);
}


void CSkimbadSettingsDlg::OnTimer(UINT_PTR nIDEvent)
{

	//Check on Skimbad service
	if (!isServiceIsRunning())
	{
		SkimBadStatus = 0;
		m_editStatusSkimbadRun.SetWindowTextW(L"Stopped");
		m_btnSkimStart.EnableWindow(1);
		m_btnSkimStop.EnableWindow(0);
		m_btnSkimRestart.EnableWindow(0);

	}
	else
	{
		SkimBadStatus = 1;
		m_editStatusSkimbadRun.SetWindowTextW(L"Running");
		m_btnSkimStart.EnableWindow(0);
		m_btnSkimStop.EnableWindow(1);
		m_btnSkimRestart.EnableWindow(1);
	}

	m_editStatusSkimbadRun.Invalidate();
	m_editStatusSkimbadRun.UpdateWindow();

	CDialogEx::OnTimer(nIDEvent);
}


HBRUSH CSkimbadSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATUSSKIMBADRUN)
	{
		if (SkimBadStatus)
		{
			pDC->SetTextColor(RGB(0, 0, 0));
			pDC->SetBkColor(RGB(255, 0, 0));
			hbr = (HBRUSH)(m_pGreenBkBrush->GetSafeHandle());
		}
		else
		{
			pDC->SetTextColor(RGB(0, 0, 0));
			pDC->SetBkColor(RGB(0, 255, 0));
			hbr = (HBRUSH)(m_pRedBkBrush->GetSafeHandle());
		}
	}

	return hbr;
}


void CSkimbadSettingsDlg::OnBnClickedBtnapply()
{
	HKEY hkey;
	LSTATUS stat;
	DWORD created;
	WCHAR buffer[100];

	m_editNumCards.GetWindowTextW(buffer, 100);
	dwNumCards = _wtoi(buffer);

	stat = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Skimbad", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, &created);

	if (stat == ERROR_SUCCESS)
	{
		for (std::vector<CString>::const_iterator it = szPName.begin(); it != szPName.end(); ++it)
		{
			WCHAR szPNameTMP[MAX_PATH];
			StrCpy(szPNameTMP, (*it));
			RegSetValueEx(hkey, L"ProcessName", NULL, REG_SZ, (BYTE *)szPNameTMP, wcslen(szPNameTMP) * sizeof(WCHAR));
		}
		
		RegSetValueEx(hkey, L"NumCards", NULL, REG_DWORD, (BYTE *)&dwNumCards, sizeof(DWORD));
	}
	else
	{
		//Failed to open key try
	}

	RegCloseKey(hkey);
}


void CSkimbadSettingsDlg::OnBnClickedBtnskimstart()
{
	StartSkimbadRun();
}


void CSkimbadSettingsDlg::OnBnClickedBtnskimstop()
{
	StopSkimbadRun();
}


void CSkimbadSettingsDlg::OnBnClickedBtnrestart()
{
	RestartSkimbadRun();
}



