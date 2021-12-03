/*
 * @author: 0x404
 * @Date: 2021-12-03 13:46:04
 * @LastEditTime: 2021-12-03 16:25:41
 * @Description: 
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <iomanip>
#include <shlwapi.h>
#include <psapi.h>
#include <TlHelp32.h>

#define SHOWMASK(dwTarget,type) if(TestSet(dwTarget,PAGE_##type)) {cout << #type;}

using namespace std;

bool TestSet(DWORD dwTarget, DWORD dwMask)
{
    return ((dwTarget & dwMask) == dwMask);
}

void showSysInfo()
{
    cout << "------------------------------------------------" << endl;
    SYSTEM_INFO sysInfo;
    memset(&sysInfo, 0, sizeof sysInfo);
    GetSystemInfo(&sysInfo);

    cout << "\t[核心数量] : " << sysInfo.dwNumberOfProcessors << endl;
    cout << "\t[分页大小] : " << sysInfo.dwPageSize << endl;
    cout << setw(10) << "\t[最小地址] : " << sysInfo.lpMinimumApplicationAddress << endl;
    cout << setw(10) << "\t[最大地址] : " << sysInfo.lpMaximumApplicationAddress << endl;
    
    cout << "------------------------------------------------" << endl;
}

void showSysMemInfo()
{
    cout << "------------------------------------------------" << endl;
    MEMORYSTATUSEX memSta;
    memset(&memSta, 0, sizeof memSta);
    memSta.dwLength = sizeof memSta;
    GlobalMemoryStatusEx(&memSta);

    cout << setw(10) << "\t[物理内存容量] : " << double(memSta.ullTotalPhys) / 1024 / 1024 / 1024 << "GB" <<endl;
    cout << setw(10) << "\t[可用物理内存] : " << double(memSta.ullAvailPhys) / 1024 / 1024 / 1024 << "GB" << endl;
    cout << setw(10) << "\t[虚拟内存容量] : " << double(memSta.ullTotalVirtual) / 1024 / 1024 / 1024 << "GB" << endl;
    cout << setw(10) << "\t[可用虚拟内存] : " << double(memSta.ullAvailVirtual) / 1024 / 1024 / 1024 << "GB" << endl;

    cout << "------------------------------------------------" << endl;
}

void showSysPerformanceInfo()
{
    cout << "------------------------------------------------" << endl;
    PERFORMACE_INFORMATION perInfo;
    memset(&perInfo, 0, sizeof perInfo);
    perInfo.cb = sizeof perInfo;

    GetPerformanceInfo(&perInfo, sizeof perInfo);
    cout << "\t[当前页面总数] : " << perInfo.CommitTotal << endl;
    cout << "\t[最大页面总数] : " << perInfo.CommitLimit << endl;
    cout << "\t[历史页面峰值] : " << perInfo.CommitPeak << endl;
    cout << "\t[物理内存总数] : " << perInfo.PhysicalTotal << endl;
    cout << "\t[可用物理内存] : " << perInfo.PhysicalAvailable << endl;
    cout << "\t[缓存页的数量] : " << perInfo.SystemCache << endl;
    cout << "\t[打开句柄数量] : " << perInfo.HandleCount << endl;
    cout << "\t[当前线程数量] : " << perInfo.ThreadCount << endl;
    cout << "\t[当前进程数量] : " << perInfo.ProcessCount << endl;
    cout << "------------------------------------------------" << endl;
}

void showSysProcessInfo(int limit = 10)
{
    cout << "------------------------------------------------" << endl;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    PROCESSENTRY32 ps;
    
    memset(&si, 0, sizeof si);
    memset(&pi, 0, sizeof pi);
    memset(&ps, 0, sizeof ps);
    si.cb = sizeof(STARTUPINFO);
    ps.dwSize = sizeof (PROCESSENTRY32);

    HANDLE hSnapshoot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshoot == INVALID_HANDLE_VALUE || !Process32First(hSnapshoot, &ps)) return;

    int counter = 0;
    do
    {
        cout << "\t[pid = " << ps.th32ProcessID << "] " << ps.szExeFile << endl;
        ++ counter;
    } while (Process32Next(hSnapshoot, &ps) && counter < limit);
    cout << "------------------------------------------------" << endl;
}

void ShowProtection(DWORD dwTarget)
{
    SHOWMASK(dwTarget, READONLY);
    SHOWMASK(dwTarget, GUARD);
    SHOWMASK(dwTarget, NOCACHE);
    SHOWMASK(dwTarget, READWRITE);
    SHOWMASK(dwTarget, WRITECOPY);
    SHOWMASK(dwTarget, EXECUTE_READ);
    SHOWMASK(dwTarget, EXECUTE);
    SHOWMASK(dwTarget, EXECUTE_READWRITE);
    SHOWMASK(dwTarget, EXECUTE_WRITECOPY);
    SHOWMASK(dwTarget, NOACCESS);
}


void showProcessVMInfo(int pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
    if (hProcess == NULL)
    {
        cout << "[error] : pid error." << endl;
        return;
    }
    
    SYSTEM_INFO sysInfo;
    MEMORY_BASIC_INFORMATION mbi;

    memset(&sysInfo, 0, sizeof sysInfo);
    memset(&mbi, 0, sizeof mbi);
    GetSystemInfo(&sysInfo);

    LPCVOID pStart = sysInfo.lpMinimumApplicationAddress;
    while (pStart < sysInfo.lpMaximumApplicationAddress)
    {
        if (VirtualQueryEx(hProcess, pStart, &mbi, sizeof mbi) == sizeof mbi)
        {
            LPCVOID pEnd = (PBYTE)pStart + mbi.RegionSize;
            cout << "\t" << pStart << " - " << pEnd <<" \t";
            cout << setw(5) << "[大小 = " << double(mbi.RegionSize) / 1024 / 1024 << "MB]\t";
            switch (mbi.State)
            {
                case MEM_COMMIT:
                    cout << "[状态=已提交]" << "\t";
                    break;
                case MEM_FREE:
                    cout << "[状态=空闲]" << "\t";
                    break;
                case MEM_RESERVE:
                    cout << "[状态=预留]" << "\t";
                    break;
                default:
                    break;
            }
            if (mbi.Protect == 0 && mbi.State != MEM_FREE)
                mbi.Protect = PAGE_READONLY;

            ShowProtection(mbi.Protect);
            cout << "\t";

            switch (mbi.Type)
            {
                case MEM_IMAGE:
                    cout << "[IMAGED]\t";
                    break;
                case MEM_MAPPED:
                    cout << "[MAPPED]\t";
                    break;
                case MEM_PRIVATE:
                    cout << "[PRIVATE]\t";
                    break;
                default:
                    break;
            }

            TCHAR szFilename[MAX_PATH];
            if (GetModuleFileName((HINSTANCE)pStart, szFilename, MAX_PATH) > 0)
            {
                PathStripPath(szFilename);
                printf("\t %s", szFilename);
            }
            printf("\n");
            pStart=pEnd;
        }
    }

}

void showMenu()
{
    cout << "[top] : 系统信息" << endl;
    cout << "[query pid] : 查询id为pid的进程信息" << endl;
}


int main()
{
    while(1)
    {
        string cmd;
        cout << "> ";
        cin >> cmd;
        if (cmd == "top")
        {
            while (1)
            {
                system("cls");
                showSysInfo();
                showSysMemInfo();
                showSysPerformanceInfo();
                showSysProcessInfo(10);
                Sleep(3000);
            }
        }
        else if (cmd.substr(0, 3) == "top")
        {
            string num = cmd.substr(3);
            int x = 0;
            for (int i = 0; i < num.length(); ++i)
                x = x * 10 + num[i] - '0';
            while (1)
            {
                system("cls");
                showSysInfo();
                showSysMemInfo();
                showSysPerformanceInfo();
                showSysProcessInfo(x);
                Sleep(3000);
            }
        }
        else if (cmd == "query")
        {
            int pid;
            cin >> pid;
            showProcessVMInfo(pid);
        }
        else
        {
            showMenu();
        }
    }
    
    
    return 0;
}
