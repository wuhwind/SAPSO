#if !defined(AFX_SUBWINDOW_H__2335308A_7D00_4A30_930A_8FE13C6671A7__INCLUDED_)
#define AFX_SUBWINDOW_H__2335308A_7D00_4A30_930A_8FE13C6671A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>

#include<iostream>
#include <cstdlib>
#include <time.h>
#include <random>

#include <io.h>
#include <direct.h>
#include <string>
#include<fstream>
#include <tchar.h>


 
#include <atlstr.h>
#include "File.h"
using namespace std;
//#define NNN 32760

int Open_app(char path[256],char file[256]);
BOOL FindandKill_process(HWND hWnd);
void SAPSO_main1(int particleN[1],double learningCof[4],double SimuACof[2],int maxIter[1],int inverseD[1],int *paraLocation,int randSeed[1],double *inverseUp,double *inverseDn,double epX[1],int monitorN[1],double *monitorX,double *monitorY,double *monitorV,double *inversePara,double misFit[1],char workingPath[256],int parallelN[1],int sectionN[1],int *sectionOrder,int monitorT[1]);
double Get_rand();
void Initial_rand(int index);
void Create_dir(char workingPath[256],int iter,int parallelN[1]);
void Assign_para(char workingPath[256],int iter,int parallelN[1],int inverseD[1],int *paraLocation,double **x,int materialN,double **materialV, int sectionN[1],int *sectionOrder);
void Cal_parallel(char workingPath[256],int iter,int parallelN[1]);
void Extract_data(char workingPath[256],int iter,int parallelN[1],int monitorT[1],int monitorN[1],double *monitorX,double *monitorY,int *infoMonitor,double ***simunateV,int sectionN[1],int *sectionOrder);
void Find_monitors(char workingPath[256],int monitorN[1],double *monitorX,double *monitorY,int *infoMonitor);
double Cal_area(double x1,double y1,double x2,double y2,double x3,double y3);
void Cal_fitness(double *ifit,int monitorT[1],int particleN[1],int monitorN[1],double ***simunateV,double *monitorV,int iter,int sectionN[1],double **p,double **x);


static default_random_engine e;
static uniform_real_distribution<double >u(0,1);

#endif