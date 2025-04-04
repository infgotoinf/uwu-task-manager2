#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <iostream>
#include <clocale>
#include <string>
#include <vector>

const char* PrintProcessInfo() {
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    std::string result;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        return "Error: Failed to enumerate processes";

    cProcesses = cbNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < cProcesses; i++)
    {
        DWORD processID = aProcesses[i];
        if (processID == 0) continue;

        wchar_t szProcessName[MAX_PATH] = L"<unknown>";
        HANDLE hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE,
            processID
        );

        if (hProcess != NULL)
        {
            HMODULE hMod;
            DWORD cbNeededModule;
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededModule))
            {
                GetModuleBaseNameW(
                    hProcess,
                    hMod,
                    szProcessName,
                    sizeof(szProcessName) / sizeof(wchar_t)
                );
            }
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
            {
                result += std::to_string(floor((float)pmc.WorkingSetSize / 104857.6) / 10);
                result.erase(result.find_last_not_of('0') + 1, std::string::npos);
                result.erase(result.find_last_not_of(',') + 1, std::string::npos);
                result += ":";
            }
            CloseHandle(hProcess);
        }
        else continue;

        // ����������� �� UTF-16 � UTF-8
        int size = WideCharToMultiByte(CP_UTF8, 0, szProcessName, -1, nullptr, 0, nullptr, nullptr);
        std::string processName;
        if (size > 0)
        {
            std::vector<char> buffer(size);
            WideCharToMultiByte(CP_UTF8, 0, szProcessName, -1, buffer.data(), size, nullptr, nullptr);
            processName = buffer.data();
        }
        else processName = "<unknown>";

        result += processName + "|" + std::to_string(processID) + "\n";
    }

    // �������� ������ ��� ����������
    char* output = new char[result.size() + 1];
    strcpy_s(output, result.size() + 1, result.c_str());
    return output;
}


bool KillProcessByPID(int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Couldn't open the process: " << pid << "\n";
        return false;
    }

    if (!TerminateProcess(hProcess, 0)) {
        std::cerr << "Error: couldn't kill the process: " << pid << "\n";
        CloseHandle(hProcess);
        return false;
    }
    return true;
}

int main() {
    setlocale(0, "");

    std::cout << PrintProcessInfo() << std::endl;

    int pid;
    std::cout << "������� PID ��������: ";
    std::cin >> pid;

    if (KillProcessByPID(pid))
        std::cout << "������� � PID " << pid << " ��������.\n";
    else
        std::cout << "�� ������� ��������� �������.\n";

    return 0;
}