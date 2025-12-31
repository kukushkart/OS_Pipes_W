#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

bool ValidateInput(const std::string& input, std::string& errorMessage) {
    std::istringstream iss(input);
    std::string token;
    int count = 0;

    while (iss >> token) {
        count++;

        bool hasDigits = false;
        for (size_t i = 0; i < token.length(); i++) {
            if (i == 0 && token[i] == '-') {
                continue;
            }
            if (!isdigit(token[i])) {
                errorMessage = "Error: " + token + " is not a valid number";
                return false;
            }
            hasDigits = true;
        }

        if (!hasDigits && token == "-") {
            errorMessage = "Error: " + token + " is not a valid number";
            return false;
        }
    }

    if (count == 0) {
        errorMessage = "Error: No numbers entered";
        return false;
    }

    return true;
}

bool RunProcess(const std::wstring& appName, HANDLE hStdIn, HANDLE hStdOut, PROCESS_INFORMATION& pi) {
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hStdIn;
    si.hStdOutput = hStdOut;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    ZeroMemory(&pi, sizeof(pi));

    std::vector<wchar_t> cmdLine(appName.begin(), appName.end());
    cmdLine.push_back(0);

    if (!CreateProcess(NULL, cmdLine.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        std::wcerr << L"Failed to create process: " << appName << std::endl;
        return false;
    }
    return true;
}

int main() {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hPipe1_Read, hPipe1_Write;
    HANDLE hPipe2_Read, hPipe2_Write;
    HANDLE hPipe3_Read, hPipe3_Write;
    HANDLE hPipe4_Read, hPipe4_Write;
    HANDLE hPipe5_Read, hPipe5_Write;

    if (!CreatePipe(&hPipe1_Read, &hPipe1_Write, &sa, 0) ||
        !CreatePipe(&hPipe2_Read, &hPipe2_Write, &sa, 0) ||
        !CreatePipe(&hPipe3_Read, &hPipe3_Write, &sa, 0) ||
        !CreatePipe(&hPipe4_Read, &hPipe4_Write, &sa, 0) ||
        !CreatePipe(&hPipe5_Read, &hPipe5_Write, &sa, 0)) {
        std::cerr << "CreatePipe failed" << std::endl;
        return 1;
    }

    PROCESS_INFORMATION piM, piA, piP, piS;

    if (!RunProcess(L"ProcessM.exe", hPipe1_Read, hPipe2_Write, piM)) {
        CloseHandle(hPipe1_Read); CloseHandle(hPipe1_Write);
        CloseHandle(hPipe2_Read); CloseHandle(hPipe2_Write);
        CloseHandle(hPipe3_Read); CloseHandle(hPipe3_Write);
        CloseHandle(hPipe4_Read); CloseHandle(hPipe4_Write);
        CloseHandle(hPipe5_Read); CloseHandle(hPipe5_Write);
        return 1;
    }
    CloseHandle(hPipe1_Read);
    CloseHandle(hPipe2_Write);

    if (!RunProcess(L"ProcessA.exe", hPipe2_Read, hPipe3_Write, piA)) {
        CloseHandle(piM.hProcess); CloseHandle(piM.hThread);
        CloseHandle(hPipe2_Read); CloseHandle(hPipe3_Write);
        CloseHandle(hPipe3_Read); CloseHandle(hPipe4_Write);
        CloseHandle(hPipe4_Read); CloseHandle(hPipe5_Write);
        CloseHandle(hPipe5_Read); CloseHandle(hPipe1_Write);
        return 1;
    }
    CloseHandle(hPipe2_Read);
    CloseHandle(hPipe3_Write);

    if (!RunProcess(L"ProcessP.exe", hPipe3_Read, hPipe4_Write, piP)) {
        CloseHandle(piM.hProcess); CloseHandle(piM.hThread);
        CloseHandle(piA.hProcess); CloseHandle(piA.hThread);
        CloseHandle(hPipe3_Read); CloseHandle(hPipe4_Write);
        CloseHandle(hPipe4_Read); CloseHandle(hPipe5_Write);
        CloseHandle(hPipe5_Read); CloseHandle(hPipe1_Write);
        return 1;
    }
    CloseHandle(hPipe3_Read);
    CloseHandle(hPipe4_Write);

    if (!RunProcess(L"ProcessS.exe", hPipe4_Read, hPipe5_Write, piS)) {
        CloseHandle(piM.hProcess); CloseHandle(piM.hThread);
        CloseHandle(piA.hProcess); CloseHandle(piA.hThread);
        CloseHandle(piP.hProcess); CloseHandle(piP.hThread);
        CloseHandle(hPipe4_Read); CloseHandle(hPipe5_Write);
        CloseHandle(hPipe5_Read); CloseHandle(hPipe1_Write);
        return 1;
    }
    CloseHandle(hPipe4_Read);
    CloseHandle(hPipe5_Write);

    std::string inputLine;
    std::string errorMessage;
    bool validInput = false;

    while (!validInput) {
        std::cout << "Enter numbers separated by space (press Enter to finish): ";
        std::getline(std::cin, inputLine);

        if (ValidateInput(inputLine, errorMessage)) {
            validInput = true;
        }
        else {
            std::cout << errorMessage << std::endl;
        }
    }

    inputLine += "\n";

    DWORD bytesWritten;
    DWORD length = static_cast<DWORD>(inputLine.length());
    WriteFile(hPipe1_Write, inputLine.c_str(), length, &bytesWritten, NULL);

    CloseHandle(hPipe1_Write);

    WaitForSingleObject(piS.hProcess, INFINITE);

    char buffer[128];
    DWORD bytesRead;
    std::string resultStr = "";

    while (ReadFile(hPipe5_Read, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        resultStr += buffer;
    }
    CloseHandle(hPipe5_Read);

    std::cout << "Result: " << resultStr << std::endl;

    CloseHandle(piM.hProcess); CloseHandle(piM.hThread);
    CloseHandle(piA.hProcess); CloseHandle(piA.hThread);
    CloseHandle(piP.hProcess); CloseHandle(piP.hThread);
    CloseHandle(piS.hProcess); CloseHandle(piS.hThread);

    system("pause");
    return 0;
}