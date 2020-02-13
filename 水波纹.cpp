// ˮ����.cpp : Defines the entry point for the application.
//
/****************************************************
 * �������ƣ�ʵ��ˮ������ʾ							*
 * ���뻷����Visual C++ 6.0��EasyX 20130114(beta)	*
 * �������ߣ��� (QQ: 415051674) 					*
 * �����㷨���ο������㷨							*
 * ����޸ģ�2013/3/20								*	
 ****************************************************/


#include "E:/Library/VisualStudioHeadFiles/EasyXEffect.h"
#include <conio.h>
#include <stdio.h>


int PIC_HEIGHT=800;
int PIC_WIDTH=1280;

void FrameFun();									// ֡�߼�����������ÿһ֡���߼�
void RenderFun();									// ֡��Ⱦ���������ÿһ֡����ʾ�豸

IMAGE src_img;										// ԭλͼ		
IMAGE dest_img;										// �������ʾ��λͼ
DWORD *img_ptr1;									// ԭͼƬƬ�ڴ�ָ��
DWORD *img_ptr2;									// �������ʾ��λͼ�ڴ�ָ��


// �������� buf Ϊÿһ����Ĳ�����ǰ��Ϊ��ǰ����������Ϊ��һ��ʱ�̵Ĳ�����
short *buf ;
short *buf2;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// ��ʼ���豸������ͼƬ
	CaptureDesktop(&src_img);
	PIC_WIDTH =GetSystemMetrics(SM_CXSCREEN);
	PIC_HEIGHT=GetSystemMetrics(SM_CYSCREEN);

    HWND Hwnd=initgraph(PIC_WIDTH, PIC_HEIGHT); 
	Resize(&src_img,PIC_WIDTH, PIC_HEIGHT); 
	Resize(&dest_img,PIC_WIDTH, PIC_HEIGHT); 
	buf  = new short[PIC_HEIGHT*PIC_WIDTH+PIC_WIDTH];
	buf2 = new short[PIC_HEIGHT*PIC_WIDTH+PIC_WIDTH];


	SetWindowLong(Hwnd,GWL_STYLE,GetWindowLong(Hwnd,GWL_STYLE)-WS_CAPTION);
	SetWindowPos(Hwnd,HWND_TOP,0 ,0 ,PIC_WIDTH,PIC_HEIGHT,SWP_SHOWWINDOW);

	SetWindowText(Hwnd, "Wave-ˮ����Ч�����������һ��ˮ���ơ��ƶ������������ˮ���ƣ�");

	setbkmode(TRANSPARENT);
	settextcolor(BLACK);
	settextstyle(25, 0, "Arial");

	// ����ڴ�ָ��
	img_ptr1 = GetImageBuffer(&src_img);
	img_ptr2 = GetImageBuffer(NULL);

	// ��ʼ����������

	memset(buf,  0, (PIC_HEIGHT*PIC_WIDTH+PIC_WIDTH) * sizeof(short));
	memset(buf2, 0, (PIC_HEIGHT*PIC_WIDTH+PIC_WIDTH) * sizeof(short));


	// Let's Go!
	BeginBatchDraw();		// ˫���壬����ʱ��Ҫ
	while(true) 
	{
		FrameFun();
		RenderFun();
		FlushBatchDraw();
		Sleep(1);
	}
	EndBatchDraw();
}

// �������һ��ʱ�����е�Ĳ���
void nextFrame()
{
	for(int i = PIC_WIDTH; i < PIC_HEIGHT*(PIC_WIDTH-1); i++){
		// ��ʽ��X0'= (X1+X2+X3+X4) / 2 - X0
		buf2[i]=((buf[i-PIC_WIDTH] + buf[i+PIC_WIDTH] + buf[i-1] + buf[i+1]) >> 1) - buf2[i];
		buf2[i]-=buf2[i] >> 5;// ����˥��
	}
	short *ptmp=buf;
	buf =buf2;
	buf2=ptmp;
}
// ����ǰʱ�̲���Ӱ��֮���λͼ�������� dest_img ��
//���ݲ�����ȾͼƬ
void Render(DWORD*Texture,DWORD*Target,short*AmplitudeList)
{
	int Ptr = 0;
	short Amplitude;
	int X_offset;
	int Y_offset;
	int X_offset_new;
	int Y_offset_new;
	for (Y_offset_new = 0; Y_offset_new < PIC_HEIGHT; Y_offset_new++) 
			for (X_offset_new = 0; X_offset_new < PIC_WIDTH; X_offset_new++) 
			{
				//��ò���
				Amplitude=1024-AmplitudeList[Ptr];
				// ����ƫ��
				X_offset = (((X_offset_new - (PIC_WIDTH >>1)) * Amplitude )>>10) + (PIC_WIDTH >>1);
				Y_offset = (((Y_offset_new - (PIC_HEIGHT>>1)) * Amplitude )>>10) + (PIC_HEIGHT>>1);
				// �߽紦��
				if (X_offset >= PIC_WIDTH)		X_offset = PIC_WIDTH - 1;
				if (X_offset < 0)				X_offset = 0;
				if (Y_offset >= PIC_HEIGHT)		Y_offset = PIC_HEIGHT - 1;
				if (Y_offset < 0)				Y_offset = 0;	
				// ����ƫ�� 
				Target [X_offset_new+Y_offset_new*PIC_WIDTH] = Texture[X_offset + (Y_offset * PIC_WIDTH)];
				// ָ��ƫ��
				Ptr++;
			}
}
void disturb(int x, int y, int stonesize, int stoneweight) 
{
	// ͻ�Ʊ߽粻����
	if ((x >= PIC_WIDTH - stonesize) ||
		(x < stonesize) ||
		(y >= PIC_HEIGHT - stonesize) ||
		(y < stonesize))
		return;
	for (int posx=x-stonesize; posx<x+stonesize; posx++)
		for (int posy=y-stonesize; posy<y+stonesize; posy++)
			if ((posx-x)*(posx-x) + (posy-y)*(posy-y) < stonesize*stonesize)
				buf[PIC_WIDTH*posy+posx] += stoneweight;
}

// ����fps
float getFps()
{
#define FPS_COUNT 8
	static int i = 0;
	static DWORD oldTime = GetTickCount();
	static float fps;

	if (i > FPS_COUNT)
	{
		i = 0;
		int newTime = GetTickCount();
		int elapsedTime = newTime - oldTime;
		fps = FPS_COUNT / (elapsedTime / 1000.0f);
		oldTime = newTime;
	}
	i++;
	return fps;
}

// ��Ⱦ
void RenderFun()
{
	Render(img_ptr1,img_ptr2,buf);

	char s[5];
	sprintf_s(s, "%.1f", getFps());
	outtextxy(0, 0, s);
}

// �߼�
void FrameFun() 
{
	// ���
	if(MouseHit()){
		MOUSEMSG msg = GetMouseMsg();
		if(msg.uMsg == WM_MOUSEMOVE)
			disturb(msg.x, msg.y, 3, 50); 
		else if(msg.uMsg == WM_LBUTTONDOWN)
			disturb(msg.x, msg.y, 3, 700);
		FlushMouseMsgBuffer();
	}
	// ������һ֡�Ĳ���
	nextFrame();
}
