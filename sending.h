// sending.h
#ifndef SENDING_H
#define SENDING_H

#include <windows.h>

void send_file(const wchar_t* filepath, const wchar_t* file_type);
void send_dir(const wchar_t* dir_path);
DWORD WINAPI sender_thread(LPVOID lpParam);

#endif // SENDING_H
