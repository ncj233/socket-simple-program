
// MFCClientDlg.h : 头文件
//

#pragma once
#include "Client.h"
#include "afxwin.h"

// CMFCClientDlg 对话框
class CMFCClientDlg : public CDialogEx
{
// 构造
public:
	CMFCClientDlg(CWnd* pParent = NULL);	// 标准构造函数
	void clear_list();
	void list_add(const char *s);
	void add_text(const char *s);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CMenu m_menu;
	Client client;

	// 生成的消息映射函数
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
