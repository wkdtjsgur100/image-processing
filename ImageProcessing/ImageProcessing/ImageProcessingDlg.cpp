
// ImageProcessingDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "ImageProcessing.h"
#include "ImageProcessingDlg.h"
#include "afxdialogex.h"
#include <Vfw.h>
#define _USE_MATH_DEFINES
#include <math.h>

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
					if (currentCnt >= 0 && currentCnt<BM_WIDTH*BM_HEIGHT)
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
	//int cnt=0;
	int cnt = BM_WIDTH+1;
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
					int currentCnt = cnt + BM_WIDTH*y + x;
					sumX += (sobelX[3 * (y + 1) + x + 1] * grayImg[currentCnt]);
					sumY += (sobelY[3 * (y + 1) + x + 1] * grayImg[currentCnt]);
				}
			}
			output[cnt++] = sqrt((sumX*sumX + sumY*sumY)/32.0);
		}
	}
}

void cannyEdge(LPBYTE grayImg, LPBYTE output ,int nThHi ,int nThLo)
{
	int sobelX[9] = {
		-1,0,1,
		-2,0,2,
		-1,0,1 };
	int sobelY[9] = {
		1,2,1,
		0,0,0,
		-1,-2,-1 };
	int cnt = BM_WIDTH+1;
	double* pMag = new double[BM_WIDTH*BM_HEIGHT]; // ��輱�� ����
	LPBYTE pAng = new BYTE[BM_WIDTH*BM_HEIGHT]; // ��輱�� ����
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
			double sum = (sumX*sumX + sumY*sumY);
			pMag[cnt] = sqrt(sum); // �� ��(cnt)������ ��輱�� ���⸦ ���ߴ�

			// ��輱�� ����(����) ���ϱ� <== ��Ȯ���� ��輱�� ���⿡ ������ ������ ���ϴµ�.
			double theta;
			if (pMag[cnt] == 0)
				if(sumY==0)theta=0; // �ٵ� �� ���⼱ ������
				else theta=90;
			else
				theta=atan2((float)sumY, (float)sumX)*180.0/M_PI;

			if ((theta > -22.5 && theta < 22.5) || theta > 157.5 || theta < -157.5)
				pAng[cnt] = 0;

			else if ((theta >= 22.5 && theta < 67.5) || (theta >= -157.5 && theta < -112.5))
				pAng[cnt] = 45;
			
			else if ((theta >= 67.5 && theta <= 112.5) || (theta >= -112.5 && theta <= -67.5))
				pAng[cnt] = 90;
			
			else
				pAng[cnt] = 135;
			
			cnt++;
		}
	}
	// ������ ���Ѱ�(����� ����)���� ���ִ밪 ����
	LPBYTE pCand = new BYTE[BM_WIDTH*BM_HEIGHT]; // ��輱 �ĺ�����
	memset(pCand,0,BM_WIDTH*BM_HEIGHT*(sizeof(BYTE)));
	cnt=BM_WIDTH+1;
	for (int r = 1; r < BM_HEIGHT - 1; r++)
	{
		for (int c = 1; c < BM_WIDTH - 1; c++)
		{
			switch (pAng[cnt])
			{
			case 0: //0�� ���� ��
				if( pMag[cnt] > pMag[cnt-1] && pMag[cnt] > pMag[cnt+1])
					pCand[cnt] = 255;
			case 45: //45�� ���� ��
				if (pMag[cnt] > pMag[cnt - BM_WIDTH + 1] && pMag[cnt] > pMag[cnt + BM_WIDTH - 1])
				{
					pCand[cnt] = 255;
				}
				break;
			case 90: //90�� ���� ��		
				if (pMag[cnt] > pMag[cnt - BM_WIDTH] && pMag[cnt] > pMag[cnt + BM_WIDTH])
				{
					pCand[cnt] = 255;
				}
				break;
			case 135:  //135�� ���� ��
				if (pMag[cnt] > pMag[cnt - BM_WIDTH - 1] && pMag[cnt] > pMag[cnt + BM_WIDTH + 1])
				{
					pCand[cnt] = 255;
				}
				break;
			}
			cnt++;
		}
	}

	// ������ ���Ѱ�(��輱�ĺ�)���� ���ΰ��˻� �� ��� output���� ����
	cnt=BM_WIDTH+1;
	for (int r = 1; r < BM_HEIGHT - 1; r++)
	{
		for (int c = 1; c < BM_WIDTH - 1; c++)
		{
			output[cnt]=0;
			if (pCand[cnt])
			{
				if (pMag[cnt] > nThHi)
				{
					output[cnt] = 255;
				}
				else if (pMag[cnt] > nThLo) // ����� �ȼ� �˻�
				{
					bool bIsEdge = true;
					switch (pAng[cnt])
					{
					case 0:		// 90�� ���� �˻�
						if ((pMag[cnt - BM_WIDTH] > nThHi) ||
							(pMag[cnt + BM_WIDTH] > nThHi))
						{
							output[cnt] = 255;
						}
						break;
					case 45:	// 135�� ���� �˻�
						if ((pMag[cnt - BM_WIDTH - 1] > nThHi) ||
							(pMag[cnt + BM_WIDTH + 1] > nThHi))
						{
							output[cnt] = 255;
						}
						break;
					case 90:		// 0�� ���� �˻�
						if ((pMag[cnt - 1] > nThHi) ||
							(pMag[cnt + 1] > nThHi))
						{
							output[cnt] = 255;
						}
						break;
					case 135:	// 45�� ���� �˻�
						if ((pMag[cnt - BM_WIDTH + 1] > nThHi) ||
							(pMag[cnt + BM_WIDTH - 1] > nThHi))
						{
							output[cnt] = 255;
						}
						break;
					}
				}
			}

			cnt++;
		}
	}

	delete[] pMag;
	delete[] pAng;
	delete[] pCand;
	
}

void HoughLines(LPBYTE imgIn, LPBYTE imgOut, int nTh)
{
	int diagonal = 2 * ((int)sqrt( (float)BM_WIDTH*BM_WIDTH + (float)BM_HEIGHT*BM_HEIGHT )); // �밢���� ����
	int** meetPoints;
	double p2d = 3.141592654 / 180.0;

	meetPoints = new int* [diagonal]; 
	for (int i = 0; i < diagonal; i++)
	{
		meetPoints[i] = new int[180];
		memset(meetPoints[i],0, 180 * sizeof(int));
	}
	double sinLUT[180];
	double cosLUT[180];

	for (int theta = 0; theta < 180; theta++)
	{
		sinLUT[theta]= sin(theta*p2d);
		cosLUT[theta]= cos(theta*p2d);
	}
	int cnt = 0;
	for (int y = 0; y < BM_HEIGHT; y++)
	{
		for (int x = 0; x < BM_WIDTH; x++)
		{
			if (imgIn[cnt] == 255) // cannyEdge()�� ���� ������ �̹������ ����
			{
				for (int theta = 0; theta <180; theta++)
				{
					int rho = (int)(x*cosLUT[theta]+y*sinLUT[theta]);
					if(rho>=0 && rho<diagonal)
						meetPoints[rho][theta]++;
				}
			}
			cnt++;
		}
	}

	for (int rho = 0; rho < diagonal; rho++)
	{
		for (int theta = 0; theta <180; theta++)
		{
			if (meetPoints[rho][theta] > nTh) // threshold �˻�
			{
				cnt = 0;

				int x, _y;
				for(int y = 0; y < BM_HEIGHT-1; y++)
				{
					if (theta == 0)
					{
						x = rho;
					}
					else if (theta == 90)
					{
						x = 0;
					}
					else
					{
						x = (int)((rho - y*sinLUT[theta]) / cosLUT[theta]);
					}

					if (x < BM_WIDTH && x >= 0)
						imgIn[y*BM_WIDTH + x] = 127;
				}
				for (int _x = 0; _x < BM_WIDTH; _x++)
				{
					if (theta == 0)
					{
						_y = rho;
					}
					else if (theta == 90)
					{
						_y = 0;
					}
					else
					{
						_y = (int)((rho - _x*cosLUT[theta]) / sinLUT[theta]);
					}

					if (_y < BM_HEIGHT-1 && _y >= 0)
						imgIn[_y*BM_WIDTH + _x] = 127;
				}
			}
		}
	}
	
	for (int i = 0; i < diagonal; i++)
		delete [] meetPoints[i];
	delete [] meetPoints;

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
//void copyGrayImg(LPBYTE destGray, LPBYTE targetGray)
//{
//	int cnt = 0;
//	for (int i = 0; i < BM_HEIGHT; i++)
//	{
//		for (int j = 0; j < BM_WIDTH; j++)
//		{
//			destGray[cnt] = targetGray[cnt];
//			cnt++;
//		}
//	}
//}

LRESULT CALLBACK FramInfo(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	/*
	���� �����ʹ�
	lpVHdr->lpData �� 1���� �迭�� ����Ǿ� �ִ�.
	���⿡ ����ó�� �ڵ带 ������ �ȴ�.

	*/
	
	LPBYTE grayImg = new BYTE[BM_HEIGHT*BM_WIDTH];
	LPBYTE gaussainFilteredImg = new BYTE[BM_HEIGHT*BM_WIDTH];
	LPBYTE filteredImg = new BYTE[BM_HEIGHT*BM_WIDTH];
	LPBYTE houghImg = new BYTE[BM_HEIGHT*BM_WIDTH];

	memset(grayImg, 0, sizeof(BYTE)*BM_WIDTH*BM_HEIGHT);

	//toGray(lpVHdr->lpData, grayImg);				//������ grayȭ �ؼ� grayImg�� ����

	//copyGrayImg(gaussainFilteredImg, grayImg);      //gaussainFilteredImg with grayImg

	//gaussianFiltering(grayImg, gaussainFilteredImg);  // graussainFiltering
	//cannyEdge(gaussainFilteredImg,filteredImg,50,20);
	//HoughLines(filteredImg,houghImg,120);
	//sobelFiltering(gaussainFilteredImg, filteredImg);

	//laplacianFiltering(gaussainFilteredImg, laplacianFilteredImg);

	int Jump = 0;
	int cnt = 0;
	
	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++)
		{
			/*
			if(houghImg[cnt] != 0)
			{
				lpVHdr->lpData[Jump + 0] = 255;
				lpVHdr->lpData[Jump + 1] = 0;
				lpVHdr->lpData[Jump + 2] = 0;
			}
			*/
			BYTE R = lpVHdr->lpData[Jump + 2];
			BYTE G = lpVHdr->lpData[Jump + 1];
			BYTE B = lpVHdr->lpData[Jump + 0];
			double _max, _min;

			_max = max(R,G);
			_max = max(_max, B);
			_min = min(R, G);
			_min = min(_min, B);

			//BYTE H = (BYTE)(acos( (R - 0.5*G - 0.5*B)/sqrt(R*R + G*G + B*B - R*G - R*B - G*B)));
			double H;
			double V = _max;
			double S = (_max != 0.0) ? (_max - _min) / _max : 0.0;

			if (S == 0.0)
				H = 0;
			else
			{
				double delta = _max - _min;
				if (R == _max) 
					H = (G - B) / delta;
				else if (G == _max)
					H = 2.0 + (B - R) / delta;
				else if (B == _max)
					H = 4.0 + (R - G) / delta;
				H *= 60.0;
				if (H < 0.0) 
					H += 360.0;

			}
			/*if (G < B)
				H = 360 - H;*/

			if ( H>180 && H<220 && S>0.2 && V>50)
			{
				grayImg[cnt] = 255;
			}
			else
			{
				grayImg[cnt] = 0;
			}
			//lpVHdr->lpData[Jump + 0] = filteredImg[cnt];
			//lpVHdr->lpData[Jump + 1] = filteredImg[cnt];
			//lpVHdr->lpData[Jump + 2] = filteredImg[cnt];
			
			cnt++;
			Jump += 3;
		}
	}

	//gaussianFiltering(grayImg, gaussainFilteredImg);
	cnt = 0;
	Jump = 0;
	
	BYTE copyImg[BM_HEIGHT][BM_WIDTH];

	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++)
		{
			copyImg[i][j] = grayImg[cnt];
			copyImg[i][j] = grayImg[cnt];
			copyImg[i][j] = grayImg[cnt];
			
			cnt++;
			Jump += 3;
		}
	}

	for (int i = 0; i < BM_HEIGHT; i += 10)
	{
		for (int j = 0; j < BM_WIDTH; j+=10)
		{
			for (int s_y = 0; s_y< 10; s_y++)
			{
				for (int s_x = 0; s_x < 10; s_x++)
				{
					copyImg[i + s_y][j + s_x] = copyImg[i][j];
				}
			}
		}
	}
	cnt = 0;
	Jump = 0;

	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++)
		{
			lpVHdr->lpData[Jump + 2] = copyImg[i][j];
			lpVHdr->lpData[Jump + 1] = copyImg[i][j];
			lpVHdr->lpData[Jump + 0] = copyImg[i][j];
			
			Jump += 3;
		}
	}
	
	delete[] grayImg;
	delete[] gaussainFilteredImg;
	delete[] filteredImg;
	delete[] houghImg;

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

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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