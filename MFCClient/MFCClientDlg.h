
// MFCClientDlg.h : ͷ�ļ�
//

#pragma once
#include "Client.h"
#include "afxwin.h"

// CMFCClientDlg �Ի���
class CMFCClientDlg : public CDialogEx
{
// ����
public:
	CMFCClientDlg(CWnd* pParent = NULL);	// ��׼���캯��
	void clear_list();
	void list_add(const char *s);
	void add_text(const char *s);

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	CMenu m_menu;
	Client client;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnConnect();
	afx_msg void OnExit();
	CString m_input_ip;
	afx_msg void OnMenuConnect();
	void disconnect();
	afx_msg void OnDisconnect();
	CListBox list;
	afx_msg void OnList();
	afx_msg void OnLbnSelchangeList3();
	afx_msg void OnSend();
	CEdit text;
	afx_msg void OnTime();
	afx_msg void OnGetName();
	CEdit mesg_send;
};
