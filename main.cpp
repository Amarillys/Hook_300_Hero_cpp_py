#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <TlHelp32.h>

typedef unsigned char int8;
#pragma comment(lib,"psapi.lib")

const DWORD start_addr = 0x400000;
using namespace std;
void memcopy_t(int8 * dst, int8 * src);
void memcopy_t2(int8 * dst, int8 * src, int size);
int existint8(int8 * string, int8 dst, int size);
inline bool id_cmp(int8 * dst, int8 * src, int size);

inline int existint8(int8 * string, int8 dst, int size)
{
    int last = -1;
    for (int i = 0; i < size; ++i)
    if (string[i] == dst && i > last)
        last = i;
    return last;
}

int maikaze_sunday(int8 *src, int8 *dst, const int len_s, const int len_d)
{
    if (len_s < len_d)
        return -1;

    int pos = 0, exist_pos = 0;
    int time;
    int src_pos = 0, dst_pos = 0;
    bool same;
    while (src_pos < len_s)
    {
        //judge if same
        same = true;
        time = 0;
        for (int i = 0; i < len_d; ++i)
        {
            if (src[src_pos + time] != dst[dst_pos + time])
            {
                same = false;
                break;
            }
            time++;
        }
        if (same)
            return src_pos;
        src_pos = src_pos + len_d - existint8(dst, src[src_pos + len_d], len_d);
        dst_pos = 0;
    }
    return -1;
}

BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
    )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup 
        &luid))        // receives LUID of privilege
    {
        printf("LookupPrivilegeValue error: %u/n", GetLastError());
        return FALSE;
    }
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL))
    {
        printf("AdjustTokenPrivileges error: %u/n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        printf("The token does not have the specified privilege. /n");
        return FALSE;
    }
    return TRUE;
}

HANDLE GetProcessHandle(int nID)
{
    HANDLE hToken;
    BOOL flag = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
    if (!flag)
    {
        DWORD err = GetLastError();
        printf("OpenProcessToken error:%d", err);
    }
    SetPrivilege(hToken, SE_DEBUG_NAME, true);
    CloseHandle(hToken);
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}

HANDLE GetProcessHandle(LPCTSTR pName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot)
    {
        return NULL;
    }
    PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL fOk;
    for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
    {
        if (!_tcscmp(pe.szExeFile, pName))
        {
            CloseHandle(hSnapshot);
            return GetProcessHandle(pe.th32ProcessID);
        }
    }
    return NULL;
}

int main()
{
    BOOL dbg;
    //Case 1:ReadMemory
    /*
    HANDLE hProcess = GetProcessHandle(L"300.exe");
    if (!hProcess)
    {
        cout << "failed to read process 300.exe" << endl;
        getchar();
        return 1;
    }
    //Get the sum of memory token
    
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
    DWORD ReqSize = pmc.PagefileUsage - start_addr;
    */
    /*
    DWORD ReqSize = 1280 * 1024 * 1024;
    int8 * memget = new int8[ReqSize];
    int8 * base_addr = memget;
    DWORD cur_addr = start_addr, ReadSize;
    MEMORY_BASIC_INFORMATION meminf;
    DWORD dwInfoSize = sizeof(MEMORY_BASIC_INFORMATION);   
    //Read all parts of Memory
    while (true)
    {
        if (VirtualQueryEx(hProcess, (LPVOID)cur_addr, &meminf, dwInfoSize) == 0)
            break;
        if (!(meminf.State == MEM_COMMIT || meminf.State == MEM_IMAGE || meminf.State == MEM_MAPPED))
        {
            cur_addr = (DWORD)meminf.BaseAddress + meminf.RegionSize;
            continue;
        }
        if ((dbg = ReadProcessMemory(hProcess, (LPCVOID)meminf.BaseAddress, memget, meminf.RegionSize, &ReadSize)) == false)
            cout << "Failed to read memory at address:" << meminf.BaseAddress << endl;
        else
            memget += meminf.RegionSize;
        cur_addr = (DWORD)meminf.BaseAddress + meminf.RegionSize;
    }
    */
    //debug   
    /*
    ReqSize = memget - base_addr;
    memget = base_addr;
    FILE * log = fopen("d:/300.dmp", "wb");
    if (log == NULL)
        cout << "failed to create dmp" << endl;
    fwrite(memget, ReqSize, 1, log);
    fclose(log);
    */

    //case 2:ReadDump
    FILE * dmp = fopen("d:/300.DMP", "rb");
    fseek(dmp, 0, SEEK_END);
    DWORD ReqSize = ftell(dmp);
    int8 * memget = new int8[ReqSize];
    fseek(dmp, 0, SEEK_SET);
    fread(memget, ReqSize, 1, dmp);
    int8 * base_addr = memget;
    
    //DWORD dwOldProt, dwNewProt = 0;
    //cout << "   S:" << GetLastError() << endl;
    //if (ReqSize == 0)
    //    cout << "you need higher permission to run this program" << endl;
    int8 enemy[9] = { 0x15, 0x01, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00 };
    const int8 relative_pos = 19;
    int8 id[14][16] = { 0 };
    int tmp_addr, psize = ReqSize;
    memget = base_addr;
    int ally_player = 0, enemy_player = 7;
    int id_len = 0;
    while ((tmp_addr = maikaze_sunday(memget, enemy, psize, 9)) > 0)
    {
        id_len = *((int8 *)(memget + tmp_addr + relative_pos + 16));
        if (id_len > 14)
            break;
        memget += tmp_addr + relative_pos;
        psize = psize - (int)tmp_addr - relative_pos;
        if (*(memget - 1) == 0)
        {
            memcopy_t2(id[enemy_player++], memget, id_len); 
            cout << static_cast<const void *>(memget) << "-> Enemy: " << id[enemy_player - 1] << endl;
        }
        else
        {
            memcopy_t2(id[ally_player++], memget, id_len);
            cout << static_cast<const void *>(memget) << "-> Ally : " << id[ally_player - 1] << endl;
        }
    }
    //execute the python program to deal the final result
    char cmd[255] = "ListPlayer.py";
    char tmp[16][16] = { 0 };
    for (int i = 0; i < 14; ++i)
    {        
        strcpy(tmp[i], (const char *)id[i]);
        strcat(cmd, " ");
        strcat(cmd, tmp[i]);
    }
    //printf(cmd);
    memget = base_addr;
    delete memget;
    system(cmd);
    getchar();
}

void memcopy_t(int8 * dst, int8 * src)
{
    for (int i = 0; src[i] != 0 && i < 14; ++i)
        dst[i] = src[i];
}

void memcopy_t2(int8 * dst, int8 * src, int size)
{
    for (int i = 0; i < 14 && i < size; ++i)
        dst[i] = src[i];
}


bool id_cmp(int8 * dst, int8 * src, int size)
{
    bool ret = true;
    for (int i = 0; i < size; ++i)
    if (dst[i] != src[i])
    {
        ret = false;
        return ret;
    }
    return ret;
}
