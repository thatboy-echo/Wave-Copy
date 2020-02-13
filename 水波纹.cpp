// 水波纹.cpp : Defines the entry point for the application.
//
/****************************************************
 * 程序名称：实现水波纹显示							*
 * 编译环境：Visual C++ 6.0，EasyX 20130114(beta)	*
 * 作　　者：豪 (QQ: 415051674) 					*
 * 核心算法：参考网上算法							*
 * 最后修改：2013/3/20								*	
 ****************************************************/


#include "E:/Library/VisualStudioHeadFiles/EasyXEffect.h"
#include <conio.h>
#include <stdio.h>


int PIC_HEIGHT=800;
int PIC_WIDTH=1280;

void FrameFun();									// 帧逻辑函数，处理每一帧的逻辑
void RenderFun();									// 帧渲染函数，输出每一帧到显示设备

IMAGE src_img;										// 原位图		
IMAGE dest_img;										// 处理后显示的位图
DWORD *img_ptr1;									// 原图片片内存指针
DWORD *img_ptr2;									// 处理后显示的位图内存指针


// 以下两个 buf 为每一个点的波幅，前者为当前波幅，后者为下一个时刻的波幅。
short *buf ;
short *buf2;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// 初始化设备，加载图片
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

	SetWindowText(Hwnd, "Wave-水波纹效果（点击产生一个水波纹。移动鼠标连续产生水波纹）");

	setbkmode(TRANSPARENT);
	settextcolor(BLACK);
	settextstyle(25, 0, "Arial");

	// 获得内存指针
	img_ptr1 = GetImageBuffer(&src_img);
	img_ptr2 = GetImageBuffer(NULL);

	// 初始化波幅数组

	memset(buf,  0, (PIC_HEIGHT*PIC_WIDTH+PIC_WIDTH) * sizeof(short));
	memset(buf2, 0, (PIC_HEIGHT*PIC_WIDTH+PIC_WIDTH) * sizeof(short));


	// Let's Go!
	BeginBatchDraw();		// 双缓冲，闪屏时需要
	while(true) 
	{
		FrameFun();
		RenderFun();
		FlushBatchDraw();
		Sleep(1);
	}
	EndBatchDraw();
}

// 计算出下一个时刻所有点的波幅
void nextFrame()
{
	for(int i = PIC_WIDTH; i < PIC_HEIGHT*(PIC_WIDTH-1); i++){
		// 公式：X0'= (X1+X2+X3+X4) / 2 - X0
		buf2[i]=((buf[i-PIC_WIDTH] + buf[i+PIC_WIDTH] + buf[i-1] + buf[i+1]) >> 1) - buf2[i];
		buf2[i]-=buf2[i] >> 5;// 波能衰减
	}
	short *ptmp=buf;
	buf =buf2;
	buf2=ptmp;
}
// 处理当前时刻波幅影响之后的位图，保存在 dest_img 中
//根据波动渲染图片
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
				//获得波幅
				Amplitude=1024-AmplitudeList[Ptr];
				// 计算偏移
				X_offset = (((X_offset_new - (PIC_WIDTH >>1)) * Amplitude )>>10) + (PIC_WIDTH >>1);
				Y_offset = (((Y_offset_new - (PIC_HEIGHT>>1)) * Amplitude )>>10) + (PIC_HEIGHT>>1);
				// 边界处理
				if (X_offset >= PIC_WIDTH)		X_offset = PIC_WIDTH - 1;
				if (X_offset < 0)				X_offset = 0;
				if (Y_offset >= PIC_HEIGHT)		Y_offset = PIC_HEIGHT - 1;
				if (Y_offset < 0)				Y_offset = 0;	
				// 处理偏移 
				Target [X_offset_new+Y_offset_new*PIC_WIDTH] = Texture[X_offset + (Y_offset * PIC_WIDTH)];
				// 指针偏移
				Ptr++;
			}
}
void disturb(int x, int y, int stonesize, int stoneweight) 
{
	// 突破边界不处理
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

// 计算fps
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

// 渲染
void RenderFun()
{
	Render(img_ptr1,img_ptr2,buf);

	char s[5];
	sprintf_s(s, "%.1f", getFps());
	outtextxy(0, 0, s);
}

// 逻辑
void FrameFun() 
{
	// 鼠标
	if(MouseHit()){
		MOUSEMSG msg = GetMouseMsg();
		if(msg.uMsg == WM_MOUSEMOVE)
			disturb(msg.x, msg.y, 3, 50); 
		else if(msg.uMsg == WM_LBUTTONDOWN)
			disturb(msg.x, msg.y, 3, 700);
		FlushMouseMsgBuffer();
	}
	// 计算下一帧的波幅
	nextFrame();
}
