#include <windows.h>
#include "resource.h"

#include "net/CurlHttpClient.hpp"
#include "net/RetryingHttpClient.hpp"

#include "services/AuthService.hpp"
#include "services/UserService.hpp"
#include "services/UploadService.hpp"

#include "models/Token.hpp"
#include "models/User.hpp"
#include "models/UploadResult.hpp"

#include "common/HttpCommon.hpp"

#include <string>

using namespace app;

HINSTANCE g_hInst;

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_BUTTON_BROWSE: {
            OPENFILENAME ofn;
            char szFile[MAX_PATH] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwndDlg;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE) {
                SetDlgItemTextA(hwndDlg, IDC_EDIT_FILE, szFile);
            }
            break;
        }

        case IDC_BUTTON_UPLOAD: {
            char _host[256], _user[128], _pass[128], _file[260];
            GetDlgItemTextA(hwndDlg, IDC_EDIT_HOST, _host, sizeof(_host));
            GetDlgItemTextA(hwndDlg, IDC_EDIT_USER, _user, sizeof(_user));
            GetDlgItemTextA(hwndDlg, IDC_EDIT_PASS, _pass, sizeof(_pass));
            GetDlgItemTextA(hwndDlg, IDC_EDIT_FILE, _file, sizeof(_file));

            try {


                std::string host(_host);
                std::string user(_user);
                std::string pass(_pass);
                std::string file(_file);
                // 1. Base client
                net::CurlHttpClient curl;

                // 2. Auth
                services::AuthService auth(curl, user, pass);
                models::Token token = auth.authenticate(host);

                // 3. Wrap with retry client
                net::RetryingHttpClient http(curl, auth, host, token);

                // 4. Services
                services::UserService users(http);
                services::UploadService uploader(http);

                // 5. Get self
                models::User me = users.getSelf(host, token);

                // 6. Upload
                models::UploadResult result =
                    uploader.uploadFile(host, token, me.homeFolderId, file, false);

               // if (!result.success && result.appError == http::ERR_FILE_EXISTS) {
               //     int resp = MessageBoxW(hwndDlg,
               //         L"File exists. Overwrite?",
               //         L"Conflict",
               //         MB_YESNO | MB_ICONQUESTION);

               //     if (resp == IDYES) {
               //         result = uploader.uploadFile(host, token, me.homeFolderId, file, true);
               //     }
               // }

               //if (result.success) {
               //     std::wstring msg = L"Upload OK: " +
               //         std::wstring(result.fileName.begin(), result.fileName.end());
               //     MessageBoxW(hwndDlg, msg.data(), L"Success", MB_OK | MB_ICONINFORMATION);
               // }
               // else {
               //     std::wstring msg(result.message.begin(), result.message.end());
               //     MessageBoxW(hwndDlg, msg.c_str(), L"Upload failed", MB_OK | MB_ICONERROR);
               // }
            }
            catch (const std::exception& ex) {
                std::wstring msg(ex.what(), ex.what() + strlen(ex.what()));
                MessageBoxW(hwndDlg, msg.c_str(), L"Fatal error", MB_OK | MB_ICONERROR);
            }

            break;
        }

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;
    }

    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    g_hInst = hInstance;
    //DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, DialogProc);
    return 0;
}

