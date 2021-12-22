#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <utime.h>
#include <unistd.h>

using namespace std;


bool showDetail = false;

bool isDir(string path)
{
    // 判断一个路径是否是目录
    struct stat fileInfo;
    stat(path.c_str(), &fileInfo);
    if (S_ISDIR(fileInfo.st_mode)) return true;
    return false;
}

string nextPath(string now, string nex)
{
    // 获取下一级路径
    return now + "/" + nex;
}

void clear(string path)
{
    // 将path对应的目录清空

    DIR *dir = opendir(path.c_str());   // 打开目录
    dirent *dirItem;
    while (dirItem = readdir(dir))      // 遍历目录中的每一个目录项
    {
        if (!strcmp(dirItem->d_name, ".") || !strcmp(dirItem->d_name, "..")) continue;
        
        string nexPath = nextPath(path, dirItem->d_name);
        if (isDir(nexPath))     // 如果是一个子目录，则递归清空
        {
            clear(nexPath);
        }
        else
        {
            remove(nexPath.c_str());    // 如果是一个普通文件，则直接删除
        }
    }
    rmdir(path.c_str());    // 删除当前这个空白文件目录
}

bool copy_softLink(string sourcePath, string targetPath, int depth)
{
    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[soft link] : " << sourcePath << " start copy." << endl;
    }
        
    char buffer[4096];
    readlink(sourcePath.c_str(), buffer, 4096);
    
    bool flag = false;
    if (buffer[0] == '.') flag = true;

    if (flag)
        realpath(sourcePath.c_str(), buffer);
    
    symlink(buffer, targetPath.c_str());

    struct stat sourceFileInfo;
    struct timeval ftime[2];

    lstat(sourcePath.c_str(), &sourceFileInfo);
    ftime[0].tv_usec = 0;
    ftime[0].tv_sec = sourceFileInfo.st_atime;
    ftime[1].tv_usec = 0;
    ftime[1].tv_sec = sourceFileInfo.st_mtime;
    lutimes(targetPath.c_str(), ftime);

    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[soft link] : " << sourcePath << " copy finished." << endl;
    }    
    return true;
}


bool copy_file(string sourcePath, string targetPath, int depth)
{
    
    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy file] : " << sourcePath << " start copy." << endl;
    }
        
    int sourceFile, targetFile;
    if ((sourceFile = open(sourcePath.c_str(), O_RDONLY)) == -1)    // 打开源文件
    {
        cout << "[error] : 复制" << sourcePath << "文件，打开文件错误." << endl;
        return false;
    }

    struct stat sourceFileInfo;
    struct utimbuf utimeInfo;
    
    stat(sourcePath.c_str(), &sourceFileInfo);
    
    if ((targetFile = creat(targetPath.c_str(), sourceFileInfo.st_mode)) == -1)     // 创建目标文件
    {
        cout << "[error] : 复制" << sourcePath << "文件，创建文件错误." << endl;
        return false;
    }

    int inputSize, outputSize;
    char buffer[4096];
    while ((inputSize = read(sourceFile, buffer, 4096)) > 0)    // 从源文件中读
    {
        outputSize = write(targetFile, buffer, inputSize);      // 向目标文件写
        if (inputSize != outputSize)
        {
            cout << "[error] : 复制" << sourcePath << "文件，写文件错误." << endl;
            return false;
        }
    }

    utimeInfo.actime = sourceFileInfo.st_atime;
    utimeInfo.modtime = sourceFileInfo.st_mtime;
    utime(targetPath.c_str(), &utimeInfo);  // 修改目标文件的时间

    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy file] : " << sourcePath << " copy finished." << endl;
    }
        
    close(sourceFile);  // 关闭源文件
    close(targetFile);  // 关闭目标文件
    return true;
}


bool copy_dir(string sourcePath, string targetPath, int depth)
{
    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy directory] : " << sourcePath << " start copy." << endl;
    }
        
    DIR *nowDir = opendir(sourcePath.c_str());
    dirent *dirItem;
    while (dirItem = readdir(nowDir))   // 遍历目录项
    {
        if (!strcmp(dirItem->d_name, ".") || !strcmp(dirItem->d_name, "..")) continue;
        if (dirItem->d_type == 10)
        {
            string nexPath_source = nextPath(sourcePath, dirItem->d_name);
            string nexPath_target = nextPath(targetPath, dirItem->d_name);
            bool ret = copy_softLink(nexPath_source, nexPath_target, depth + 1);
            if (!ret) return false;
        }
        else if (isDir(nextPath(sourcePath, dirItem->d_name)))   // 如果当前目录项是一个子目录
        {
            string nexPath_source = nextPath(sourcePath, dirItem->d_name);
            string nexPath_target = nextPath(targetPath, dirItem->d_name);
            DIR *nexDir = opendir(nexPath_target.c_str());
            if (nexDir == NULL)     // 创建目标子目录，由于事先清空此处一定为空
            {
                struct stat sourceFileInfo;
                struct utimbuf utimeInfo;

                stat(nexPath_source.c_str(), &sourceFileInfo);
                mkdir(nexPath_target.c_str(), sourceFileInfo.st_mode);
            }
            copy_dir(nexPath_source, nexPath_target, depth + 2);   // 递归复制子目录

            struct stat sourceFileInfo;
            struct utimbuf utimeInfo;
            stat(nexPath_source.c_str(), &sourceFileInfo);
            utimeInfo.actime = sourceFileInfo.st_atime;
            utimeInfo.modtime = sourceFileInfo.st_mtime;
            utime(nexPath_target.c_str(), &utimeInfo);
        }
        else    // 当前目录项是一个普通文件
        {
            string nexPath_source = nextPath(sourcePath, dirItem->d_name);
            string nexPath_target = nextPath(targetPath, dirItem->d_name);
            bool ret = copy_file(nexPath_source, nexPath_target, depth + 1);   // 调用文件复制进行复制
            if (!ret) return false;
        }
    }

    if (showDetail)
    {
        for (int i = 1; i <= 4 * depth; ++i) cout << "-";
        cout << "[copy directory] : " << sourcePath << " copy finished." << endl;
    }

    closedir(nowDir);
    return true;
}



int main(int argc, char * argv[])
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

    string sourceDir = argv[1];
    string targetDir = argv[2];
    DIR *dir = opendir(sourceDir.c_str());
    if (dir == NULL)
    {
        cout << "[error] : 待复制文件夹错误." << endl;
        return 0;
    }

    dir = opendir(targetDir.c_str());
    if (dir == NULL)
    {
        struct stat sourceFileInfo;
        stat(sourceDir.c_str(), &sourceFileInfo);
        mkdir(targetDir.c_str(), sourceFileInfo.st_mode);
    }


    copy_dir(sourceDir, targetDir, 0); // 开始复制

    struct utimbuf utimeInfo;
    struct stat sourceFileInfo;
    stat(sourceDir.c_str(), &sourceFileInfo);
    utimeInfo.actime = sourceFileInfo.st_atime;
    utimeInfo.modtime = sourceFileInfo.st_mtime;
    utime(targetDir.c_str(), &utimeInfo);

    return 0;
}