#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <windows.h>
#include <cstdio>

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
    MessageBox(hwnd, ("There was a problem starting your game.\nError message: "+text).c_str(), "EengineLauncher - ERROR", MB_ICONERROR | MB_OK);
}

void ReportError(std::string text, int code)
{
    cout << "ERROR: " << text << ", code: " << code << endl;
    char* codeStr = new char[4];
    sprintf(codeStr, "%d", code);
    MessageBox(hwnd, ("There was a problem starting your game.\nError message: "+text+"\nError code: "+codeStr).c_str(), "EengineLauncher - ERROR", MB_ICONERROR | MB_OK);
    delete codeStr;
}

// --------

std::string colobotDir;

enum ColobotVersion {
	VERSION_UNKNOWN,
	VERSION_1_9_PL,
	VERSION_1_3_EN,
	VERSION_1_8_EN,
	VERSION_1_9_PL_KS,
	VERSION_CEEBOT4_PL,
	VERSION_C4S_EN,
	VERSION_CEEBOT_A_EN,
	VERSION_BC_1_0_EN
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
	version_unknown.fullVersionStr = "Eengine game";
	versions[VERSION_UNKNOWN] = version_unknown;

	VersionData version_1_9_pl;
	version_1_9_pl.md5 = "991ff670d594e064333bf8b3ef14a395";
	version_1_9_pl.websiteStringPtr = (void*)0x0053CEE0;
	version_1_9_pl.versionStringPtr = (void*)0x005416A8;
	version_1_9_pl.versionStr = "1.9 PL";
	version_1_9_pl.fullVersionStr = "Colobot";
	versions[VERSION_1_9_PL] = version_1_9_pl;

	VersionData version_1_3_en;
	version_1_3_en.md5 = "19b10cab2e24170e75791e0d8ef6adb2";
	version_1_3_en.websiteStringPtr = (void*)0x00537790;
	version_1_3_en.versionStringPtr = (void*)0x0053BE28;
	version_1_3_en.versionStr = "1.3 EN";
	version_1_3_en.fullVersionStr = "Colobot";
	versions[VERSION_1_3_EN] = version_1_3_en;

	VersionData version_1_8_en;
	version_1_8_en.md5 = "60b187eb8caa297af90fb9f1c618f9b0";
	version_1_8_en.websiteStringPtr = (void*)0x0053CEE8;
	version_1_8_en.versionStringPtr = (void*)0x005416B8;
	version_1_8_en.versionStr = "1.8 EN";
	version_1_8_en.fullVersionStr = "Colobot";
	versions[VERSION_1_8_EN] = version_1_8_en;
	
	VersionData version_1_9_pl_ks;
	version_1_9_pl_ks.md5 = "83e54ee377bcdf9f4930e0892952c4ab";
	version_1_9_pl_ks.websiteStringPtr = (void*)0x0053CEE8;
	version_1_9_pl_ks.versionStringPtr = (void*)0x005416B0;
	version_1_9_pl_ks.versionStr = "1.9 PL";
	version_1_9_pl_ks.fullVersionStr = "Colobot";
	versions[VERSION_1_9_PL_KS] = version_1_9_pl_ks;
	
	VersionData version_ceebot4_pl;
	version_ceebot4_pl.md5 = "c5e4674367ff2f842cc7b5761d6a744a";
	version_ceebot4_pl.fullVersionStr = "CeeBot4";
	versions[VERSION_CEEBOT4_PL] = version_ceebot4_pl;
	
	VersionData version_c4s_en;
	version_c4s_en.md5 = "12b77fc84e07e69fed370e8e3e4c8d8e";
	version_c4s_en.fullVersionStr = "CeeBot4 SCHOOL";
	versions[VERSION_C4S_EN] = version_c4s_en;
	
	VersionData version_ceebot_a_en;
	version_ceebot_a_en.md5 = "d719d5546777ba4d320adc51e8c99c63";
	version_ceebot_a_en.fullVersionStr = "CeeBot-A";
	versions[VERSION_CEEBOT_A_EN] = version_ceebot_a_en;
	
	VersionData version_bc_1_0_en;
	version_bc_1_0_en.md5 = "b85b3d09a12601fee4b8a3c234922882";
	version_bc_1_0_en.fullVersionStr = "BuzzingCars";
	versions[VERSION_BC_1_0_EN] = version_bc_1_0_en;
	
}

string checkGame()
{
	CHAR* workingDir = new CHAR[513];
	GetCurrentDirectory(512, workingDir);
	std::string found;
	if(fexists(std::string(workingDir)+"\\colobot.exe")) {
		found = "colobot.exe";
	} 
	else if(fexists(std::string(workingDir)+"\\ceebot.exe")) {
		found = "ceebot.exe";
	}
	else if(fexists(std::string(workingDir)+"\\ceebot-a.exe")) {
		found = "ceebot-a.exe";
	}
	else if(fexists(std::string(workingDir)+"\\ceebot4.exe")) {
		found = "ceebot4.exe";
	}
	else if(fexists(std::string(workingDir)+"\\buzzingcars.exe")) {
		found = "buzzingcars.exe";
	}

	return found;
}

ColobotVersion GetColobotVersion()
{
	std::string game = checkGame();
	std::string current = GetFileMD5(colobotDir+game);
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
	std::string game = checkGame();
    colobotDir = std::string(workingDir)+"\\";

	if(game == "")
	{
		ReportError("exe not found!");
		return EXIT_FAILURE;
	}
	
    if(argc >= 2) {
    	if(!strcmp(argv[1], "getmd5")) {
	    	cout << "MD5: " << GetFileMD5(colobotDir+game) << endl;
						MessageBox(hwnd, ("MD5: "+GetFileMD5(colobotDir+game)).c_str(), "EengineLauncher", MB_ICONINFORMATION | MB_OK);
			return EXIT_SUCCESS;
		}
		
		if(!strcmp(argv[1], "getclassname")) {
			hwnd = FindWindow(NULL, "COLOBOT");
			char* classname = new char[64];
			GetClassName(hwnd, classname, 64);
			
			cout << "Class name: " << classname << endl;
			MessageBox(hwnd, (std::string("Class name: ")+classname).c_str(), "EengineLauncher", MB_ICONINFORMATION | MB_OK);

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
    res = RegSetValueEx(hkey, (LPCTSTR)((colobotDir+game).c_str()), 0, REG_SZ, (LPBYTE)data, strlen(data)+1);
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
    if(!StartProcess(colobotDir.c_str(), (colobotDir+game+" -nocd").c_str(), &pi)) {
        ReportError("creating process");
        return EXIT_FAILURE;
    }
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pi.dwProcessId);
    cout << "OK" << endl;
    
    cout << "Waiting for game window... ";
    while(hwnd == NULL) {
    	hwnd = FindWindow("D3D Window", "COLOBOT");
		hwnd = FindWindow("D3D Window", "BuzzingCars");
		hwnd = FindWindow("D3D Window", "CeeBot");
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
    
    CloseHandle(process);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return EXIT_SUCCESS;
}
