
// ImageProcessing.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CImageProcessingApp:
// �� Ŭ������ ������ ���ؼ��� ImageProcessing.cpp�� �����Ͻʽÿ�.
//

class CImageProcessingApp : public CWinApp
{
public:
	CImageProcessingApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CImageProcessingApp theApp;