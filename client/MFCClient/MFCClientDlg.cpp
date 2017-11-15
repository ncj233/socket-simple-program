
// MFCClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include <string>
#include "MFCClient.h"
#include "MFCClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDisconnect();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_COMMAND(ID_DISCONNECT, &CAboutDlg::OnDisconnect)
END_MESSAGE_MAP()


// CMFCClientDlg �Ի���



CMFCClientDlg::CMFCClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCCLIENT_DIALOG, pParent)
	, m_input_ip(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCClientDlg::clear_list()
{
	list.ResetContent();
}

void CMFCClientDlg::list_add(const char *s)
{
	list.AddString(CString(s));
}

void CMFCClientDlg::add_text(const char * s)
{
	CString txt;

	text.GetWindowTextW(txt);
	text.SetWindowTextW(txt + CString(s) + CString("\r\n"));
	int cnt = text.GetLineCount();
	text.LineScroll(cnt, 0);
}

void CMFCClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IP, m_input_ip);
	DDV_MaxChars(pDX, m_input_ip, 20);
	DDX_Control(pDX, IDC_LIST3, list);
	DDX_Control(pDX, IDC_EDIT6, text);
	DDX_Control(pDX, IDC_EDIT1, mesg_send);
}

BEGIN_MESSAGE_MAP(CMFCClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMFCClientDlg::OnBnClickedOk)
	ON_COMMAND(CONNECT, &CMFCClientDlg::OnConnect)
	ON_COMMAND(ID_EXIT, &CMFCClientDlg::OnExit)
	ON_COMMAND(ID_CONNECT, &CMFCClientDlg::OnMenuConnect)
	ON_COMMAND(ID_DISCONNECT, &CMFCClientDlg::OnDisconnect)
	ON_COMMAND(GET_LIST, &CMFCClientDlg::OnList)
	ON_LBN_SELCHANGE(IDC_LIST3, &CMFCClientDlg::OnLbnSelchangeList3)
	ON_COMMAND(SEND, &CMFCClientDlg::OnSend)
	ON_COMMAND(GET_TIME, &CMFCClientDlg::OnTime)
	ON_COMMAND(GET_NAME, &CMFCClientDlg::OnGetName)
END_MESSAGE_MAP()


// CMFCClientDlg ��Ϣ�������

BOOL CMFCClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	client.ui = this;
	m_menu.LoadMenu(IDR_MENU1);
	SetMenu(&m_menu);
	GetDlgItem(PORT)->SetWindowTextW(CString("5015"));
	((CEdit *)GetDlgItem(PORT))->SetReadOnly(TRUE);
	//���õ��˵���һЩ����
	GetMenu()->GetSubMenu(0)->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(2)->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMFCClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFCClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMFCClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCClientDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CMFCClientDlg::OnConnect()
{
	// TODO: Add your command handler code here
}


void CMFCClientDlg::OnExit()
{
	// TODO: Add your command handler code here
	client.myDisconnect();
	CDialogEx::OnOK();
}

void CMFCClientDlg::OnMenuConnect()
{
	// TODO: Add your command handler code here
	UpdateData(TRUE);
	char str_ip[101];
	for (int i = 0; i < m_input_ip.GetLength() && i < 100; i++) {
		str_ip[i] = (char)m_input_ip.GetAt(i);
	}
	str_ip[m_input_ip.GetLength()] = 0;
	try {
		client.start(str_ip);
		MessageBox(CString("���ӳɹ�"), CString("�ͻ���"));
		((CEdit *)GetDlgItem(IP))->SetReadOnly(TRUE);
		GetMenu()->GetSubMenu(0)->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		GetMenu()->GetSubMenu(0)->EnableMenuItem(1, MF_BYPOSITION | MF_ENABLED);
		GetMenu()->GetSubMenu(1)->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
		GetMenu()->GetSubMenu(1)->EnableMenuItem(1, MF_BYPOSITION | MF_ENABLED);
		GetMenu()->GetSubMenu(1)->EnableMenuItem(2, MF_BYPOSITION | MF_ENABLED);
		GetMenu()->GetSubMenu(2)->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
		add_text("�����Ѵ���");
	}
	catch (SocketException e){
		MessageBox(CString(e.getErrorType().c_str()), CString("����"));
	}

}

void CMFCClientDlg::disconnect()
{
	MessageBox(CString("�����ѶϿ�"), CString("��Ϣ"));
	((CEdit *)GetDlgItem(IP))->SetReadOnly(FALSE);
	list.ResetContent();
	add_text("�����ѶϿ�");
	GetMenu()->GetSubMenu(0)->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
	GetMenu()->GetSubMenu(0)->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	GetMenu()->GetSubMenu(2)->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
}

void CMFCClientDlg::OnDisconnect()
{
	// ����disconnectֵ����true����Ϊ����˳�close socket���ǻᵼ���ӽ��̵ı���
	client.myDisconnect();
}

void CAboutDlg::OnDisconnect()
{

}

void CMFCClientDlg::OnTime()
{
	// ��ȡʱ�������
	client.mysend(0);
}

void CMFCClientDlg::OnGetName()
{
	// �������������
	client.mysend(1);
}


void CMFCClientDlg::OnList()
{
	// ��ȡ�б������
	client.mysend(2);
}

void CMFCClientDlg::OnSend()
{
	// TODO: Add your command handler code here
	int usrid = list.GetCurSel();

	if (usrid == -1) {
		MessageBox(CString("û��ѡ���û�"), CString("��ʾ"));
		return;
	}
	
	std::string message;
	CString str_get;
	mesg_send.GetWindowTextW(str_get);

	for (int i = 0; i < str_get.GetLength(); i++) {
		short k = str_get.GetAt(i);
		if (!(k & 0xFF00)) {
			message = message + (char)k;
		}
	}

	client.mysend(3, usrid, message.c_str());
}


void CMFCClientDlg::OnLbnSelchangeList3()
{
	// nothing: don't use this function
}
