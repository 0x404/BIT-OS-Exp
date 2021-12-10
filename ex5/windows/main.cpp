/*
 * @author: 0x404
 * @Date: 2021-12-10 15:47:33
 * @LastEditTime: 2021-12-10 16:45:13
 * @Description: 
 */
#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <sys/stat.h>

using namespace std;

bool showDetail = false;


string nextPath(string now, string nex)
{
    // 获取下一级路径
    return now + "\\" + nex;
}

bool copy_file(string sourcePath, string targetPath)
{
    
}

bool copy_dir(string sourcePath, string targetPath)
{
    string nowPath = sourcePath + "\\*.*";
    WIN32_FIND_DATAA lpFindData;
    HANDLE hFind = FindFirstFileA(nowPath.c_str(), &lpFindData);
    do
    {
        if (!strcmp(lpFindData.cFileName, ".") || !strcmp(lpFindData.cFileName, "..")) continue;
        cout << lpFindData.cFileName << endl;

        if (lpFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            string nexPath_source = nextPath(sourcePath, lpFindData.cFileName);
            string nexPath_target = nextPath(targetPath, lpFindData.cFileName);
            CreateDirectoryA(nexPath_target.c_str(), NULL);
            HANDLE hSource = CreateFileA(nexPath_source.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
            HANDLE hTarget = CreateFileA(nexPath_target.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
            if (hSource == INVALID_HANDLE_VALUE)
            {
                cout << "[error] : 打开" << nexPath_source << "失败."  << endl;
                return false;
            }
            if (hTarget == INVALID_HANDLE_VALUE)
            {
                cout << "[error] : 打开" << nexPath_target << "失败."  << endl;
                return false;
            }
            FILETIME createTime, accessTime, writeTime;
			GetFileTime(hSource, &createTime, &accessTime, &writeTime);
			SetFileTime(hTarget, &createTime, &accessTime, &writeTime);
			SetFileAttributes(nexPath_target.c_str(), GetFileAttributes(nexPath_source.c_str()));
            copy_dir(nexPath_source, nexPath_target);

        }
        else
        {

        }
    }
    while (FindNextFileA(hFind, &lpFindData) != 0);
    
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 4)
    {
        cout << "[error] : 参数错误." << endl;
        return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "--help"))
    {
        cout << "mycp --help \t 显示帮助信息"  << endl;
        cout << "mycp [source path] [target path] \t 复制" << endl;
        cout << "mycp [source path] [target path] -show \t 显示复制过程" << endl;
        return 0;
    }
    if (argc == 4)
    {
        if (strcmp(argv[3], "-show") == 0)
        {
            showDetail = true;
        }
        else 
        {
            cout << "[error] : 参数错误." << endl;
            return 0;
        }
    }

    string sourcePath = argv[1];
    string targetPath = argv[2];
    WIN32_FIND_DATAA lpFindData;
    HANDLE hFind = FindFirstFileA(sourcePath.c_str(), &lpFindData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "[error] : 源文件不存在." << endl;
        return 0;
    }

    hFind = FindFirstFileA(targetPath.c_str(), &lpFindData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        CreateDirectoryA(targetPath.c_str(), NULL);
	    
    }

    copy_dir(sourcePath, targetPath);

    return 0;
}