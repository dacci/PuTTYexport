/*
 * Copyright (C) 2011 dacci.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include <atlbase.h>

typedef void (*PFNPRINTCALLBACK)(char*, PBYTE, DWORD);

void print_callback(char* szName, PBYTE pData, DWORD dwType) {
  if (dwType == 0xFFFFFF) {
    ::printf("[%s]\n", szName);
    return;
  }

  ::printf("%s=", szName);

  if (dwType == REG_DWORD) {
    ::printf("%d", *reinterpret_cast<DWORD*>(pData));
  } else {
    ::printf("%s", pData);
  }

  ::printf("\n");
}

void print_sessions_callback(char* szName, PBYTE pData, DWORD dwType) {
  if (dwType == 0xFFFFFF) {
    ::printf("[Session:%s]\n", szName);
    return;
  }

  ::printf("%s=", szName);

  if (dwType == REG_DWORD) {
    ::printf("%d", *reinterpret_cast<DWORD*>(pData));
  } else {
    ::printf("%s", pData);
  }

  ::printf("\n");
}

void export_key(const CRegKey& _regKey, char* szKeyName, PFNPRINTCALLBACK pfn) {
  CRegKey regKey;
  if (regKey.Open(_regKey, szKeyName)) {
    return;
  }

  pfn(szKeyName, NULL, 0xFFFFFF);

  char szName[256];
  DWORD dwLength, dwSize, dwType;
  BYTE pData[1024];

  for (DWORD dwIndex = 0; ; dwIndex++) {
    dwLength = _countof(szName);
    dwSize = _countof(pData);
    if (::RegEnumValue(regKey, dwIndex, szName, &dwLength, NULL, &dwType, pData,
                       &dwSize)) {
      break;
    }

    pfn(szName, pData, dwType);
  }

  ::printf("\n");
}

void export_sessions(const CRegKey& _regKey, char* szKeyName) {
  CRegKey regKey;
  if (regKey.Open(_regKey, szKeyName)) {
    return;
  }

  char szName[256];
  DWORD dwLength;

  for (DWORD iIndex = 0; ; iIndex++) {
    dwLength = sizeof(szName) / sizeof(*szName);
    if (regKey.EnumKey(iIndex, szName, &dwLength)) {
      break;
    }

    ::export_key(regKey, szName, print_sessions_callback);
  }
}

int main(int argc, char* argv[]) {
  CRegKey regKey;
  if (regKey.Open(HKEY_CURRENT_USER, "Software\\SimonTatham\\PuTTY")) {
    return -1;
  }

  ::printf("[Generic]\nUseIniFile=1\n\n");

  char szName[256];
  DWORD dwLength;

  for (DWORD iIndex = 0; ; iIndex++) {
    dwLength = sizeof(szName) / sizeof(*szName);
    if (regKey.EnumKey(iIndex, szName, &dwLength)) {
      break;
    }

    if (_strcmpi(szName, "Sessions")) {
      ::export_key(regKey, szName, print_callback);
    }
  }

  ::export_sessions(regKey, "Sessions");

  return 0;
}
