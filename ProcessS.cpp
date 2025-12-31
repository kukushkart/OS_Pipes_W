#include <windows.h>
#include <stdio.h>
#include <string>

int main(int argc, char* argv[])
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    char buffer[4096];
    DWORD bytesRead, bytesWritten;

    if (ReadFile(hInput, buffer, sizeof(buffer) - 1, &bytesRead, NULL))
    {
        buffer[bytesRead] = '\0';

        std::string input(buffer);
        long long sum = 0;

        size_t pos = 0;
        while (pos < input.length())
        {
            while (pos < input.length() && (input[pos] == ' ' || input[pos] == '\r'))
                pos++;

            if (pos >= input.length() || input[pos] == '\n')
                break;

            long long num = 0;
            bool negative = false;
            if (input[pos] == '-')
            {
                negative = true;
                pos++;
            }

            while (pos < input.length() && input[pos] >= '0' && input[pos] <= '9')
            {
                num = num * 10 + (input[pos] - '0');
                pos++;
            }

            if (negative) num = -num;

            sum += num;
        }

        char result[64];
        sprintf_s(result, sizeof(result), "%lld\n", sum);
        DWORD length = static_cast<DWORD>(strlen(result));
        WriteFile(hOutput, result, length, &bytesWritten, NULL);
    }

    return 0;
}