// Driver.cpp
//
#include "stdafx.h"
#include "string.h"
#include <windows.h>

#define PC_DEBUG_MODE			TRUE //FALSE  
#define MAX_KEY_LENGTH			255
#define MAX_VALUE_NAME			16383
#define REG_DRIVERS_BUILTIN		"Drivers\\Builtin\\"


// placeholders for use when debugging on a non Windows CE PC workstation
#if PC_DEBUG_MODE
typedef struct _DevmgrDeviceInformation_tag {
    DWORD dwSize;                   // size of this structure
    HANDLE hDevice;                 // device handle from ActivateDevice()
    HANDLE hParentDevice;           // parent device's handle from ActivateDevice()
    WCHAR szLegacyName[6];          // e.g, "COM1:"
    WCHAR szDeviceKey[MAX_PATH];    // key path passed to ActivateDevice
    WCHAR szDeviceName[MAX_PATH];   // in $device namespace
    WCHAR szBusName[MAX_PATH];      // in $bus namespace
} DEVMGR_DEVICE_INFORMATION, *PDEVMGR_DEVICE_INFORMATION;

typedef enum {
    DeviceSearchByLegacyName,
    DeviceSearchByDeviceName,
    DeviceSearchByBusName,
    DeviceSearchByGuid,
    DeviceSearchByParent
} DeviceSearchType;

HANDLE ActivateDeviceEx(LPCWSTR lpszDevKey, LPCVOID lpRegEnts, DWORD cRegEnts, LPVOID lpvParam)
{
    return NULL;
}

BOOL DeactivateDevice(__in_opt HANDLE hDevice)
{
    return FALSE;
}

HANDLE FindFirstDevice(DeviceSearchType searchType, LPCVOID pvSearchParam, __out PDEVMGR_DEVICE_INFORMATION pdi)
{
    return NULL;
}

BOOL FindNextDevice(HANDLE h, __out PDEVMGR_DEVICE_INFORMATION pdi)
{
    return FALSE;
}

#endif

enum CommandType
{
    LoadCommand,
    UnloadCommand,
    ReloadCommand,
    ListDriversCommand,
    ListRegistryValuesCommand,
    SetRegistryValueCommand,
    InvalidCommand
};

BOOL AreEqual(const char *a, const char *b)
{
    // note stricmp is case insensitive string compare
    if (_stricmp(a, b) == 0) {
        return TRUE;
    }

    return FALSE;
}

BOOL StartsWith(const char *string, const char *substring)
{
    if (strncmp(string, substring, strlen(substring)) == 0) {
        return TRUE;
    }

    return FALSE;
}

// Returns a value indicating whether or not the specified character is contained in the 
// specified string
BOOL Contains(const char *string, const char character)
{
    const char* pPosition = strchr(string, character);
    return (pPosition != NULL);
}

// Convert char array to wchar_t array
// http://stackoverflow.com/questions/3074776/how-to-convert-char-array-to-wchar-t-array
static wchar_t* CharToWChar(const char* text)
{
    size_t size = strlen(text) + 1;
    wchar_t* wa = new wchar_t[size];
    mbstowcs(wa,text,size);
    return wa;
}

CommandType GetCommandType(char* parameter)
{	
    if ((AreEqual(parameter, "-load")) 
        || (AreEqual(parameter, "-l"))) {
        return LoadCommand;
    }

    if ((AreEqual(parameter, "-unload")) 
        || (AreEqual(parameter, "-u"))) {
        return UnloadCommand;
    }

    if ((AreEqual(parameter, "-reload")) 
        || (AreEqual(parameter, "-r"))) {
        return ReloadCommand;
    }

    if (AreEqual(parameter, "-list")) {
        return ListDriversCommand;
    }

    if (AreEqual(parameter, "-reg")) {
        return ListRegistryValuesCommand;
    }

    if (StartsWith(parameter, "-reg:")) {
        return SetRegistryValueCommand;
    }

    return InvalidCommand;
}

void LoadDriver(char* deviceKey)
{
    HANDLE hDriver;

    char regKey[80];
    strcpy(regKey, REG_DRIVERS_BUILTIN);
    strcat(regKey, deviceKey);
    
    // convert char to wchar_t for ActivateDeviceEx parameter
    wchar_t wRegKey[80];
    mbstowcs(wRegKey, regKey, strlen(regKey) + 1);
    hDriver = ActivateDeviceEx(wRegKey, NULL, 0, NULL);

    if (hDriver != INVALID_HANDLE_VALUE && hDriver != 0)
    {
        printf("\n Driver for '%s' loaded successfully.\n\n", deviceKey);
    }
    else
    {
        printf("\n Error loading driver for '%s'.\n\n", deviceKey);
    }
}

BOOL FindLoadedDriver(char* driverName, PDEVMGR_DEVICE_INFORMATION deviceInfo)
{    
    DeviceSearchType deviceSearch = DeviceSearchByLegacyName;
    deviceInfo->dwSize = sizeof(DEVMGR_DEVICE_INFORMATION);
    HANDLE hDriver = FindFirstDevice(deviceSearch, L"*", deviceInfo);

    if (hDriver == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    // convert char array to wchar_t array for wcsncmp function
    WCHAR* wDriverName = CharToWChar(driverName);

    BOOL isFound = FALSE;
    while (FindNextDevice(hDriver, deviceInfo))
    {
        if (wcsncmp(deviceInfo->szLegacyName, wDriverName, 4) == 0)
        {
            isFound = TRUE;
            break;
        }
    }

    FindClose(hDriver);
    return isFound;
}

void UnloadDriver(char* driverName)
{
    DEVMGR_DEVICE_INFORMATION deviceInfo;
    BOOL isFound = FindLoadedDriver(driverName, &deviceInfo);

    if (isFound == FALSE) {   
        printf("\n Driver '%s' not loaded.\n\n", driverName);
        printf(" Note driver name is case sensitive. Use -help switch for sample usage.\n\n");
        return;
    }
 
    printf("\n Found driver '%ls'. Attempting to unload.\n", deviceInfo.szLegacyName);

    BOOL result = DeactivateDevice(deviceInfo.hDevice);
    if (result == FALSE) {
        printf("\n ERROR unloading driver.\n\n");
        return;
    }

    printf(" Driver unloaded successfully.\n\n");
}

void ReloadDriver(char* driverName)
{
    printf("\n Reloading driver '%s'.\n", driverName);
    printf("\n Feature not implemented.\n\n");
}

void ListDrivers()
{
    HANDLE hDriver;
    DeviceSearchType deviceSearch = DeviceSearchByLegacyName;
    DEVMGR_DEVICE_INFORMATION deviceInfo;
    deviceInfo.dwSize = sizeof(DEVMGR_DEVICE_INFORMATION);
    hDriver = FindFirstDevice(deviceSearch, L"*", &deviceInfo);

    if (hDriver != INVALID_HANDLE_VALUE) {
        printf(" Legacy Name / Device Key / Device Name / Bus Name\n");
        
        do {
            printf(" %ls %ls %ls %ls\n",
                deviceInfo.szLegacyName,
                deviceInfo.szDeviceKey,
                deviceInfo.szDeviceName,
                deviceInfo.szBusName);
        } 
        while (FindNextDevice(hDriver, &deviceInfo));
        
        FindClose(hDriver);
    }
}

// http://www.cplusplus.com/forum/windows/45454/
void ListRegistryValues(HKEY hKey) 
{ 
    TCHAR    achKey[MAX_KEY_LENGTH];  // buffer for subkey name
    DWORD    cbName;				  // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT(""); // buffer for class name 
    DWORD    cchClassName = MAX_PATH; // size of class string 
    DWORD    cSubKeys=0;              // number of subkeys 
    DWORD    cbMaxSubKey;             // longest subkey size 
    DWORD    cchMaxClass;             // longest class string 
    DWORD    cValues;                 // number of values for key 
    DWORD    cchMaxValue;             // longest value name 
    DWORD    cbMaxValueData;          // longest value data 
    DWORD    cbSecurityDescriptor;    // size of security descriptor 
    FILETIME ftLastWriteTime;         // last write time 
    DWORD i, retCode; 

    // get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                   // key handle 
        achClass,               // buffer for class name 
        &cchClassName,          // size of class string 
        NULL,                   // reserved 
        &cSubKeys,              // number of subkeys 
        &cbMaxSubKey,           // longest subkey size 
        &cchMaxClass,           // longest class string 
        &cValues,               // number of values for this key 
        &cchMaxValue,           // longest value name 
        &cbMaxValueData,        // longest value data 
        &cbSecurityDescriptor,  // security descriptor 
        &ftLastWriteTime);      // last write time 

    // enumerate the subkeys (if any are present)
    if (cSubKeys) {
        printf("\n");
        printf(" Subkeys:");

        for (i=0; i < cSubKeys; i++) { 
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(
                hKey, 
                i,
                achKey, 
                &cbName, 
                NULL, 
                NULL, 
                NULL, 
                &ftLastWriteTime);
            if (retCode == ERROR_SUCCESS) {
                _tprintf(TEXT(" %s\n"), achKey);
            }
        }
    } 

    // enumerate the key values
    BYTE* buffer = (BYTE*)malloc(cbMaxValueData);
    ZeroMemory(buffer, cbMaxValueData);
    TCHAR valueName[MAX_VALUE_NAME]; 
    DWORD valueNameSize = MAX_VALUE_NAME; 

    if (cValues) {
        printf("\n");
        printf(" %-20s %-15s %s\n", "Name:", "Type:", "Value:");

        for (i=0, retCode = ERROR_SUCCESS; i < cValues; i++) {
            valueNameSize = MAX_VALUE_NAME; 
            valueName[0] = '\0';
            retCode = RegEnumValue(
                hKey, 
                i, 
                valueName, 
                &valueNameSize, 
                NULL, 
                NULL,
                NULL,
                NULL);

            if (retCode == ERROR_SUCCESS ) { 
                DWORD valueType; 
                buffer[0] = '\0';
                DWORD lpData = cbMaxValueData;
                LONG dwRes = RegQueryValueEx(hKey, valueName, 0, &valueType, buffer, &lpData);

                switch(valueType)
                {
                    case REG_BINARY:
                        _tprintf(TEXT(" %-20s %-15s %d\n"), valueName, TEXT("REG_BINARY"), *buffer);
                        break;
                    case REG_SZ:
                        _tprintf(TEXT(" %-20s %-15s %s\n"), valueName, TEXT("REG_SZ"), buffer);
                        break;
                    case REG_DWORD:
                        _tprintf(TEXT(" %-20s %-15s %d\n"), valueName, TEXT("REG_DWORD"), *buffer);
                        break;
                    case REG_MULTI_SZ:
                        _tprintf(TEXT(" %-20s %-15s %s\n"), valueName, TEXT("REG_MULTI_SZ"), buffer);
                        break;
                    case REG_EXPAND_SZ:
                        _tprintf(TEXT(" %-20s %-15s %s\n"), valueName, TEXT("REG_EXPAND_SZ"), buffer);
                        break;
                    default:
                        _tprintf(TEXT(" %-20s %-15s %s\n"), valueName, TEXT("(UNKNOWN)"), buffer); 
                }
            } 
        }
    }

    printf("\n");
    free(buffer);
}

void ListRegistryValues(char* deviceKey)
{
    // assemble registry key string
    char regKey[80];
    strcpy(regKey, REG_DRIVERS_BUILTIN);
    strcat(regKey, deviceKey);

    printf("\n");
    printf(" Registry values for driver key: \n");
    printf("   HKEY_LOCAL_MACHINE\\%s\n", regKey);

    // convert char array to wchar_t array for RegOpenKeyEx function
    WCHAR* wRegKey = CharToWChar(regKey);
    HKEY hKey;

    #if PC_DEBUG_MODE
    LONG dwRegOPenKey = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ATestKey\\"), 0, KEY_READ, &hKey);
    #else
    LONG dwRegOPenKey = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wRegKey, 0, KEY_READ, &hKey);
    #endif

    if (dwRegOPenKey == ERROR_SUCCESS) {
        ListRegistryValues(hKey);
    } 
    else {
        printf("\n ERROR: RegOpenKeyEx failed, error code %d\n\n", dwRegOPenKey);
    }

    RegCloseKey(hKey);
}

void ParseRegistryValue(char* argument, char* pValueName, char* pValue)
{
    char nameValue[50];
    memcpy(nameValue, &argument[5], 50);

    strcpy(pValueName, strtok(nameValue, "="));
    strcpy(pValue, strtok(NULL, "="));
}

BOOL QueryKeyValue(HKEY hKey, char* valueName, DWORD* valueType) 
{ 
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;	// size of class string 
    DWORD    cSubKeys=0;				// number of subkeys 
    DWORD    cbMaxSubKey;				// longest subkey size 
    DWORD    cchMaxClass;				// longest class string 
    DWORD    cValues;					// number of values for key 
    DWORD    cchMaxValue;				// longest value name 
    DWORD    cbMaxValueData;			// longest value data 
    DWORD    cbSecurityDescriptor;		// size of security descriptor 
    FILETIME ftLastWriteTime;			// last write time 
    BOOL isValidValueName = FALSE;

    // get the class name and the value count. 
    DWORD retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 

    if (!cValues) {
        return FALSE;
    }

    BYTE* buffer = new BYTE[cbMaxValueData];
    ZeroMemory(buffer, cbMaxValueData);
    TCHAR lpValueName[MAX_VALUE_NAME]; 
    DWORD valueNameSize = MAX_VALUE_NAME;

    // enumerate the key values
    for (int i=0; i<(int)cValues; i++) {
        valueNameSize = MAX_VALUE_NAME; 
        lpValueName[0] = '\0';
        int retCode = RegEnumValue(
            hKey, 
            i, 
            lpValueName, 
            &valueNameSize, 
            NULL, 
            NULL,
            NULL,
            NULL);

        if (retCode == ERROR_SUCCESS ) { 
            DWORD lpType; 
            buffer[0] = '\0';
            DWORD lpData = cbMaxValueData;
            LONG dwRes = RegQueryValueEx(hKey, lpValueName, 0, &lpType, buffer, &lpData);
            WCHAR* wValueName = CharToWChar(valueName);

            if (lstrcmp(lpValueName, wValueName) == 0) {
                *valueType = lpType;
                isValidValueName = TRUE;
                break;
            }
        } 
    }

    delete []buffer;
    return isValidValueName;
}

void SetRegistryValue(char* deviceKey, char* valueName, char* value)
{
    char regKey[80];
    strcpy(regKey, REG_DRIVERS_BUILTIN);
    strcat(regKey, deviceKey);

    WCHAR* wRegKey = CharToWChar(regKey);
    HKEY hRegKey;

    #if PC_DEBUG_MODE
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ATestKey\\"), 0, KEY_ALL_ACCESS, &hRegKey);
    #else
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wRegKey, 0, KEY_ALL_ACCESS, &hRegKey);
    #endif

    if (result != ERROR_SUCCESS) {
        printf("\n ERROR: Registry entry not found for key %s.\n\n", regKey);
        return;
    }

    // verify that the registy key has an existing value of the name specified
    DWORD valueType;
    result = QueryKeyValue(hRegKey, valueName, &valueType);
    if (result != TRUE) {
        printf("\n ERROR finding registry value with name '%s' for key %s.\n\n", valueName, regKey);
        RegCloseKey(hRegKey);
        return;
    }

    // set the value
    WCHAR* wValueName = CharToWChar(valueName);
    wchar_t* wValue;
    DWORD dwValue;
    DWORD dwSize;

    switch(valueType)
    {
        case REG_BINARY:
            printf("\n Unable to set 'REG_BINARY' value type. Feature not implemented.\n\n");
            break;
        case REG_SZ:
            wValue = CharToWChar(value);
            dwSize = (wcslen(wValue) + 1) * sizeof(wchar_t);
            result = RegSetValueEx(hRegKey, wValueName, 0, valueType, (CONST BYTE *)wValue, dwSize);
            if (result == ERROR_SUCCESS) {
                printf("\n Successfully set value for %s (REG_SZ) for registry key:\n   HKEY_LOCAL_MACHINE\\%s\n\n", valueName, regKey);
            }
            else {
                printf("\n ERROR setting value for %s (REG_SZ) for registry key:\n   HKEY_LOCAL_MACHINE\\%s\n\n", valueName, regKey);
            }
            break;
        case REG_DWORD:
            dwValue = atoi(value);
            dwSize = sizeof(DWORD);
            result = RegSetValueEx(hRegKey, wValueName, 0, valueType, (CONST BYTE *)&dwValue, dwSize);
            if (result == ERROR_SUCCESS) {
                printf("\n Successfully set value for %s (REG_DWORD) for registry key:\n   HKEY_LOCAL_MACHINE\\%s\n\n", valueName, regKey);
            }
            else {
                printf("\n ERROR setting value for %s (REG_DWORD) for registry key:\n   HKEY_LOCAL_MACHINE\\%s\n\n", valueName, regKey);
            }
            break;
        case REG_MULTI_SZ:
            printf("\n Unable to set 'REG_MULTI_SZ' value type. Feature not implemented.\n\n");
            break;
        case REG_EXPAND_SZ:
            printf("\n Unable to set 'REG_EXPAND_SZ' value type. Feature not implemented.\n\n");
            break;
        default:
            printf("\n Unsupported data type.\n\n");
            break;
    }

    RegCloseKey(hRegKey);
}


void SetRegistryValue(char* deviceKey, char* nameValuePair)
{
    if (Contains(nameValuePair, '=') == FALSE) {
        printf("\n ERROR: Incorrect syntax. Use -help to view correct syntax.\n\n");
        return;
    }

    char valueName[50];
    char value[50];
    ParseRegistryValue(nameValuePair, valueName, value);

    SetRegistryValue(deviceKey, valueName, value);
}

void DisplayHelp()
{
    printf("\n");
    printf(" Options\n");
    printf("  -load <key>      Loads a driver with the specified registry key\n");
    printf("  -unload <name>   Unloads the specified driver\n");
    printf("  -reload <name>   Reloads the specified driver\n");
    printf("  -list            Lists all loaded drivers\n");
    printf("  -reg <key>       Displays values for the specified registy key\n");
    printf("  -reg:<name>=<value> <key>  Sets the value for the specified registy key\n");
    printf("\n");
    printf(" Sample usage:\n");
    printf("  driver -list\n");
    printf("  driver -load DriverShell\n");
    printf("  driver -unload DRV1\n");
    printf("  driver -reload DRV1\n");
    printf("  driver -reg DriverShell\n");
    printf("  driver -reg:Flags=2 DriverShell\n");
    printf("  driver -reg:Prefix=\"DRV\" DriverShell\n");
}

int main(int argc, char* argv[])
{
    if (argc == 1) {
        DisplayHelp();
        return 0;
    }

    CommandType command = GetCommandType(argv[1]);

    switch (command)
    {
        case LoadCommand:
            LoadDriver(argv[2]);
            break;
        case UnloadCommand:
            UnloadDriver(argv[2]);
            break;
        case ReloadCommand:
            ReloadDriver(argv[2]);
            break;
        case ListDriversCommand:
            ListDrivers();
            break;
        case ListRegistryValuesCommand:
            ListRegistryValues(argv[2]);
            break;
        case SetRegistryValueCommand:
            if (argc < 3) {
                printf("Invalid number of arguments.");
                return 0;
            }

            SetRegistryValue(argv[2], argv[1]);
            break;
        default:
            DisplayHelp();
    }

    return 0;
}


