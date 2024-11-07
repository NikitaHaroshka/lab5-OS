#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

const int MAX_MESSAGE_LENGTH = 20;
#define FILE_NAME "messages.dat"

// Указатели на массивы событий для каждого Sender
HANDLE* hReadyToSend = nullptr;
HANDLE* hMessageWritten = nullptr;
HANDLE hReceiverReady = nullptr;

// Функция для создания бинарного файла
void CreateFileForMessages(const std::string& fileName, int numEntries) {
    std::ofstream file(fileName, std::ios::binary);
    for (int i = 0; i < numEntries; ++i) {
        char empty[MAX_MESSAGE_LENGTH] = { 0 };
        file.write(empty, sizeof(empty));
    }
}

// Функция ожидания сигналов от всех Sender
void WaitForSenders(int numSenders) {
    for (int i = 0; i < numSenders; ++i) {
        WaitForSingleObject(hReadyToSend[i], INFINITE); // Ожидание готовности каждого Sender
    }
}

// Чтение сообщения из файла
void ReadMessageFromFile(const std::string& fileName, int index) {
    std::ifstream file(fileName, std::ios::binary);
    file.seekg(index * MAX_MESSAGE_LENGTH, std::ios::beg);
    char message[MAX_MESSAGE_LENGTH];
    file.read(message, MAX_MESSAGE_LENGTH);
    std::cout << "Received message: " << message << std::endl;
}

int main() {
    int numEntries, numSenders;

    // Ввод данных с консоли
    std::cout << "Enter the number of records in the binary file: ";
    std::cin >> numEntries;

    // Создание файла сообщений
    CreateFileForMessages(FILE_NAME, numEntries);

    // Ввод количества процессов Sender
    std::cout << "Enter the number of Sender processes: ";
    std::cin >> numSenders;

    // Выделяем динамическую память для массивов событий
    hReceiverReady = CreateEvent(NULL, TRUE, FALSE, NULL);  // Для синхронизации с Receiver
    if (hReceiverReady == NULL) {
        std::cerr << "CreateEvent failed for hReceiverReady. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    // Динамическое выделение памяти для массивов событий
    hReadyToSend = new HANDLE[numSenders];
    hMessageWritten = new HANDLE[numSenders];

    // Создание событий для каждого Sender
    for (int i = 0; i < numSenders; ++i) {
        hReadyToSend[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        hMessageWritten[i] = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (hReadyToSend[i] == NULL || hMessageWritten[i] == NULL) {
            std::cerr << "CreateEvent failed for Sender events. Error code: " << GetLastError() << std::endl;
            return 1;
        }
    }

    // Ожидание готовности всех процессов Sender
    WaitForSenders(numSenders);

    // Чтение сообщений по команде
    int messageIndex = 0;
    while (true) {
        std::string command;
        std::cout << "Enter command (read/exit): ";
        std::cin >> command;
        if (command == "read") {
            ReadMessageFromFile(FILE_NAME, messageIndex);
            messageIndex = (messageIndex + 1) % numEntries;
        }
        else if (command == "exit") {
            break;
        }
    }

    // Освобождение ресурсов
    CloseHandle(hReceiverReady);
    for (int i = 0; i < numSenders; ++i) {
        CloseHandle(hReadyToSend[i]);
        CloseHandle(hMessageWritten[i]);
    }

    // Освобождение динамически выделенной памяти
    delete[] hReadyToSend;
    delete[] hMessageWritten;

    return 0;
}
