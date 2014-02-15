#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <windows.h>

#include "utils.h"

using namespace std;

HWND hwnd = NULL;

bool StartProcess(const char* workdir, const char* cmd, PROCESS_INFORMATION* pi)
{
    STARTUPINFO si;
    
    ZeroMemory( pi, sizeof( PROCESS_INFORMATION ) );
    ZeroMemory( & si, sizeof( si ) );
    si.cb = sizeof( si );
    
    return CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, FALSE, 0, NULL, workdir, &si, pi );
}

void ReportError(std::string text)
{
    cout << "ERROR: " << text << endl;
    MessageBox(hwnd, ("There was a problem starting your game.\nError message: "+text).c_str(), "ColobotLauncher - ERROR", MB_ICONERROR | MB_OK);
}

void ReportError(std::string text, int code)
{
    cout << "ERROR: " << text << ", code: " << code << endl;
    char* codeStr = new char[4];
    sprintf(codeStr, "%d", code);
    MessageBox(hwnd, ("There was a problem starting your game.\nError message: "+text+"\nError code: "+codeStr).c_str(), "ColobotLauncher - ERROR", MB_ICONERROR | MB_OK);
    delete codeStr;
}

// --------

std::string colobotDir;

enum ColobotVersion {
	VERSION_UNKNOWN,
	VERSION_1_9_PL,
	VERSION_1_3_EN,
	VERSION_1_8_EN
};

struct VersionData {
	std::string md5;
	void* websiteStringPtr;
	void* versionStringPtr;
	std::string versionStr; // <= 7
	std::string fullVersionStr;
};

std::map<ColobotVersion, VersionData> versions;

void LoadVersions()
{
	//TODO: Auto-update? Reading from file?
	
	VersionData version_unknown;
	version_unknown.fullVersionStr = "Colobot Orginal (unknown version)";
	versions[VERSION_UNKNOWN] = version_unknown;
	
	VersionData version_1_9_pl;
	version_1_9_pl.md5 = "991ff670d594e064333bf8b3ef14a395";
	version_1_9_pl.websiteStringPtr = (void*)0x0053CEE0;
	version_1_9_pl.versionStringPtr = (void*)0x005416A8;
	version_1_9_pl.versionStr = "1.9 PL";
	version_1_9_pl.fullVersionStr = "Colobot Orginal 1.9 PL";
	versions[VERSION_1_9_PL] = version_1_9_pl;
	
	VersionData version_1_3_en;
	version_1_3_en.md5 = "19b10cab2e24170e75791e0d8ef6adb2";
	version_1_3_en.websiteStringPtr = (void*)0x00537790;
	version_1_3_en.versionStringPtr = (void*)0x0053BE28;
	version_1_3_en.versionStr = "1.3 EN";
	version_1_3_en.fullVersionStr = "Colobot Orginal 1.3 EN";
	versions[VERSION_1_3_EN] = version_1_3_en;
	
	VersionData version_1_8_en;
	version_1_8_en.md5 = "60b187eb8caa297af90fb9f1c618f9b0";
	version_1_8_en.websiteStringPtr = (void*)0x0053CEE8;
	version_1_8_en.versionStringPtr = (void*)0x005416B8;
	version_1_8_en.versionStr = "1.8 EN";
	version_1_8_en.fullVersionStr = "Colobot Orginal 1.8 EN";
	versions[VERSION_1_8_EN] = version_1_8_en;
}

ColobotVersion GetColobotVersion()
{
	std::string current = GetFileMD5(colobotDir+"colobot.exe");
	for(std::map<ColobotVersion, VersionData>::iterator ver = versions.begin(); ver != versions.end(); ver++) {
		if(ver->first == VERSION_UNKNOWN) continue;
		if(ver->second.md5 == current) {
			return ver->first;
		}
	}
	return VERSION_UNKNOWN;
}

int main(int argc, char *argv[])
{
	LoadVersions();
	
	CHAR* workingDir = new CHAR[513];
	GetCurrentDirectory(512, workingDir);
	
	if(!fexists(std::string(workingDir)+"\\colobot.exe")) {
		//TODO: Search in Program Files
		ReportError("colobot.exe not found!");
		return EXIT_FAILURE;
	}
	
    colobotDir = std::string(workingDir)+"\\";
    
    if(argc >= 2) {
    	if(!strcmp(argv[1], "getmd5")) {
	    	cout << "MD5: " << GetFileMD5(colobotDir+"colobot.exe") << endl;
	    	return EXIT_SUCCESS;
		}
    }
    
    cout << "Setting combatibility mode... ";
    HKEY hkey;
    LONG res = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 0, KEY_ALL_ACCESS, &hkey);
    if(res != ERROR_SUCCESS) {
           ReportError("opening key", res);
           return EXIT_FAILURE;
    }
    LPCTSTR data = "WINXPSP3 RUNASADMIN\0";
    res = RegSetValueEx(hkey, (LPCTSTR)((colobotDir+"colobot.exe").c_str()), 0, REG_SZ, (LPBYTE)data, strlen(data)+1);
    if(res != ERROR_SUCCESS) {
           ReportError("writing value", res);
           return EXIT_FAILURE;
    }
    cout << "OK" << endl;
    
    cout << "Checking version... ";
	ColobotVersion version = GetColobotVersion();
	VersionData versionData = versions[version];
	cout << versionData.fullVersionStr << endl;
    
    cout << "Creating process... ";
    PROCESS_INFORMATION pi;
    if(!StartProcess(colobotDir.c_str(), (colobotDir+"colobot.exe -nocd").c_str(), &pi)) {
        ReportError("creating process");
        return EXIT_FAILURE;
    }
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pi.dwProcessId);
    cout << "OK" << endl;
    
    cout << "Waiting for game window... ";
    while(hwnd == NULL) {
    	hwnd = FindWindow(NULL, "COLOBOT");
    }
    cout << "OK" << endl;
    
    cout << "Changing window title... ";
    if(!SetWindowText(hwnd, versionData.fullVersionStr.c_str())) {
    	cout << "ERROR (" << GetLastError() << "), SKIP" << endl;
    } else {
        cout << "OK" << endl;
    }
    
    cout << "Changing website text... ";
    if(version == VERSION_UNKNOWN || versionData.websiteStringPtr == NULL) {
    	cout << "SKIP" << endl;
    } else {
    	const char* newWebsite = "colobot.info"; // must be shorter than 15
    	SIZE_T written;
    	if(!WriteProcessMemory(process, versionData.websiteStringPtr, newWebsite, strlen(newWebsite)+1, &written)) {
   			cout << "ERROR (" << GetLastError() << "), SKIP" << endl;
    	} else {
	    	cout << "OK" << endl;
    	}
    }
    
    cout << "Changing version text... ";
    if(version == VERSION_UNKNOWN || versionData.versionStringPtr == NULL) {
    	cout << "SKIP" << endl;
    } else {
    	std::string data = versionData.versionStr + " by PPC"; // version + by PPC <= 14
    	SIZE_T written;
    	if(!WriteProcessMemory(process, versionData.versionStringPtr, data.c_str(), data.length()+1, &written)) {
   			cout << "ERROR (" << GetLastError() << "), SKIP" << endl;
    	} else {
	    	cout << "OK" << endl;
    	}
    }
    
    CloseHandle(process);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return EXIT_SUCCESS;
}
