#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include "subwindow.h"
#include <tchar.h>
#include <iostream>   
#include <atlstr.h>
using   namespace   std;

//int main(void)
//{
//	int i;
//	char strDir[256] ;
//	FILE *finput=fopen("code.txt","r"); 
//	fscanf(finput, "%s", strDir) ;
//	fclose(finput) ;
//	srand((unsigned)time(NULL)); 
//
//	char path1[256], file1[256];
//	sprintf(path1,"%s","G:\\Ideas\\DSCM-CAU\\test\\");
//	sprintf(file1,"%s","G:\\Ideas\\DSCM-CAU\\test\\ParallelEPCA.exe");
//	Initial_rand(1);
//	for (i=0;i<10;i++)
//		cout<<Get_rand()<<endl;
//
//	for (i = 0 ; i<2; i++)
//	{
//		printf("%d app running ",i+1);
//		openAPP(path1,file1);
//
//
//	}
//
//	_sleep(2*1000);
//	char* szStr = file1;	 
//	CString str = CString(szStr);
//	USES_CONVERSION;
//	LPCWSTR wszClassName = A2CW(W2A(str));
//	str.ReleaseBuffer();
//
//
//	HWND hWndRcv = ::FindWindow(NULL,wszClassName);
//	if (FindProcessHandleAndKill(hWndRcv))
//		return TRUE;
//	return FALSE;
//
//	while (hWndRcv!= NULL )
//	{
//		hWndRcv = ::FindWindowEx(NULL,NULL,NULL,wszClassName);
//		if (FindProcessHandleAndKill(hWndRcv))
//			return TRUE;
//		return FALSE;
//
//	}
//	
//	//HWND hWndRcv = ::FindWindow(NULL,TEXT("G:\\Ideas\\DSCM-CAU\\test\\ParallelEPCA.exe"));
//	printf("%x\n",hWndRcv);
//	//HWND hWndRcv = ::FindWindowEx(NULL,NULL,TEXT("ParallelEPCA"),NULL);
//	//HWND hWndRcv = ::FindWindowEx(NULL,NULL,L"ParallelEPCA",NULL);
//	//HWND hWnd = ::GetForegroundWindow();
//	/*if (FindProcessHandleAndKill(hWndRcv))
//	return TRUE;
//	return FALSE;*/
//}

int main(void)
{

	cout<<"*----------------------------------------------------------------------------------------------*"<<endl;
	cout<<"*------    Combined Displacement Field Extration and Cellular Automata Inverse Method    ------*"<<endl;
	cout<<"*------                              DFE-CA Inverse Method v1.0                          ------*"<<endl;
	cout<<"*------                             Copyright: IRSM, CAS; HENU                           ------*"<<endl;
	cout<<"*------                               Developer: Zhaofeng Wang                           ------*"<<endl;
	cout<<"*----------------------------------------------------------------------------------------------*"<<endl;
	//printf("\n");
	//开发日志 Developing Log
	//2022.12.5：重改了main，完成SAPSO_main主体
	int i;
	char workingDir[256], commandPath[256];
	int particleN[1];
	double learningCof[4];
	double SimuACof[2];
	int maxIter[1];
	int inverseD[1];
	int *paraLocation;
	int randSeed[1];
	double *inverseUp;
	double *inverseDn;
	double epX[1];
	int monitorN[1];
	int monitorT[1];
	double *monitorX;
	double *monitorY;
	double *monitorV;
	double *inversePara;
	double misFit[1];
	int parallelN[1];
	int sectionN[1];
	int *sectionOrder;

	FILE *finput=fopen("code.txt","r"); 
	fscanf(finput, "%s", &workingDir) ;
	fclose(finput) ;

	//读取参数，从command.txt里面
	sprintf(commandPath,"%scommand.txt",workingDir);
	FILE *commandFile=fopen(commandPath,"r"); 
	printf("Reading parameters.");

	fscanf(commandFile, "%d", &particleN[0]) ;
	printf("..");
	fscanf(commandFile, "%lf %lf %lf %lf", &learningCof[0], &learningCof[1], &learningCof[2], &learningCof[3]) ;
	printf("..");
	fscanf(commandFile, "%lf %lf", &SimuACof[0], &SimuACof[1]);
	printf("..");
	fscanf(commandFile, "%d", &maxIter[0]) ;
	printf("..");
	fscanf(commandFile, "%d", &inverseD[0]) ;
	printf("..");

	paraLocation = new int [2*inverseD[0]];

	for (i = 0; i< inverseD[0];i++)
	{
		fscanf(commandFile, "%d %d", &paraLocation[2*i],&paraLocation[2*i+1]) ;
	}
	printf("..");
	fscanf(commandFile, "%d", &randSeed[0]) ;
	printf("..");
	inverseUp = new double [2*inverseD[0]];
	inverseDn = new double [2*inverseD[0]];

	for (i = 0; i< inverseD[0];i++)
	{
		fscanf(commandFile, "%lf %lf", &inverseUp[2*i],&inverseUp[2*i+1]) ;
	}
	printf("..");
	for (i = 0; i< inverseD[0];i++)
	{
		fscanf(commandFile, "%lf %lf", &inverseDn[2*i],&inverseDn[2*i+1]) ;
	}
	printf("..");
	fscanf(commandFile, "%lf", &epX[0]) ;
	fscanf(commandFile, "%d", &monitorN[0]) ;
	fscanf(commandFile, "%d", &monitorT[0]) ;
	printf("..");
	monitorX = new double [monitorN[0]*2];
	monitorY = new double [monitorN[0]*2];
	monitorV = new double [monitorN[0]*2];

	if (monitorT[0] == 0)
	{
		for (i = 0; i < monitorN[0];i++)
		{
			fscanf(commandFile, "%lf %lf %lf %lf", &monitorX[i],&monitorY[i],&monitorV[i],&monitorV[i+monitorN[0]]) ;
		}
	}
	else
	{
		for (i = 0; i < monitorN[0];i++)
		{
			fscanf(commandFile, "%lf %lf %lf", &monitorX[i],&monitorY[i],&monitorV[i]) ;
		}
	}
	printf("..");

	fscanf(commandFile, "%d", &parallelN[0]) ;
	fscanf(commandFile, "%d", &sectionN[0]);

	sectionOrder = new int [sectionN[0]];

	for (i = 0; i< sectionN[0];i++)
	{
		fscanf(commandFile, "%d", &sectionOrder[i]);
	}


	printf("\n");
	fclose(commandFile) ;

	inversePara = new double [inverseD[0]];
	SAPSO_main1(particleN,learningCof,SimuACof, maxIter,inverseD,paraLocation, randSeed,inverseUp,inverseDn,epX, monitorN,monitorX,monitorY,monitorV,inversePara, misFit, workingDir,parallelN, sectionN,sectionOrder,monitorT);


	delete []paraLocation;
	delete []inverseUp;
	delete []inverseDn;
	delete []monitorX;
	delete []monitorY;
	delete []monitorV;
	delete []inversePara;
	delete []sectionOrder;
}