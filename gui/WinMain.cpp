#include <windows.h>
#include <commdlg.h> // for GetOpenFileName
#include "resource.h"

#include "net/CurlHttpClient.hpp"
#include "json.hpp"

using json = nlohmann::json;

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_BROWSE: {
            char file[MAX_PATH] = {};
            OPENFILENAME ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFilter = "All Files\0*.*\0";
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                SetDlgItemText(hDlg, IDC_EDIT_FILE, file);
            }
            return TRUE;
        }

        case IDC_BUTTON_UPLOAD: {
            char host[256], user[256], pass[256], file[512];
            GetDlgItemText(hDlg, IDC_EDIT_HOST, host, sizeof(host));
            GetDlgItemText(hDlg, IDC_EDIT_USER, user, sizeof(user));
            GetDlgItemText(hDlg, IDC_EDIT_PASS, pass, sizeof(pass));
            GetDlgItemText(hDlg, IDC_EDIT_FILE, file, sizeof(file));

            if (strlen(host) == 0 || strlen(user) == 0 || strlen(pass) == 0 || strlen(file) == 0) {
                SetDlgItemText(hDlg, IDC_EDIT_LOG, "Please fill all fields.");
                return TRUE;
            }

            app::net::CurlHttpClient http;

            // 1. Auth
            auto tokenResp = http.postForm(std::string(host) + "/api/v1/token",
                                           {{"grant_type","password"},
                                            {"username",user},
                                            {"password",pass}});
            if (tokenResp.status != 200) {
                SetDlgItemText(hDlg, IDC_EDIT_LOG, tokenResp.body.c_str());
                return TRUE;
            }
            std::string access = json::parse(tokenResp.body).at("access_token");

            // 2. Self
            auto selfResp = http.get(std::string(host) + "/api/v1/users/self",
                                     {{"Authorization","Bearer " + access},
                                      {"Accept","application/json"}});
            if (selfResp.status != 200) {
                SetDlgItemText(hDlg, IDC_EDIT_LOG, selfResp.body.c_str());
                return TRUE;
            }
            long long homeId = json::parse(selfResp.body).at("homeFolderID");

            // 3. Upload
            auto upResp = http.postMultipart(
                std::string(host) + "/api/v1/folders/" + std::to_string(homeId) + "/files",
                file,
                {},
                {{"Authorization","Bearer " + access}});
            if (upResp.status != 201) {
                SetDlgItemText(hDlg, IDC_EDIT_LOG, upResp.body.c_str());
                return TRUE;
            }

            SetDlgItemText(hDlg, IDC_EDIT_LOG, "Upload OK!\r\n");
            return TRUE;
        }
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
    }
    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, DlgProc);
}
