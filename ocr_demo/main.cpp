/*
dependencies:
pvt.cppan.demo.google.tesseract.libtesseract: master
pvt.cppan.demo.danbloomberg.leptonica: 1
*/

#include <iostream>
#include <io.h>
#include<fstream>
#include <memory>

#include <allheaders.h> // leptonica main header for image io
#include <baseapi.h> // tesseract main header

#include <list>

#include  <direct.h>  
#include  <stdio.h> 

#include <thread>
#include <mutex>

char   buffer[MAX_PATH];


using namespace std;

std::list<string> lists;
std::mutex mutex_;

void listFiles(const char * dir)
{
	lists.clear();

	intptr_t handle;
	_finddata_t findData;

	handle = _findfirst(dir, &findData);    // 查找目录中的第一个文件
	if (handle == -1)
	{
		cout << "Failed to find first file!\n";
		return;
	}

	do
	{
		if (findData.attrib & _A_SUBDIR
			&& strcmp(findData.name, ".") != 0
			&& strcmp(findData.name, "..") != 0
			) {
			// 是否是子目录并且不为"."或".."
			//cout << findData.name << "\t<dir>\n";
			//cout << "";
		}
		else {
			//cout << findData.name << "\t" << findData.size << endl;
			string str(findData.name);
			int tmp = str.find(".png");
			int tmp2 = str.find(".jpg");

			if ((tmp >= 0) || (tmp2 >= 0)) {
				tmp = str.find(".png.");
				tmp2 = str.find(".jpg.");
				if (tmp<0 && tmp2<0)
				{
					lists.push_back(str);
				}
			}

			//if ((str.find(".png") >= 0) || (str.find(".jpg") >= 0)) {
			//	if (str.find(".png.") < 0 || str.find(".jpg.") < 0)
			//	{
			//		lists.push_back(str);
			//	}
			//}
		}
			
	} while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件

	cout << "Done!\n";
	_findclose(handle);    // 关闭搜索句柄
}

void dealOCR(tesseract::TessBaseAPI *tess) {
	string filename;
	
	bool isRunning = true;
	while (isRunning) {
		{
			std::lock_guard<std::mutex> lg(mutex_);
			if (lists.size()>0) {
				filename = *lists.begin();
				lists.pop_front();
			}
			else {
				isRunning = false;
				continue;
			}
		}

		auto pixs = pixRead(filename.c_str());
		if (!pixs)
		{
			std::cout << "Cannot open input file: " << filename << std::endl;
			isRunning = false;
			return;
		}

		//std::string name(argv[1]);
		std::string name(filename.c_str());
		// recognize
		tess->SetImage(pixs);
		tess->Recognize(0);

		std::ofstream ofs(name + ".txt");

		// get result and delete[] returned char* string
		std::cout << std::unique_ptr<char[]>(tess->GetUTF8Text()).get() << std::endl;
		ofs << std::unique_ptr<char[]>(tess->GetUTF8Text()).get();
		ofs.close();

		// cleanup
		pixDestroy(&pixs);
	}	
}


int main(int argc, char *argv[])
{
	//std::cout << argv[0] << std::endl;
	_getcwd(buffer, MAX_PATH);
	char dirNew[200];
	strcpy_s(dirNew, buffer);
	strcat_s(dirNew, "\\*.*");
	listFiles(dirNew);
	string dirNewPath(buffer);
	/*auto it = lists.begin();
	for (; it!=lists.end(); it++)
	{
		std::cout << *it << std::endl;
	}*/

	//if (argc == 1)
	//	return 1;

	tesseract::TessBaseAPI tess;
	tesseract::TessBaseAPI tess2;
	tesseract::TessBaseAPI tess3;

	if (tess.Init("./tessdata", "chi_sim"))
	{
		std::cout << "OCRTesseract: Could not initialize tesseract." << std::endl;
		return 1;
	}

	if (tess2.Init("./tessdata", "chi_sim"))
	{
		std::cout << "OCRTesseract: Could not initialize tesseract." << std::endl;
		return 1;
	}

	if (tess3.Init("./tessdata", "chi_sim"))
	{
		std::cout << "OCRTesseract: Could not initialize tesseract." << std::endl;
		return 1;
	}

	// setup
	tess.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	tess.SetVariable("save_best_choices", "T");

	tess2.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	tess2.SetVariable("save_best_choices", "T");

	tess3.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	tess3.SetVariable("save_best_choices", "T");
	//threadgroup

	std::thread th(dealOCR, &tess);
	std::thread th2(dealOCR, &tess2);
	std::thread th3(dealOCR, &tess3);

	th.join();
	th2.join();
	th3.join();

	// read image
	//it = lists.begin();;
	//auto it = lists.begin();
	//for (; it != lists.end(); it++)
	//{
	//	string tName = dirNewPath + "\\" + *it;

	//	std::thread th(&dealOCR, tName,tess);
	//	th.join();

	//	

	//	////auto pixs = pixRead(argv[1]);
	//	//auto pixs = pixRead(tName.c_str());
	//	//if (!pixs)
	//	//{
	//	//	std::cout << "Cannot open input file: " << argv[1] << std::endl;
	//	//	return 1;
	//	//}


	//	////std::string name(argv[1]);
	//	//std::string name(tName.c_str());
	//	//// recognize
	//	//tess.SetImage(pixs);
	//	//tess.Recognize(0);

	//	//std::ofstream ofs(name + ".txt");

	//	//// get result and delete[] returned char* string
	//	//std::cout << std::unique_ptr<char[]>(tess.GetUTF8Text()).get() << std::endl;
	//	//ofs << std::unique_ptr<char[]>(tess.GetUTF8Text()).get();
	//	//ofs.close();

	//	//// cleanup
	//	//
	//	//pixDestroy(&pixs);
	//	
	//}
	tess.Clear();
	tess2.Clear();
	tess3.Clear();

	return 0;
}

