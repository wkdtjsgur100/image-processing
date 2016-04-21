
// ImageProcessingDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "ImageProcessing.h"
#include "ImageProcessingDlg.h"
#include "afxdialogex.h"
#include <Vfw.h>

#pragma comment(lib, "vfw32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BM_WIDTH 640
#define BM_HEIGHT 480

void filtering(LPBYTE inputImg, LPBYTE output, double **mask,int mask_length)
{
	int cnt = 0;
	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++)
		{
			double pxValue = 0;
			
			for (int iOffset = -mask_length/2; iOffset <= mask_length/2; iOffset++)
			{
				for (int jOffset = -mask_length/2; jOffset <= mask_length/2; jOffset++)
				{
					int currentCnt = cnt + (BM_WIDTH*iOffset) + jOffset;
					pxValue += (mask[iOffset+mask_length/2][jOffset+mask_length/2] * inputImg[currentCnt]);
				}
			}

			output[cnt++] = pxValue;
		}
	}
}
void laplacianFiltering(LPBYTE grayImg, LPBYTE output)
{
	double** gFilter;

	gFilter = new double*[3];
	for (int i = 0; i < 3; i++)
		gFilter[i] = new double[3];

	gFilter[0][0] = -1; gFilter[0][1] = -1; gFilter[0][2] = -1; 
	gFilter[1][0] = -1; gFilter[1][1] = 8; gFilter[1][2] = -1;
	gFilter[2][0] = -1; gFilter[2][1] = -1; gFilter[2][2] = -1;
	// ����þ� ���͸��� ���� ����ũ

	//����ũ ���� ��!
	filtering(grayImg, output, gFilter, 3);

	for (int i = 0; i < 3; i++)
		delete[] gFilter[i];
	delete[] gFilter;
}
void sobelFiltering(LPBYTE grayImg, LPBYTE output)
{
	int sobelX[9] = {
		-1,0,1,
		-2,0,2,
		-1,0,1 };
	int sobelY[9] = {
		1,2,1,
		0,0,0,
		-1,-2,-1 };
	
	int cnt = 0;
	for (int r = 1; r < BM_HEIGHT - 1; r++)
	{
		for (int c = 1; c < BM_WIDTH - 1; c++)
		{
			int sumX = 0;
			int sumY = 0;
			for (int y = -1; y <= 1; y++)
			{
				for (int x = -1; x <= 1; x++)
				{
					sumX += (sobelX[3 * (y + 1) + x + 1] * grayImg[cnt + BM_WIDTH*y + x]);
					sumY += (sobelY[3 * (y + 1) + x + 1] * grayImg[cnt + BM_WIDTH*y + x]);
				}
			}
			output[cnt++] = sqrt((sumX*sumX + sumY*sumY)/32.0);
		}
	}
}
void gaussianFiltering(LPBYTE grayImg ,LPBYTE output)
{
	double** gFilter;

	gFilter = new double*[5];
	for (int i = 0; i < 5; i++)
		gFilter[i] = new double[5];

	gFilter[0][0] = 2; gFilter[0][1] = 4; gFilter[0][2] = 5;   gFilter[0][3] = 4; gFilter[0][4] = 2;
	gFilter[1][0] = 4; gFilter[1][1] = 9; gFilter[1][2] = 12;  gFilter[1][3] = 9; gFilter[1][4] = 4;
	gFilter[2][0] = 5; gFilter[2][1] = 12; gFilter[2][2] = 15; gFilter[2][3] = 12; gFilter[2][4] = 5;
	gFilter[3][0] = 4; gFilter[3][1] = 9; gFilter[3][2] = 12;  gFilter[3][3] = 9; gFilter[3][4] = 4;
	gFilter[4][0] = 2; gFilter[4][1] = 4; gFilter[4][2] = 5;   gFilter[4][3] = 4; gFilter[4][4] = 2;
	// ����þ� ���͸��� ���� ����ũ

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
			gFilter[i][j] *= 1 / 159.0f;
	} 
	//����ũ ���� ��!
	filtering(grayImg, output, gFilter, 5);

	for (int i = 0; i < 5; i++)
		delete[] gFilter[i];
	delete[] gFilter;
}
void toGray(LPBYTE input, LPBYTE output)
{
	int Jump = 0;
	int cnt = 0;
	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++) {
			BYTE gray = (input[Jump + 0] + input[Jump + 1] + input[Jump + 2]) / 3;

			output[cnt++] = gray;

			Jump += 3;
		}
	}
}

void copyGrayImg(LPBYTE destGray, LPBYTE targetGray)
{
	int cnt = 0;
	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++)
		{
			destGray[cnt] = targetGray[cnt];
			cnt++;
		}
	}

}
LRESULT CALLBACK FramInfo(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	/*
	���� �����ʹ�
	lpVHdr->lpData �� 1���� �迭�� ����Ǿ� �ִ�.
	���⿡ ����ó�� �ڵ带 ������ �ȴ�.

	*/
	LPBYTE grayImg = new BYTE[BM_HEIGHT*BM_WIDTH];
	LPBYTE gaussainFilteredImg = new BYTE[BM_HEIGHT*BM_WIDTH];
	LPBYTE sobelFilteredImg = new BYTE[BM_HEIGHT*BM_WIDTH];

	toGray(lpVHdr->lpData, grayImg);				//������ grayȭ �ؼ� grayImg�� ����

	copyGrayImg(gaussainFilteredImg, grayImg);      //gaussainFilteredImg with grayImg

	gaussianFiltering(grayImg, gaussainFilteredImg);  // graussainFiltering

	sobelFiltering(gaussainFilteredImg, sobelFilteredImg);

	//laplacianFiltering(gaussainFilteredImg, laplacianFilteredImg);

	int Jump = 0;
	int cnt = 0;

	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++)
		{
			lpVHdr->lpData[Jump + 0] = sobelFilteredImg[cnt];
			lpVHdr->lpData[Jump + 1] = sobelFilteredImg[cnt];
			lpVHdr->lpData[Jump + 2] = sobelFilteredImg[cnt];

			cnt++;
			Jump += 3;
		}
	}
	
	delete[] sobelFilteredImg;
	delete[] grayImg;
	delete[] gaussainFilteredImg;

	return (LRESULT)true;
}


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CImageProcessingDlg ��ȭ ����



CImageProcessingDlg::CImageProcessingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMAGEPROCESSING_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CImageProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CImageProcessingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CImageProcessingDlg �޽��� ó����

BOOL CImageProcessingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ���� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	BITMAPINFO BmInfo;

	/////////////////////////////////////////////////���⼭ ���� Vfw
	m_HCam = capCreateCaptureWindow(TEXT("Capture Window"), WS_CHILD | WS_VISIBLE, 0, 0, 640, 480, this->m_hWnd, NULL);

	if (capDriverConnect(m_HCam, 0) == false)
	{
		return false;
	}
	capGetVideoFormat(m_HCam, &BmInfo, sizeof(BITMAPINFO));
	if (BmInfo.bmiHeader.biBitCount != 24)
	{
		BmInfo.bmiHeader.biBitCount = 24;
		BmInfo.bmiHeader.biCompression = 0;
		BmInfo.bmiHeader.biSizeImage = BmInfo.bmiHeader.biWidth*BmInfo.bmiHeader.biHeight * 3;
		capSetVideoFormat(m_HCam, &BmInfo, sizeof(BITMAPINFO));
	}

	if (capSetCallbackOnFrame(this->m_HCam, FramInfo) == false)
		return false;
	capPreviewRate(m_HCam, 10);
	capOverlay(m_HCam, false);
	capPreview(m_HCam, true);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CImageProcessingDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CImageProcessingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void CImageProcessingDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	capDriverDisconnect(m_HCam);
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CImageProcessingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}