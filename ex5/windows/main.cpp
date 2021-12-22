/*
 * @author: 0x404
 * @Date: 2021-12-10 15:47:33
 * @LastEditTime: 2021-12-23 05:51:16
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



bool copy_file(string sourcePath, string targetPath, int depth)
{
    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy file] : " << sourcePath << " copy start." << endl;
    }

    WIN32_FIND_DATAA lpFindData;
    HANDLE hFind = FindFirstFileA(sourcePath.c_str(), &lpFindData);
    HANDLE hSource = CreateFileA(sourcePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);    
    HANDLE hTarget = CreateFileA(targetPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "[error] : 复制文件" << sourcePath << "，源文件不存在" << endl;
        return false;
    }
    if (hSource == INVALID_HANDLE_VALUE)
    {
        cout << "[error] : 复制文件" << sourcePath << "， 源文件打开失败" << endl;
        return false;
    }
    if (hTarget == INVALID_HANDLE_VALUE)
    {
        cout << "[error] : 复制文件" << targetPath << "，目标文件打开失败" << endl;
        return false;
    }

    long long fileSize = lpFindData.nFileSizeLow - lpFindData.nFileSizeHigh;    // 计算文件大小
    int *buffer = new int[fileSize];
    DWORD wordBit;

    ReadFile(hSource, buffer, fileSize, &wordBit, NULL);        // 读取源文件
    WriteFile(hTarget, buffer, fileSize, &wordBit, NULL);       // 写入目标文件

    // 设置目标文件的时间和属性
    SetFileTime(hTarget, &lpFindData.ftCreationTime, &lpFindData.ftLastAccessTime, &lpFindData.ftLastWriteTime);
    SetFileAttributes(targetPath.c_str(), GetFileAttributes(sourcePath.c_str()));

    // 关闭句柄 释放资源
    CloseHandle(hFind);
    CloseHandle(hSource);
    CloseHandle(hTarget);

    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy file] : " << targetPath << " copy finished." << endl;
    }
    return true;
}

bool clear(string path)
{
    string nowPath = path + "\\*.*";
    WIN32_FIND_DATAA lpFindData;
    HANDLE hFind = FindFirstFileA(nowPath.c_str(), &lpFindData);
    do
    {
        if (!strcmp(lpFindData.cFileName, ".") || !strcmp(lpFindData.cFileName, "..")) continue;

        if (lpFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            clear(nextPath(path, lpFindData.cFileName));
        }
        else
        {
            DeleteFileA(nextPath(path, lpFindData.cFileName).c_str());
        }

    } while (FindNextFileA(hFind, &lpFindData)); 
    RemoveDirectoryA(path.c_str());
    return true;
}


bool copy_dir(string sourcePath, string targetPath, int depth)
{
    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy directory] : " << sourcePath << " copy start." << endl;
    }
        
    string nowPath = sourcePath + "\\*.*";
    WIN32_FIND_DATAA lpFindData;
    HANDLE hFind = FindFirstFileA(nowPath.c_str(), &lpFindData);
    do
    {
        if (!strcmp(lpFindData.cFileName, ".") || !strcmp(lpFindData.cFileName, "..")) continue;

        if (lpFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // 当前目录项是一个文件夹
        {
            string nexPath_source = nextPath(sourcePath, lpFindData.cFileName);
            string nexPath_target = nextPath(targetPath, lpFindData.cFileName);

            CreateDirectoryA(nexPath_target.c_str(), NULL);
            HANDLE hSource = CreateFileA(nexPath_source.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
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

            copy_dir(nexPath_source, nexPath_target, depth + 2);   // 递归复制子目录

            // 设置复制文件目录的时间和属性
            FILETIME createTime, accessTime, writeTime;
			GetFileTime(hSource, &createTime, &accessTime, &writeTime);
			SetFileTime(hTarget, &createTime, &accessTime, &writeTime);

			SetFileAttributes(nexPath_target.c_str(), GetFileAttributes(nexPath_source.c_str()));

            CloseHandle(hSource);   // 关闭句柄
            CloseHandle(hTarget);   // 关闭句柄

            
        }
        else    // 当前目录项是一个普通文件
        {
            string nexPath_source = nextPath(sourcePath, lpFindData.cFileName);
            string nexPath_target = nextPath(targetPath, lpFindData.cFileName);
            copy_file(nexPath_source, nexPath_target, depth + 1);  // 调用普通目录复制完成复制
        }
    }
    while (FindNextFileA(hFind, &lpFindData) != 0);     // 遍历目录项

    CloseHandle(hFind);     // 关闭句柄 释放资源
    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy directory] : " << targetPath << " copy finished." << endl;
    }
    return true;
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
    HANDLE hFind = FindFirstFileA(sourcePath.c_str(), &lpFindData);     // 检查源文件夹是否存在
    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "[error] : 源文件不存在." << endl;
        return 0;
    }
    CloseHandle(hFind);

    hFind = FindFirstFileA(targetPath.c_str(), &lpFindData);            // 检查目标文件夹是否存在
    if (hFind == INVALID_HANDLE_VALUE)
    {
        CreateDirectoryA(targetPath.c_str(), NULL);
    }
    
    copy_dir(sourcePath, targetPath, 0);

    HANDLE hSource = CreateFileA(sourcePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    HANDLE hTarget = CreateFileA(targetPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    FILETIME createTime, accessTime, writeTime;
    GetFileTime(hSource, &createTime, &accessTime, &writeTime);
    SetFileTime(hTarget, &createTime, &accessTime, &writeTime);

    SetFileAttributes(targetPath.c_str(), GetFileAttributes(sourcePath.c_str()));

    CloseHandle(hSource);
    CloseHandle(hTarget);
    return 0;
}   