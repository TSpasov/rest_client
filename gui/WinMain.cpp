#include <windows.h>
#include <string>
#include "common/Logger.hpp"

#include "net/CurlHttpClient.hpp"
#include "net/RetryingHttpClient.hpp"
#include "services/AuthService.hpp"
#include "services/UserService.hpp"
#include "services/UploadService.hpp"

#include "models/Token.hpp"
#include "models/User.hpp"
#include "models/UploadResult.hpp"

#include "common/HttpCommon.hpp"
#include "resource.h"

using app::common::Logger;
static Logger logger;   // global logger shared across this translation unit

using namespace app;
using namespace app::common;

INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static std::string host, user, pass, file;

    switch (msg) {
    case WM_INITDIALOG: {
        HWND hwndEdit = GetDlgItem(hwndDlg, IDC_LOGVIEW);
        logger.setGuiTarget(GetDlgItem(hwndDlg, IDC_LOGVIEW));

        logger.info("Logger initialized (GUI target).");
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CHOOSEFILE: {
            OPENFILENAME ofn{};
            char szFile[MAX_PATH] = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwndDlg;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "All Files\0*.*\0";
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                file = szFile;
                SetDlgItemTextA(hwndDlg, IDC_FILEPATH, file.c_str());
            }
            break;
        }
        case IDC_UPLOAD: {


            wchar_t bufHost[256], bufUser[256], bufPass[256];
            GetDlgItemTextW(hwndDlg, IDC_HOST, bufHost, 256);
            GetDlgItemTextW(hwndDlg, IDC_USERNAME, bufUser, 256);
            GetDlgItemTextW(hwndDlg, IDC_PASSWORD, bufPass, 256);

            auto to_utf8 = [](const std::wstring& ws) {
                if (ws.empty()) return std::string{};
                int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string s(len - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, s.data(), len, nullptr, nullptr);
                return s;
                };

            host = to_utf8(bufHost);
            user = to_utf8(bufUser);
            pass = to_utf8(bufPass);

            logger.info("Host: " + host);
            logger.info("User: " + user);

            try {
                net::CurlHttpClient curl;
                services::AuthService auth(curl, user, pass);
                models::Token token = auth.authenticate(host);
                logger.info("Token acquired (expires in " + std::to_string(token.expires_in) + "s)");

                net::RetryingHttpClient http(curl, auth, host, token);
                services::UserService users(http);
                services::UploadService uploader(http);

                models::User me = users.getSelf(host, token);
                logger.info("homeFolderID = " + me.homeFolderId);

                auto existing = uploader.findFileId(host, token, me.homeFolderId, file);
                if (existing.has_value()) {
                    logger.warn("File already exists (id=" + std::to_string(existing.value()) + ")");
                    int resp = MessageBoxA(hwndDlg, "File exists. Overwrite?", "Conflict", MB_YESNO | MB_ICONQUESTION);
                    if (resp == IDYES) {
                        uploader.deleteFile(host, token, existing.value());
                        logger.info("Deleted existing file id=" + std::to_string(existing.value()));
                    }
                    else {
                       logger.info("Upload canceled by user.");
                        return TRUE;
                    }
                }

                models::UploadResult result = uploader.uploadFile(host, token, me.homeFolderId, file, false);

                if (result.success) {
                    MessageBoxA(hwndDlg, ("Upload OK: " + result.fileName).c_str(), "Success", MB_OK | MB_ICONINFORMATION);
                    logger.info("Upload OK: " + result.fileName + " (id=" + std::to_string(result.fileId) + ")");
                }
                else {
                    MessageBoxA(hwndDlg, ("Upload failed: " + result.message).c_str(), "Error", MB_OK | MB_ICONERROR);
                    logger.error("Upload failed: " + result.message);
                }

            }
            catch (const std::exception& ex) {
                MessageBoxA(hwndDlg, ex.what(), "Fatal Error", MB_OK | MB_ICONERROR);
                logger.error(std::string("Fatal error: ") + ex.what());
            }
            break;
        }
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        break;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {

    DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, DlgProc);
    return 0;
}
