
// ImageProcessingDlg.cpp : 구현 파일
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
	// 가우시안 필터링을 위한 마스크

	//마스크 생성 끝!
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
	double* pMag = new double[BM_WIDTH*BM_HEIGHT]; // 경계선의 세기
	LPBYTE pAng = new BYTE[BM_WIDTH*BM_HEIGHT]; // 경계선의 방향
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
			pMag[cnt] = sqrt(sum); // 한 점(cnt)에서의 경계선의 세기를 구했다

			// 경계선의 방향(각도) 구하기 <== 정확히는 경계선의 방향에 수직인 각도를 구하는듯.
			double theta;
			if (pMag[cnt] == 0)
				if(sumY==0)theta=0; // 근데 왜 여기선 이지랄
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
	// 위에서 구한값(세기와 방향)으로 비최대값 억제
	LPBYTE pCand = new BYTE[BM_WIDTH*BM_HEIGHT]; // 경계선 후보저장
	memset(pCand,0,BM_WIDTH*BM_HEIGHT*(sizeof(BYTE)));
	cnt=BM_WIDTH+1;
	for (int r = 1; r < BM_HEIGHT - 1; r++)
	{
		for (int c = 1; c < BM_WIDTH - 1; c++)
		{
			switch (pAng[cnt])
			{
			case 0: //0도 방향 비교
				if( pMag[cnt] > pMag[cnt-1] && pMag[cnt] > pMag[cnt+1])
					pCand[cnt] = 255;
			case 45: //45도 방향 비교
				if (pMag[cnt] > pMag[cnt - BM_WIDTH + 1] && pMag[cnt] > pMag[cnt + BM_WIDTH - 1])
				{
					pCand[cnt] = 255;
				}
				break;
			case 90: //90도 방향 비교		
				if (pMag[cnt] > pMag[cnt - BM_WIDTH] && pMag[cnt] > pMag[cnt + BM_WIDTH])
				{
					pCand[cnt] = 255;
				}
				break;
			case 135:  //135도 방향 비교
				if (pMag[cnt] > pMag[cnt - BM_WIDTH - 1] && pMag[cnt] > pMag[cnt + BM_WIDTH + 1])
				{
					pCand[cnt] = 255;
				}
				break;
			}
			cnt++;
		}
	}

	// 위에서 구한값(경계선후보)으로 문턱값검사 후 결과 output으로 저장
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
				else if (pMag[cnt] > nThLo) // 연결된 픽셀 검사
				{
					bool bIsEdge = true;
					switch (pAng[cnt])
					{
					case 0:		// 90도 방향 검사
						if ((pMag[cnt - BM_WIDTH] > nThHi) ||
							(pMag[cnt + BM_WIDTH] > nThHi))
						{
							output[cnt] = 255;
						}
						break;
					case 45:	// 135도 방향 검사
						if ((pMag[cnt - BM_WIDTH - 1] > nThHi) ||
							(pMag[cnt + BM_WIDTH + 1] > nThHi))
						{
							output[cnt] = 255;
						}
						break;
					case 90:		// 0도 방향 검사
						if ((pMag[cnt - 1] > nThHi) ||
							(pMag[cnt + 1] > nThHi))
						{
							output[cnt] = 255;
						}
						break;
					case 135:	// 45도 방향 검사
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

void HoughLines(LPBYTE imgIn, int nTh)
{
	int diagonal = (int)sqrt( (float)BM_WIDTH*BM_WIDTH + (float)BM_HEIGHT*BM_HEIGHT ); // 대각선의 길이
	int** meetPoints;
	meetPoints = new int* [diagonal]; 
	for (int i = 0; i < diagonal; i++)
	{
		meetPoints[i] = new int[180];
		memset(meetPoints[i],0, 180 * sizeof(int));
	}
	double sinLUT[180];
	double cosLUT[180];

	for (int theta = 0; theta <= 180; theta++)
	{
		sinLUT[theta]=sin(theta*M_PI/180.0);
		cosLUT[theta]=cos(theta*M_PI/180.0);
	}

	for (int r = 1; r < BM_HEIGHT - 1; r++)
	{
		for (int c = 1; c < BM_WIDTH - 1; c++)
		{
			if (imgIn[r*BM_WIDTH+c] == 255) // cannyEdge()를 먼저 실행한 이미지라고 가정
			{
				for (int theta = 0; theta <=180; theta++)
				{
					int rho = c*cosLUT[theta]+r*sinLUT[theta];
					if(rho>=0 && rho<=diagonal)
						meetPoints[rho][theta]++;
				}
			}
		}
	}

	for (int rho = 0; rho <= diagonal; rho++)
	{
		for (int theta = 0; theta <= 180; theta++)
		{
			if (meetPoints[rho][theta] > nTh) // ThresHold 검사
			{
				for (int r = 1; r < BM_HEIGHT - 1; r++)
					for (int c = 1; c < BM_WIDTH - 1; c++)
						if(c*cosLUT[theta]+r*sinLUT[theta]==rho)
							imgIn[r*BM_WIDTH+c]=128;
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
	// 가우시안 필터링을 위한 마스크

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
			gFilter[i][j] *= 1 / 159.0f;
	} 
	//마스크 생성 끝!
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
	영상 데이터는
	lpVHdr->lpData 에 1차원 배열로 저장되어 있다.
	여기에 영상처리 코드를 넣으면 된다.

	*/
	
	LPBYTE grayImg = new BYTE[BM_HEIGHT*BM_WIDTH];
	LPBYTE gaussainFilteredImg = new BYTE[BM_HEIGHT*BM_WIDTH];
	LPBYTE filteredImg = new BYTE[BM_HEIGHT*BM_WIDTH];

	toGray(lpVHdr->lpData, grayImg);				//영상을 gray화 해서 grayImg에 저장

	//copyGrayImg(gaussainFilteredImg, grayImg);      //gaussainFilteredImg with grayImg

	gaussianFiltering(grayImg, gaussainFilteredImg);  // graussainFiltering
	cannyEdge(grayImg,filteredImg,60,30);
	//HoughLines(filteredImg,10);
	//sobelFiltering(gaussainFilteredImg, filteredImg);

	//laplacianFiltering(gaussainFilteredImg, laplacianFilteredImg);

	int Jump = 0;
	int cnt = 0;
	
	for (int i = 0; i < BM_HEIGHT; i++)
	{
		for (int j = 0; j < BM_WIDTH; j++)
		{
			//if (filteredImg[cnt] == 127)//허프 직선~~~ 시간업어서 이렇게짬~~
			//{
			//	lpVHdr->lpData[Jump + 0] = 0;
			//	lpVHdr->lpData[Jump + 1] = 0;
			//	lpVHdr->lpData[Jump + 2] = 255;
			//}
			//else
			//{
				lpVHdr->lpData[Jump + 0] = filteredImg[cnt];
				lpVHdr->lpData[Jump + 1] = filteredImg[cnt];
				lpVHdr->lpData[Jump + 2] = filteredImg[cnt];
			//}

			cnt++;
			Jump += 3;
		}
	}
	
	
	delete[] grayImg;
	delete[] gaussainFilteredImg;
	delete[] filteredImg;

	return (LRESULT)true;
}


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CImageProcessingDlg 대화 상자



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


// CImageProcessingDlg 메시지 처리기

BOOL CImageProcessingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	BITMAPINFO BmInfo;

	/////////////////////////////////////////////////여기서 부터 Vfw
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

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CImageProcessingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
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

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CImageProcessingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}