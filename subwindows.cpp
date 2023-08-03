#include "subwindow.h"
int Open_app(char path[256],char file[256])
{
	//HINSTANCE hNewExe = ShellExecuteA(NULL, "open", "G:\\Ideas\\DSCM-CAU\\test\\ParallelEPCA.exe", NULL, NULL, SW_SHOW);
	//HINSTANCE hNewExe1= ShellExecute(NULL,"open","C:\\xxx.exe",NULL,“c：\\”,SW_SHOW);
	//HINSTANCE hNewExe = ShellExecuteA(NULL, "open", "G:\\Ideas\\DSCM-CAU\\test\\ParallelEPCA.exe", NULL, "G:\\Ideas\\DSCM-CAU\\test\\", SW_SHOWNORMAL);

	//HINSTANCE hNewExe = ShellExecuteA(NULL, "open", file, NULL, path, SW_SHOWNORMAL);
	HINSTANCE hNewExe = ShellExecuteA(NULL, "open", file, NULL, path, SW_SHOW);

	if ((DWORD)hNewExe <= 32)
	{
		printf("return value:%d\n", (DWORD)hNewExe);
	}
	else
	{
		printf("successed!\n");
	}

	/*printf("GetLastError: %d\n", GetLastError());*/
	//system("pause");
	return 1;
}

BOOL FindandKill_process(HWND hWnd)
{
	DWORD   ProcessID;
	HANDLE   hProcess;
	if (hWnd == NULL)
	{
		return   FALSE;
	}
	else
	{
		if (GetWindowThreadProcessId(hWnd, &ProcessID) == 0)//&#65418;&#65383;&#65392;&#65436;  
		{
			return FALSE;
		}
		else
		{
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
			if (hProcess == NULL)
			{
				return FALSE;
			}
			else
			{
				if (TerminateProcess(hProcess, 0))
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
		}
	}
}

void SAPSO_main1(int particleN[1],double learningCof[4],double SimuACof[2],int maxIter[1],int inverseD[1],int *paraLocation,int randSeed[1],double *inverseUp,double *inverseDn,double epX[1],int monitorN[1],double *monitorX,double *monitorY,double *monitorV,double *inversePara,double misFit[1],char workingPath[256],int parallelN[1],int sectionN[1],int *sectionOrder,int monitorT[1])
{
	//*开发日志--------------------------------------------------------------------------------------------------------------------------------
	//Developer：Jade Wang
	//Version：  1.0
	//2022.12.04：搭建软件基本框架
	//2022.12.05：编好SA-PSO主程序
	//*输入------------------------------------------------------------------------------------------------------------------------------------
	//particleN：粒子数
	//learningCof：学习率，0-wmin，1-wmax，2-c1,3-c2
	//SimuACof：退火因子,0-初始温度,1-衰减系数
	//maxIter：最大迭代数
	//inverseD：反分析参数个数
	//paralocation:反分析参数在material.txt的位置
	//randSeed：随机种子数
	//inverseUp:反分析参数上限,包括位置和速度，如果是2维度的，10,0.49,1,0.05
	//inverseDn:反分析参数下限，包括位置和速度，如果是2维度的，2,0.2,-1,-0.05
	//epX：误差
	//monitorN：监测点个数
	//monitorX：监测点X
	//monitorY：监测点Y
	//monitorV：监测点实测值
	//workingPath：工作目录
	//parallelN：并行个数
	//sectionN：监测时刻个数
	//sectionOrder：监测时刻代号（步数）
	//*输出------------------------------------------------------------------------------------------------------------------------------------
	//inversePara：反分析参数值
	//misFit：误差
	//*初始声明与赋值--------------------------------------------------------------------------------------------------------------------------------
	int i,j,k;
	double **x;
	double **v;
	double **p;
	double *pg;
	double *ifit;
	double *TF;
	double gfit;
	double ***simunateV;
	x = new double *[particleN[0]];
	v = new double *[particleN[0]];
	p = new double *[particleN[0]];
	pg = new double [inverseD[0]];
	ifit = new double [particleN[0]];
	TF = new double [particleN[0]];
	for (i = 0; i< particleN[0];i++)
	{
		x[i] = new double [inverseD[0]];
		v[i] = new double [inverseD[0]];
		p[i] = new double [inverseD[0]];
	}
	double sumTF;
	double T;
	int iter;
	double w;

	int materialN;
	double **materialV;
	//int **materialI;
	char materialPath[256];

	simunateV = new double **[particleN[0]];

	for(i = 0; i<particleN[0];i++)
	{
		simunateV[i] = new double *[sectionN[0]];
		for (j = 0; j<sectionN[0];j++)
		{
			simunateV[i][j] = new double [monitorN[0]*2];
		}
	}

	int *infoMonitor;
	infoMonitor = new int [5*monitorN[0]];

	char misFitPath[256];
	char processPath[256];
	//*主程序---------------------------------------------------------------------------------------------------------------------------------------
	sprintf(misFitPath,"%s\\MisFit.txt",workingPath);
	sprintf(processPath,"%s\\Process.txt",workingPath);
	ofstream misFitFile(misFitPath);
	ofstream processFile(processPath);


	sprintf(materialPath,"%sMODEL\\Materials.txt",workingPath);
	FILE *materialFile=fopen(materialPath,"r");
	fscanf(materialFile, "%d" ,&materialN);

	materialV = new double *[materialN];
	//materialI = new int *[materialN];

	for (i = 0;i<materialN;i++)
	{
		materialV[i] = new double [22];
		//materialI[i] = new int [2];
		for (j=0;j<22;j++)
			fscanf(materialFile, "%lf",&materialV[i][j]);

	}
	/*for (i = 0; i< materialN;i++)
	{
	for (j=0;j<22;j++)
	printf("%lf\n",materialV[i][j]);	

	}*/

	fclose(materialFile);

	//--找到监测点所属的单元
	Find_monitors(workingPath,monitorN,monitorX,monitorY,infoMonitor);


	iter = 0;
	//--初始化速度和位置
	Initial_rand(randSeed[0]);
	for (i = 0 ; i<particleN[0];i++)
	{
		for (j = 0; j<inverseD[0];j++)
		{
			x[i][j] = inverseDn[j] + (inverseUp[j] - inverseDn[j]) * Get_rand();
			v[i][j] = inverseDn[j+inverseD[0]] + (inverseUp[j+inverseD[0]] - inverseDn[j+inverseD[0]]) * Get_rand();
			p[i][j] = x[i][j];
		}
	}

	//--计算适应值

	//新建文件夹
	Create_dir(workingPath,iter+1,parallelN);
	//更改txt参数赋值
	Assign_para(workingPath,iter+1,parallelN,inverseD,paraLocation,x,materialN,materialV,sectionN,sectionOrder);
	//并行计算
	Cal_parallel(workingPath,iter+1,parallelN);	
	//提取对应点的数据
	Extract_data(workingPath,iter+1,parallelN,monitorT,monitorN,monitorX,monitorY,infoMonitor,simunateV,sectionN,sectionOrder);

	/*for (i=0;i<particleN[0];i++)
	{
	for (j = 0;j<monitorN[0];j++)
	{
	printf("%d %d %e\n",i,j,simunateV[i][j]);
	}
	}
	exit(0);*/
	//计算适应值大小，比较局部最优适应值，存入ifit[i]
	Cal_fitness(ifit,monitorT,particleN,monitorN,simunateV,monitorV,iter,sectionN,p,x);

	/*for (i = 0;i<particleN[0];i++)
	printf("%d %e\n",i,ifit[i]);*/



	//--计算全局最优
	gfit = ifit[0];
	for (j=0;j<inverseD[0];j++)
		pg[0]=p[0][j];
	for (i=1;i<particleN[0];i++)
	{
		if (ifit[i]<gfit)
		{
			gfit = ifit[i];
			for (j=0;j<inverseD[0];j++)
				pg[j]=p[i][j];
		}
	}

	//--确定初始温度
	T = SimuACof[0];

	do{

		//--确定温度适配值
		sumTF = 0.0;

		for (i=0;i<particleN[0];i++)
		{
			TF[i] = exp(-(ifit[i]-gfit)/T);
			sumTF += TF[i];
		}
		for (i=0;i<particleN[0];i++)
		{
			TF[i]/=sumTF;
		}
		//轮盘赌
		sumTF = 0.0;
		for (i=0;i<particleN[0];i++)
		{
			sumTF += TF[i];
			if (Get_rand()<=sumTF)
			{
				gfit = ifit[i];
				for (j=0;j<inverseD[0];j++)
					pg[j]=p[i][j];
				break;
			}
		}
		//速度更新
		w = learningCof[1] - (learningCof[1]-learningCof[0])*iter/maxIter[0];
		for (i=0;i<particleN[0];i++)
		{
			for (j=0;j<inverseD[0];j++)
			{
				v[i][j] = w * v[i][j] + learningCof[2] * Get_rand() * (p[i][j] - x[i][j]) + learningCof[3] * Get_rand() * (pg[j] - x[i][j]) ;
				if (v[i][j]>inverseUp[j+inverseD[0]])
					v[i][j] = inverseUp[j+inverseD[0]];
				if (v[i][j]<inverseDn[j+inverseD[0]])
					v[i][j] = inverseDn[j+inverseD[0]];
				x[i][j] = x[i][j] + v[i][j];
				if (x[i][j]>inverseUp[j])
					x[i][j] = inverseUp[j];
				if (x[i][j]<inverseDn[j])
					x[i][j] = inverseDn[j];

			}
		}
		iter++;

		

		//processFile.precision(9);

		processFile<<"Step "<<iter <<endl;
		for (i =0;i<particleN[0];i++)
		{
			for (j =0;j<sectionN[0];j++)
			{
				for (k=0;k<monitorN[0];k++)
				{
					processFile<<i+1<<" "<<j+1<<" "<<k+1<<" "<<simunateV[i][j][k]<<" "<<monitorV[k]<<endl;
				}
			}
		}

		for (j=0;j<inverseD[0];j++)
			inversePara[j]=pg[j];
		misFit[0] = gfit;

		misFitFile.precision(9);

		misFitFile<<iter<<" "<<1e4*misFit[0];

		for (j=0;j<inverseD[0];j++)
			misFitFile<<" "<<inversePara[j];

		misFitFile<<endl;

		//--计算适应值
		//新建文件夹
		Create_dir(workingPath,iter+1,parallelN);
		//更改txt参数赋值
		Assign_para(workingPath,iter+1,parallelN,inverseD,paraLocation,x,materialN,materialV,sectionN,sectionOrder);
		//并行计算
		Cal_parallel(workingPath,iter+1,parallelN);	
		//提取对应点的数据
		Extract_data(workingPath,iter+1,parallelN,monitorT,monitorN,monitorX,monitorY,infoMonitor,simunateV,sectionN,sectionOrder);

		/*for (i=0;i<particleN[0];i++)
		{
		for (j = 0;j<monitorN[0];j++)
		{
		printf("%d %d %e\n",i,j,simunateV[i][j]);
		}
		}
		exit(0);*/
		//计算适应值大小，比较局部最优适应值，存入ifit[i]
		Cal_fitness(ifit,monitorT,particleN,monitorN,simunateV,monitorV,iter,sectionN,p,x);

		/*for (i = 0;i<particleN[0];i++)
		printf("%d %e\n",i,ifit[i]);*/

		//--计算全局最优
		gfit = ifit[0];
		for (j=0;j<inverseD[0];j++)
			pg[0]=p[0][j];
		for (i=1;i<particleN[0];i++)
		{
			if (ifit[i]<gfit)
			{
				gfit = ifit[i];
				for (j=0;j<inverseD[0];j++)
					pg[j]=p[i][j];
			}
		}

		//--存储
		

		/*for (i=1;i<particleN[0];i++)
		{
		printf("%e\n",ifit[i]);
		}*/





		//if (gfit<epX[0])
			if (misFit[0]<epX[0])
		{
			break;
		}


		T *= SimuACof[1];

		//iter++;

	}while(iter<maxIter[0]);

	misFitFile.close();
	processFile.close();
	//*主程序end---------------------------------------------------------------------------------------------------------------------------------------

	//*内存清零------------------------------------------------------------------------------------------------------------------------------------

	for (i = 0; i< particleN[0];i++)
	{
		delete []x[i];
		delete []v[i];
		delete []p[i];
	}
	delete []x;
	delete []v;
	delete []pg;
	delete []ifit;
	delete []TF;
	for (i=0;i<materialN;i++)
	{
		delete []materialV[i];
		//delete []materialI[i];
	}

	for (i = 0;i<particleN[0];i++)
	{
		for (j=0;j<sectionN[0];j++)
		{
			delete []simunateV[i][j];
		}
		delete []simunateV[i];
	}
	delete []materialV;
	delete []simunateV;
	delete []infoMonitor;
	//delete []materialI;

}

double Get_rand()
{
	//srand(time(0));//srand()函数产生一个以当前时间开始的随机种子.应该放在for等循环语句前面 不然要很长时间等待
	//return rand()%(NNN+1)/(double)(NNN+1);

	//std::default_random_engine e;
	//for (size_t i = 0; i < 5; ++i) //生成五个随机数       
	//	std::cout << e() << std::endl;
	//
	//return 0;
	//int nn = 99999;
	//static uniform_int_distribution<unsigned> u(0,nn);
	return ((double)u(e));


}

void Initial_rand(int index)
{

	if (index == 0)
	{
		e.seed(time(0));
	}
	else
	{
		e.seed(index);
	}

}

void Create_dir(char workingPath[256],int iter,int parallelN[1])
{
	int i;
	char command[256];
	char iter_Char[256];
	char currentDir[256];

	//system("stty -echo");

	itoa(iter,iter_Char,10);
	sprintf(currentDir,"%s\\iter%s", workingPath,iter_Char);

	std::string prefix = currentDir;

	if (_access(prefix.c_str(), 0) == -1)	//如果文件夹不存在
		_mkdir(prefix.c_str());				//则创建


	for (i=0;i<parallelN[0];i++)
	{
		char path1[256];
		char path2[256];
		char path3[256];
		char path4[256];


		char oldDir[256];
		char newDir[256];
		char oldDir2[256];
		char newDir2[256];
		char i_Char[256];

		itoa(i+1,i_Char,10);
		sprintf(path1,"%s\\%s", currentDir,i_Char);
		sprintf(path2,"%s\\%s\\cal\\", currentDir,i_Char);
		sprintf(path3,"%s\\%s\\cal\\data\\", currentDir,i_Char);
		sprintf(path4,"%s\\%s\\cal\\input\\", currentDir,i_Char);

		prefix = path1;
		if (_access(prefix.c_str(), 0) == -1)	//如果文件夹不存在
			_mkdir(prefix.c_str());				//则创建

		prefix = path2;
		if (_access(prefix.c_str(), 0) == -1)	//如果文件夹不存在
			_mkdir(prefix.c_str());				//则创建

		prefix = path3;
		if (_access(prefix.c_str(), 0) == -1)	//如果文件夹不存在
			_mkdir(prefix.c_str());				//则创建

		prefix = path4;
		if (_access(prefix.c_str(), 0) == -1)	//如果文件夹不存在
			_mkdir(prefix.c_str());				//则创建


		sprintf(oldDir,"%s\\MODEL", workingPath);
		sprintf(newDir,"%s\\%s\\cal\\input", currentDir,i_Char);

		std::string pre = oldDir;
		std::string now = newDir;

		Dir::copy(pre, now);

		sprintf(oldDir2,"%s\\APP", workingPath);
		sprintf(newDir2,"%s\\%s", currentDir,i_Char);

		pre = oldDir2;
		now = newDir2;

		Dir::copy(pre, now);


	}
}

void Assign_para(char workingPath[256],int iter,int parallelN[1],int inverseD[1],int *paraLocation,double **x,int materialN,double **materialV, int sectionN[1],int *sectionOrder)
{
	int i,j,k;
	char currentDir[256];
	char iter_Char[256];
	itoa(iter,iter_Char,10);
	sprintf(currentDir,"%s\\iter%s", workingPath,iter_Char);

	for (i=0;i<parallelN[0];i++)
	{
		char path[256];
		char path2[256];
		char path3[256];
		char i_Char[256];
		itoa(i+1,i_Char,10);
		sprintf(path,"%s\\%s\\cal\\input\\Materials.txt", currentDir,i_Char);
		sprintf(path2,"%s\\%s\\cal\\input\\Monitoring.txt", currentDir,i_Char);
		sprintf(path3,"%s\\%s\\cal\\data\\IsEnd.txt", currentDir,i_Char);

		for(k = 0;k<materialN;k++)
		{
			for (j = 0; j<inverseD[0];j++)
			{
				if (paraLocation[2*j]==k+1)
				{
					materialV[k][paraLocation[2*j+1]-1] = x[i][j];
				}
			}
		}

		ofstream materialNew(path);
		materialNew<<materialN<<endl;
		for(k = 0;k<materialN;k++)
		{
			/*materialNew<<(int)materialV[k][0]<<" "<<(int)materialV[k][1]<<" "<<materialV[k][2]<<" "<<materialV[k][3]<<" "<<materialV[k][4]<<" " \
				<<materialV[k][5]<<" "<<materialV[k][6]<<" "<<materialV[k][7]<<" "<<materialV[k][8]<<" "<<materialV[k][9]<<" "\
				<<materialV[k][10]<<" "<<materialV[k][11]<<" "<<materialV[k][12]<<" "<<materialV[k][13]<<" "<<materialV[k][14]<<" "\
				<<materialV[k][15]<<" "<<materialV[k][16]<<" "<<materialV[k][17]<<" "<<materialV[k][18]<<" "<<(int)materialV[k][19]<<" "\
				<<materialV[k][20]<<" "<<materialV[k][21]<<" "<<materialV[k][22]<<endl;*/
			materialNew<<(int)materialV[k][0]<<" "<<(int)materialV[k][1]<<" "<<materialV[k][2]<<" "<<materialV[k][3]<<" "<<materialV[k][4]<<" " \
				<<materialV[k][5]<<" "<<materialV[k][6]<<" "<<materialV[k][7]<<" "<<materialV[k][8]<<" "<<materialV[k][9]<<" "\
				<<materialV[k][10]<<" "<<materialV[k][11]<<" "<<materialV[k][12]<<" "<<materialV[k][13]<<" "<<materialV[k][14]<<" "\
				<<materialV[k][15]<<" "<<materialV[k][16]<<" "<<materialV[k][17]<<" "<<materialV[k][18]<<" "<<(int)materialV[k][19]<<" "\
				<<materialV[k][20]<<" "<<materialV[k][21]<<endl;
		}
		materialNew.close();

		ofstream monitoringFile(path2);

		monitoringFile<<sectionN[0]<<endl;
		for(j=0;j<sectionN[0];j++)
		{
			monitoringFile<<sectionOrder[j]<<endl;
		}

		monitoringFile.close();

		std::string isEndFile = path3;

		if (access(isEndFile.c_str(), 0) == 0)//文件存在
		{
			if (remove(isEndFile.c_str()) == 0)
			{

			}
		}
	}
}

void Cal_parallel(char workingPath[256],int iter,int parallelN[1])
{
	int i,j;
	int index = 0;
	int indexTemp;

	char currentDir[256];
	char iter_Char[256];
	itoa(iter,iter_Char,10);
	sprintf(currentDir,"%s\iter%s", workingPath,iter_Char);

	//for (i=0;i<2;i++)
	for (i=0;i<parallelN[0];i++)
	{
		char path[256];
		char file[256];
		char i_Char[256];
		itoa(i+1,i_Char,10);
		sprintf(path,"%s\\%s\\", currentDir,i_Char);
		sprintf(file,"%s\\%s\\ParallelEPCA.exe", currentDir,i_Char);
		printf("%d app running ",i+1);
		Open_app(path,file);
	}

	do
	{
		index = 1;
		//for (i=0;i<2;i++)
		for (i=0;i<parallelN[0];i++)
		{
			char path[256];

			char i_Char[256];
			itoa(i+1,i_Char,10);
			sprintf(path,"%s\\%s\\cal\\data\\IsEnd.txt", currentDir,i_Char);

			std::string isEndFile = path;

			if (access(isEndFile.c_str(), 0) == 0)//文件存在
			{
				FILE *fpIsEnd = fopen(path,"r");

				fscanf(fpIsEnd,"%d",&indexTemp);
				fclose(fpIsEnd);

				index = index * indexTemp;

			}
			else
			{
				index = 0;
			}

		}

		//printf("%d",index);
		if (index == 1)
		{
			break;
		}


	}while(1);

	//_sleep(2*1000);
	HWND *hWndRcv;
	hWndRcv = new HWND [parallelN[0]];
	if (index == 1)
	{
		//for (i=0;i<4;i++)
		for (i=0;i<parallelN[0];i++)
		{
			char path[256];
			char file[256];
			char i_Char[256];
			itoa(i+1,i_Char,10);
			sprintf(path,"%s\\%s\\", currentDir,i_Char);
			sprintf(file,"%s\\%s\\ParallelEPCA.exe", currentDir,i_Char);


			char* szStr = file;	 
			CString str = CString(szStr);
			USES_CONVERSION;
			LPCWSTR wszClassName = A2CW(W2A(str));
			str.ReleaseBuffer();


			//HWND hWndRcv = ::FindWindow(NULL,TEXT("G:\\Ideas\\DSCM-CAU\\test\\ParallelEPCA.exe"));
			if (i==0)
			{
				//hWndRcv[0] = ::FindWindow(NULL,TEXT("G:\\Ideas\\DSCM-CAU\\test\\ParallelEPCA.exe"));
				//hWndRcv[0] = ::FindWindow(NULL,TEXT("G:\\Ideas\\DSCM-CAU\\SAPSO\\SAPSO1\\SAPSO1\\Zhao1\\iter1\\1\\ParallelEPCA.exe"));
				hWndRcv[0] = ::FindWindow(NULL,wszClassName);

				//printf("%x\n",hWndRcv[0]);
			}
			else
			{
				hWndRcv[i] = ::FindWindow(NULL,wszClassName);
				//hWndRcv[i] = ::FindWindowEx(NULL,hWndRcv[i-1],NULL,wszClassName);
				//printf("%x\n",hWndRcv[i]);
			}


			FindandKill_process(hWndRcv[i]);

			//_sleep(500);
			//HWND hWndRcv = ::FindWindow(NULL,TEXT("G:\\Ideas\\DSCM-CAU\\SAPSO\\SAPSO1\\SAPSO1\\Zhao1\\iter1\\1\\ParallelEPCA.exe"));
			//HWND hWndRcv = ::FindWindow(NULL,TEXT("ParallelEPCA.exe"));
			//HWND hWndRcv = ::FindWindow(NULL,TEXT("ParallelEPCA"));

			//HWND hWndRcv = ::FindWindow(NULL,wszClassName);


			printf("%d app is closed!\n",i+1);
		}
	}





	delete []hWndRcv;



}

void Extract_data(char workingPath[256],int iter,int parallelN[1],int monitorT[1],int monitorN[1],double *monitorX,double *monitorY,int *infoMonitor,double ***simunateV,int sectionN[1],int *sectionOrder)
{
	int i,j,k;
	char currentDir[256];
	char iter_Char[256];
	char headFile[256];
	char tailFile[256];
	double tmp[20];

	double x,y,x1,y1,x2,y2,x3,y3,x4,y4;
	double N1,N2,N3,N4;
	int n[4];
	double **nodeX,**nodeY,**nodeDispX,**nodeDispY;


	nodeX = new double *[monitorN[0]];
	nodeY = new double *[monitorN[0]];
	nodeDispX = new double *[monitorN[0]];
	nodeDispY = new double *[monitorN[0]];

	for (i = 0 ; i<monitorN[0];i++)
	{
		nodeX[i] = new double [4];
		nodeY[i] = new double [4];
		nodeDispX[i] = new double [4];
		nodeDispY[i] = new double [4];
	}

	itoa(iter,iter_Char,10);
	sprintf(currentDir,"%s\iter%s", workingPath,iter_Char);

	sprintf(headFile,"stress_strain");
	sprintf(tailFile,"-2.3depca");


	for (i=0;i<parallelN[0];i++)
	{
		char path[256];
		char file[256];
		char i_Char[256];
		char order_Char[256];
		itoa(i+1,i_Char,10);


		for (j = 0;j<sectionN[0];j++)
		{
			sprintf(path,"%s\\%s\\cal\\data\\", currentDir,i_Char);
			itoa(sectionOrder[j]-1,order_Char,10);
			sprintf(file,"%s%s%s",headFile,order_Char,tailFile);
			strcat(path,file);

			for (k = 0;k<monitorN[0];k++)
			{
				/*x = monitorX[k];
				y = monitorY[k];*/

				n[0] = infoMonitor[5*k+1];
				n[1] = infoMonitor[5*k+2];
				n[2] = infoMonitor[5*k+3];
				n[3] = infoMonitor[5*k+4];

				int index = 4;

				if (n[2]==n[3])
					index = 3;


				int nCount = 0;
				if (index == 3)
				{
					//三角形

					for (nCount =0; nCount<3;nCount++)
					{
						ifstream fin(path);
						if (fin)
						{
							string str;
							char ttt[1024];
							int curLine = 0;
							//int nCount = 0;
							while (getline(fin, str))
							{
								curLine++;

								if (curLine>2)
								{

									//三角形

									if (curLine-3==n[nCount])
									{
										strcpy(ttt,str.c_str());
										sscanf(ttt, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &nodeX[k][nCount],&nodeY[k][nCount],&tmp[0],&nodeDispX[k][nCount],&nodeDispY[k][nCount] ,\
											&tmp[1],&tmp[2],&tmp[3],&tmp[4],&tmp[5],\
											&tmp[6],&tmp[7],&tmp[8],&tmp[9],&tmp[10],\
											&tmp[11],&tmp[12],&tmp[13],&tmp[14],&tmp[15]) ;

									}



								}

							}
						}
						else
						{
							std::cout << "Open file faild." << std::endl;

							printf("%s\n",path);
						}

						fin.close();
					}
				}
				else
				{
					//四边形
					for (nCount =0; nCount<4;nCount++)
					{
						ifstream fin(path);
						if (fin)
						{
							string str;
							char ttt[1024];
							int curLine = 0;
							//int nCount = 0;
							while (getline(fin, str))
							{
								curLine++;

								if (curLine>2)
								{



									//四边形

									if (curLine-3==n[nCount])
									{
										strcpy(ttt,str.c_str());
										sscanf(ttt, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &nodeX[k][nCount],&nodeY[k][nCount],&tmp[0],&nodeDispX[k][nCount],&nodeDispY[k][nCount] ,\
											&tmp[1],&tmp[2],&tmp[3],&tmp[4],&tmp[5],\
											&tmp[6],&tmp[7],&tmp[8],&tmp[9],&tmp[10],\
											&tmp[11],&tmp[12],&tmp[13],&tmp[14],&tmp[15]) ;

									}



								}

							}
						}
						else
						{
							std::cout << "Open file faild." << std::endl;

							printf("%s\n",path);
						}

						fin.close();
					}
				}



			}

			for (k = 0;k<monitorN[0];k++)
			{
				x = monitorX[k];
				y = monitorY[k];

				if (infoMonitor[5*k+3] == infoMonitor[5*k+4])
				{
					//三角形
					N1 = ((nodeX[k][1]*nodeY[k][2] -nodeX[k][2]*nodeY[k][1] ) + (nodeY[k][1]-nodeY[k][2])*x + (nodeX[k][2] - nodeX[k][1])*y) \
						/ (nodeX[k][1]*nodeY[k][2] + nodeX[k][2]*nodeY[k][0] + nodeX[k][0] * nodeY[k][1] - nodeX[k][2] * nodeY[k][1] - nodeX[k][0] * nodeY[k][2] - nodeX[k][1] * nodeY[k][0] );

					N2 = ((nodeX[k][2]*nodeY[k][0] -nodeX[k][0]*nodeY[k][2] ) + (nodeY[k][2]-nodeY[k][0])*x + (nodeX[k][0] - nodeX[k][2])*y) \
						/ (nodeX[k][2]*nodeY[k][0] + nodeX[k][0]*nodeY[k][1] + nodeX[k][1] * nodeY[k][2] - nodeX[k][0] * nodeY[k][2] - nodeX[k][1] * nodeY[k][0] - nodeX[k][2] * nodeY[k][1] );

					N3 = ((nodeX[k][0]*nodeY[k][1] -nodeX[k][1]*nodeY[k][0] ) + (nodeY[k][0]-nodeY[k][1])*x + (nodeX[k][1] - nodeX[k][0])*y) \
						/ (nodeX[k][0]*nodeY[k][1] + nodeX[k][1]*nodeY[k][2] + nodeX[k][2] * nodeY[k][0] - nodeX[k][1] * nodeY[k][0] - nodeX[k][2] * nodeY[k][1] - nodeX[k][0] * nodeY[k][2] );

					if (monitorT[0] == 0)
					{
						simunateV[i][j][k] = nodeDispX[k][0] * N1 + nodeDispX[k][1] * N2 + nodeDispX[k][2] * N3;
						simunateV[i][j][k+monitorN[0]] = nodeDispY[k][0] * N1 + nodeDispY[k][1] * N2 + nodeDispY[k][2] * N3;

					}
					else if (monitorT[0] == 1)
					{
						simunateV[i][j][k] = nodeDispX[k][0] * N1 + nodeDispX[k][1] * N2 + nodeDispX[k][2] * N3;
					}
					else
					{
						simunateV[i][j][k+monitorN[0]] = nodeDispY[k][0] * N1 + nodeDispY[k][1] * N2 + nodeDispY[k][2] * N3;
					}




				}
				else
				{
					//四边形
					N1 = 0.25*(1.0+nodeX[k][0]*x) * (1.0 + nodeY[k][0] * y);
					N2 = 0.25*(1.0+nodeX[k][1]*x) * (1.0 + nodeY[k][1] * y);
					N3 = 0.25*(1.0+nodeX[k][2]*x) * (1.0 + nodeY[k][2] * y);
					N4 = 0.25*(1.0+nodeX[k][3]*x) * (1.0 + nodeY[k][3] * y);

					if (monitorT[0] == 0)
					{
						simunateV[i][j][k] = nodeDispX[k][0] * N1 + nodeDispX[k][1] * N2 + nodeDispX[k][2] * N3 + nodeDispX[k][3] * N4;
						simunateV[i][j][k+monitorN[0]] = nodeDispY[k][0] * N1 + nodeDispY[k][1] * N2 + nodeDispY[k][2] * N3+ nodeDispY[k][3] * N4;

					}
					else if (monitorT[0] == 1)
					{
						simunateV[i][j][k] = nodeDispX[k][0] * N1 + nodeDispX[k][1] * N2 + nodeDispX[k][2] * N3 + nodeDispX[k][3] * N4;
					}
					else
					{
						simunateV[i][j][k+monitorN[0]] = nodeDispY[k][0] * N1 + nodeDispY[k][1] * N2 + nodeDispY[k][2] * N3+ nodeDispY[k][3] * N4;
					}
				}

				/*printf("%d %d %d %d\n",infoMonitor[5*k+1],infoMonitor[5*k+2],infoMonitor[5*k+3],infoMonitor[5*k+4]);
				printf("%lf %lf %lf %lf\n",nodeX[k][0],nodeX[k][1],nodeX[k][2],nodeX[k][3]);
				printf("%lf %lf %lf %lf\n",nodeY[k][0],nodeY[k][1],nodeY[k][2],nodeY[k][3]);
				printf("%lf %lf %lf %lf\n",nodeDispX[k][0],nodeDispX[k][1],nodeDispX[k][2],nodeDispX[k][3]);
				printf("%lf %lf %lf %lf\n",nodeDispY[k][0],nodeDispY[k][1],nodeDispY[k][2],nodeDispY[k][3]);*/
			}

			//exit(0);
		}
	}


	//----------------------------------------------------------
	for (i = 0 ; i<monitorN[0];i++)
	{
		delete []nodeX[i];
		delete []nodeY[i];
		delete []nodeDispX[i];
		delete []nodeDispY[i];
	}
	delete []nodeX;
	delete []nodeY;
	delete []nodeDispX;
	delete []nodeDispY;

}

void Find_monitors(char workingPath[256],int monitorN[1],double *monitorX,double *monitorY,int *infoMonitor)
{
	int i,j;
	char nodePath[256];
	char elementPath[256];
	int nodeN,elementN;
	double *nodeX,*nodeY;
	int *elementC;
	int temp,temp1;
	double tmp,tmp1;
	double x,y,x1,y1,x2,y2,x3,y3,x4,y4;
	double area1,area2,area3,area4,area;

	double lengthMax;
	double lengthMin;
	double valueX;
	double valueY;

	sprintf(nodePath,"%sMODEL\\Nodes.txt",workingPath);
	sprintf(elementPath,"%sMODEL\\elements.txt",workingPath);
	FILE *nodeFile=fopen(nodePath,"r");
	fscanf(nodeFile, "%d" ,&nodeN);
	nodeX = new double [nodeN];
	nodeY = new double [nodeN];

	for (i = 0; i<nodeN; i++)
	{
		fscanf(nodeFile, "%d %lf %lf %lf" ,&temp, &nodeX[i],&nodeY[i],&tmp);
	}
	fclose(nodeFile);

	FILE *elementFile=fopen(elementPath,"r");
	fscanf(elementFile, "%d" ,&elementN);
	elementC = new int [4*elementN];

	for (i = 0; i<elementN; i++)
	{
		fscanf(elementFile, "%d %d %d %d %d %d" ,&temp, &temp1,&elementC[i],&elementC[i+elementN],&elementC[i+elementN*2],&elementC[i+elementN*3]);
	}

	fclose(elementFile);

	lengthMax = 0.0;
	lengthMin = 2e17;

	for (i=0;i<nodeN;i++)
	{
		if (nodeX[i]>lengthMax)
			lengthMax = nodeX[i];
		if (nodeX[i]<lengthMin)
			lengthMin = nodeX[i];

	}
	valueX = lengthMax - lengthMin;

	lengthMax = 0.0;
	lengthMin = 2e17;

	for (i=0;i<nodeN;i++)
	{
		if (nodeY[i]>lengthMax)
			lengthMax = nodeY[i];
		if (nodeY[i]<lengthMin)
			lengthMin = nodeY[i];

	}
	valueY = lengthMax - lengthMin;

	if (valueX<valueY)
		valueX = valueY;

	valueX = valueX * valueX * 1e-6;

	for (i =0 ;i<elementN;i++)
	{
		elementC[i]--;
		elementC[i+elementN*1]--;
		elementC[i+elementN*2]--;
		elementC[i+elementN*3]--;
	}


	for (i = 0; i< monitorN[0];i++)
	{
		x = monitorX[i];
		y = monitorY[i];

		for (j = 0; j<elementN;j++)
		{
			if (elementC[j+elementN*2] == elementC[j+elementN*3])
			{
				//三角形
				x1 = nodeX[elementC[j]];y1 = nodeY[elementC[j]];
				x2 = nodeX[elementC[j+elementN*1]];y2 = nodeY[elementC[j+elementN*1]];
				x3 = nodeX[elementC[j+elementN*2]];y3 = nodeY[elementC[j+elementN*2]];

				area1 = fabs(Cal_area(x,y,x1,y1,x2,y2));
				area2 = fabs(Cal_area(x,y,x1,y1,x3,y3));
				area3 = fabs(Cal_area(x,y,x3,y3,x2,y2));
				area = fabs(Cal_area(x3,y3,x1,y1,x2,y2));

				if (fabs(area1+area2+area3-area)<valueX)
				{
					infoMonitor[5*i] = j;
					infoMonitor[5*i+1] = elementC[j];
					infoMonitor[5*i+2] = elementC[j+elementN*1];
					infoMonitor[5*i+3] = elementC[j+elementN*2];
					infoMonitor[5*i+4] = elementC[j+elementN*3];
					break;
				}


			}
			else
			{
				//四边形

				x1 = nodeX[elementC[j]];y1 = nodeY[elementC[j]];
				x2 = nodeX[elementC[j+elementN*1]];y2 = nodeY[elementC[j+elementN*1]];
				x3 = nodeX[elementC[j+elementN*2]];y3 = nodeY[elementC[j+elementN*2]];
				x4 = nodeX[elementC[j+elementN*3]];y4 = nodeY[elementC[j+elementN*3]];

				area1 = fabs(Cal_area(x,y,x1,y1,x2,y2));
				area2 = fabs(Cal_area(x,y,x2,y2,x3,y3));
				area3 = fabs(Cal_area(x,y,x3,y3,x4,y4));
				area4 = fabs(Cal_area(x,y,x4,y4,x1,y1));
				area = fabs(Cal_area(x3,y3,x1,y1,x2,y2))+ fabs(Cal_area(x3,y3,x1,y1,x4,y4));

				//printf("%d %lf %lf %lf %lf %lf %lf\n",j,fabs(area1+area2+area3+area4-area),area1,area2,area3,area4,area);

				//printf("%d %lf %lf %lf %lf %lf %lf %lf %lf\n",j,x1,y1,x2,y2,x3,y3,x4,y4);
				//printf("%lf %lf\n",x,y);

				if (fabs(area1+area2+area3+area4-area)<valueX)
				{
					infoMonitor[5*i] = j;
					infoMonitor[5*i+1] = elementC[j];
					infoMonitor[5*i+2] = elementC[j+elementN*1];
					infoMonitor[5*i+3] = elementC[j+elementN*2];
					infoMonitor[5*i+4] = elementC[j+elementN*3];
					break;
				}

			}
		}

	}


	delete []nodeX;
	delete []nodeY;
	delete []elementC;

}

double Cal_area(double x1,double y1,double x2,double y2,double x3,double y3)
{
	return (x2*y3-x3*y2) - x1 * (y3-y2) + y1 * (x3-x2);
}

void Cal_fitness(double *ifit,int monitorT[1],int particleN[1],int monitorN[1],double ***simunateV,double *monitorV,int iter,int sectionN[1],double **p,double **x)
{
	int i,j,k;
	double error;
	double *curFit;
	curFit = new double [particleN[0]];
	for (i = 0;i<particleN[0];i++)
	{
		curFit[i] = ifit[i];
	}

	if (iter == 0)
	{
		switch (monitorT[0]){
		case 0:
			{
				for (i = 0;i<particleN[0];i++)
				{

					error = 0.0;
					for (k=0;k<sectionN[0];k++)
					{
						for (j = 0;j<monitorN[0];j++)
						{
							error += fabs(simunateV[i][k][j] - monitorV[j])+fabs(simunateV[i][k][j+monitorN[0]] - monitorV[j+monitorN[0]]);
						}
					}
					ifit[i] = error/monitorN[0]/2/sectionN[0];					

				}
			}
			break;
		case 1:
			{
				for (i = 0;i<particleN[0];i++)
				{

					error = 0.0;
					for (k=0;k<sectionN[0];k++)
					{
						for (j = 0;j<monitorN[0];j++)
						{
							error += fabs(simunateV[i][k][j] - monitorV[j]);
						}
					}
					ifit[i] = error/monitorN[0]/sectionN[0];					

				}

			}
			break;
		case 2:
			{
				for (i = 0;i<particleN[0];i++)
				{

					error = 0.0;
					for (k = 0;k<sectionN[0];k++)
					{
						for (j = 0;j<monitorN[0];j++)
						{
							error += fabs(simunateV[i][k][j+monitorN[0]] - monitorV[j+monitorN[0]]);
						}
					}
					ifit[i] = error/monitorN[0]/sectionN[0];					

				}
			}
			break;
		}
	}
	else
	{
		switch (monitorT[0]){
		case 0:
			{
				for (i = 0;i<particleN[0];i++)
				{

					error = 0.0;
					for (k=0;k<sectionN[0];k++)
					{
						for (j = 0;j<monitorN[0];j++)
						{
							error += fabs(simunateV[i][k][j] - monitorV[j])+fabs(simunateV[i][k][j+monitorN[0]] - monitorV[j+monitorN[0]]);
						}
					}
					ifit[i] = error/monitorN[0]/2/sectionN[0];					

				}
			}
			break;
		case 1:
			{
				for (i = 0;i<particleN[0];i++)
				{

					error = 0.0;
					for (k=0;k<sectionN[0];k++)
					{
						for (j = 0;j<monitorN[0];j++)
						{
							error += fabs(simunateV[i][k][j] - monitorV[j]);
						}
					}
					ifit[i] = error/monitorN[0]/sectionN[0];					

				}

			}
			break;
		case 2:
			{
				for (i = 0;i<particleN[0];i++)
				{

					error = 0.0;
					for (k=0;k<sectionN[0];k++)
					{
						for (j = 0;j<monitorN[0];j++)
						{
							error += fabs(simunateV[i][k][j+monitorN[0]] - monitorV[j+monitorN[0]]);
						}
					}
					ifit[i] = error/monitorN[0]/sectionN[0];					

				}
			}
			break;
		}

		for (i = 0;i<particleN[0];i++)
		{
			if (ifit[i]<curFit[i])
			{
				//do nothing
			}
			else
			{
				ifit[i] = curFit[i];
				for (j = 0;j<monitorN[0];j++)
				{
					p[i][j] = x[i][j];
				}


			}
		}

	}
	delete []curFit;
}