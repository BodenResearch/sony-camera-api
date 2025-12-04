#include <cstdlib>
#if defined(USE_EXPERIMENTAL_FS)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#if defined(__APPLE__)
#include <unistd.h>
#endif
#endif

#include <cstdint>
#include <iomanip>
#include "CRSDK/CameraRemote_SDK.h"
#include "CameraDevice.h"
#include "Text.h"

//#define LIVEVIEW_ENB

//#define MSEARCH_ENB

namespace SDK = SCRSDK;

int main()
{
    // Change global locale to native locale
    std::locale::global(std::locale(""));

    // Make the stream's locale the same as the current global locale
    cli::tin.imbue(std::locale());
    cli::tout.imbue(std::locale());

    cli::tout << "RemoteSampleApp v2.00.00 running...\n\n";

    CrInt32u version = SDK::GetSDKVersion();
    int major = (version & 0xFF000000) >> 24;
    int minor = (version & 0x00FF0000) >> 16;
    int patch = (version & 0x0000FF00) >> 8;
    // int reserved = (version & 0x000000FF);

    cli::tout << "Remote SDK version: ";
    cli::tout << major << "." << minor << "." << std::setfill(TEXT('0')) << std::setw(2) << patch << "\n";

    cli::tout << "Initialize Remote SDK...\n";

#if defined(__APPLE__)
        char path[MAC_MAX_PATH]; /*MAX_PATH*/
        memset(path, 0, sizeof(path));
        if(NULL == getcwd(path, sizeof(path) - 1)){
            cli::tout << "Folder path is too long.\n";
            std::exit(EXIT_FAILURE);
            return;
        }
        getcwd(path, sizeof(path) -1);
        cli::tout << "Working directory: " << path << '\n';
#else
        cli::tout << "Working directory: " << fs::current_path() << '\n';
#endif

    auto init_success = SDK::Init();
    if (!init_success) {
        cli::tout << "Failed to initialize Remote SDK. Terminating.\n";
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    cli::tout << "Remote SDK successfully initialized.\n\n";

#ifdef MSEARCH_ENB
    cli::tout << "Enumerate connected camera devices...\n";
    SDK::ICrEnumCameraObjectInfo* camera_list = nullptr;
    auto enum_status = SDK::EnumCameraObjects(&camera_list);
    if (CR_FAILED(enum_status) || camera_list == nullptr) {
        cli::tout << "No cameras detected. Connect a camera and retry.\n";
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    auto ncams = camera_list->GetCount();
    cli::tout << "Camera enumeration successful. " << ncams << " detected.\n\n";

    for (CrInt32u i = 0; i < ncams; ++i) {
        auto camera_info = camera_list->GetCameraObjectInfo(i);
        cli::text conn_type(camera_info->GetConnectionTypeName());
        cli::text model(camera_info->GetModel());
        cli::text id = TEXT("");
        if (TEXT("IP") == conn_type) {
            //id.append((TCHAR*)camera_info->GetMACAddressChar());
            // or
            id.append((TCHAR*)camera_info->GetMACAddressChar(), (size_t)camera_info->GetMACAddressCharSize());
        }
        else id = ((TCHAR*)camera_info->GetId());
        cli::tout << '[' << i + 1 << "] " << model.data() << " (" << id.data() << ")\n";
    }

    cli::tout << std::endl << "Connect to camera with input number...\n";
    cli::tout << "input> ";
    cli::text connectNo;
    std::getline(cli::tin, connectNo);
    cli::tout << '\n';

    cli::tsmatch smatch;
    CrInt32u no = 0;
    while (true) {
        no = 0;
        if (std::regex_search(connectNo, smatch, cli::tregex(TEXT("[0-9]")))) {
            no = stoi(connectNo);
            if (no == 0)
                break; // finish

            if (camera_list->GetCount() < no) {
                cli::tout << "input value over \n";
                cli::tout << "input> "; // Redo
                std::getline(cli::tin, connectNo);
                continue;
            }
            else
                break; // ok
        }
        else
            break; // not number
    }

    if (no == 0) {
        cli::tout << "Invalid Number. Finish App.\n";
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }

    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    typedef std::vector<CameraDevicePtr> CameraDeviceList;
    CameraDeviceList cameraList; // all
    std::int32_t cameraNumUniq = 1;
    std::int32_t selectCamera = 1;

    cli::tout << "Connect to selected camera...\n";
    auto* camera_info = camera_list->GetCameraObjectInfo(no - 1);

    cli::tout << "Create camera SDK camera callback object.\n";
    CameraDevicePtr camera = CameraDevicePtr(new cli::CameraDevice(cameraNumUniq, camera_info));
    cameraList.push_back(camera); // add 1st

    cli::tout << "Release enumerated camera list.\n";
    camera_list->Release();

    // Overview
    //   loop-A : main loop
    //   loop-B : TOP-MENU ____ Switching between 'Remote Control Mode' connection and 'Contents Transfer Mode' connection.
    //   loop-C : REMOTE-MENU
    //   loop-D : CONTENTS-MENU
#else
    SDK::CrError err = SDK::CrError_None;

    CrChar* serialNum = new CrChar[SDK::USB_SERIAL_LENGTH + 1];
    int serialSiz = sizeof(CrChar) * (SDK::USB_SERIAL_LENGTH + 1);
    memset(serialNum, 0, serialSiz);
#if defined (_WIN32) || defined(_WIN64) 
    MemCpyEx(serialNum, L"123456789012", serialSiz);// dummy data
#else
    strncpy((char*)serialNum, "123456789012", serialSiz);// dummy data
#endif
    CrInt32 ipAddr = 0x0A00A8C0; // 192.168.0.10
    CrInt8u macAddr[6] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC }; // dummy data
    SDK::CrCameraDeviceModelList usbModel = SDK::CrCameraDeviceModelList::CrCameraDeviceModel_ILCE_9M2;
    SDK::CrCameraDeviceModelList ethernetModel = SDK::CrCameraDeviceModelList::CrCameraDeviceModel_ILME_FX6;

    SDK::ICrCameraObjectInfo* pCam = nullptr;
    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    typedef std::vector<CameraDevicePtr> CameraDeviceList;

    CameraDeviceList cameraList; // all
    std::int32_t cameraNumUniq = 1;
    std::int32_t selectCamera = 1;
    CameraDevicePtr camera;
#if 0
    // USB
    pCam = nullptr;
    err = SDK::CreateCameraObjectInfoUSBConnection(&pCam, usbModel, (unsigned char*)serialNum);
    if (err == 0) {
        cli::tout << "[" << cameraNumUniq << "] " << pCam->GetModel() << "(" << (TCHAR*)pCam->GetId() << ")\n";
        camera = CameraDevicePtr(new cli::CameraDevice(cameraNumUniq, pCam));
        cameraList.push_back(camera);
        cameraNumUniq++;
    }
#endif
    // Ether
    pCam = nullptr;
    CrInt32u inputSshSupport = 0; // OFF
    {
        cli::tout << std::endl << "Please enter the IP address\n  ex) 192.168.0.10\n";
        cli::tout << "input > ";
        cli::text inputIp;
        std::getline(cli::tin, inputIp);
        cli::text copyIp = inputIp.c_str();
        cli::text_stringstream ss(inputIp);
        std::vector<cli::text> vwords{};
        size_t pos = 0;
        int cntSeg = 0;
        while ((pos = inputIp.find('.')) != std::string::npos) {
            vwords.push_back(inputIp.substr(0, pos));
            inputIp.erase(0, pos + 1); // 1 = delimiter length
        }
        if (4 > vwords.size()) vwords.push_back(inputIp); // last segment
        cntSeg = 0;
        CrInt32 inputAddr = 0;
        for (const auto & ipSeg : vwords) {
            //cli::tout << ipSeg;
            cli::text_stringstream seg(ipSeg);
            int decSeg = 0;
            seg >> decSeg;
            if (4 > cntSeg) {
                inputAddr += (decSeg << (8 * cntSeg++));
            }
        }
        if (4 <= cntSeg) {
            ipAddr = inputAddr; // update
            cli::tout << "Set IP address: " << copyIp << std::endl;
        }
        else
        {
            cli::tout << "Set IP address: 192.168.0.10" << std::endl;
        }

        cli::tout << std::endl << "Is it an SSH connection? (y/n)\n";
        cli::tout << "input> ";
        cli::text inputYesNo;
        std::getline(cli::tin, inputYesNo);
        if (inputYesNo == TEXT("y")) {
            cli::tout << "Connect with SSH\n" << std::endl;
            inputSshSupport = 1;
        }
        else {
            cli::tout << "Connect with non-SSH\n" << std::endl;
        }
    }
    err = SDK::CreateCameraObjectInfoEthernetConnection(&pCam, ethernetModel, ipAddr, (unsigned char*)macAddr, inputSshSupport);
    if (err == 0) {
        cli::tout << "[" << cameraNumUniq << "] " << pCam->GetModel() << "(" << pCam->GetMACAddressChar() << ")";
        camera = CameraDevicePtr(new cli::CameraDevice(cameraNumUniq, pCam));
        cameraList.push_back(camera);
        cameraNumUniq++;
    }
    pCam = nullptr;
        
    cli::tout << std::endl << "Connect to camera with input number...\n";
    cli::tout << "input> ";
    cli::text connectNo;
    std::getline(cli::tin, connectNo);
    cli::tout << '\n';
    cli::tsmatch smatch;
    CrInt32u no = 0;
    while (true) {
        no = 0;
        if (std::regex_search(connectNo, smatch, cli::tregex(TEXT("[0-9]")))) {
            no = stoi(connectNo);
            if (no == 0)
                break; // finish

            if (cameraList.size() < no) {
                cli::tout << "input value over \n";
                cli::tout << "input> "; // Redo
                std::getline(cli::tin, connectNo);
                continue;
            }
            else
                break; // ok
        }
        else
            break; // not number
    }

    if (no == 0) {
        cli::tout << "Invalid Number. Finish App.\n";
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    selectCamera = no;
    camera = cameraList[no - 1];

#endif
    // loop-A
    for (;;) {

        // 1,2   = break of TOP-MENU, continue to REMOTE-MENU or CONTENTS-MENU
        // x     = quit the app
        // other = ignore
        bool bQuit = false;
        // loop-B
        while (true)
        {
            cli::tout << "<< TOP-MENU >>\nWhat would you like to do? Enter the corresponding number.\n";
            cli::tout
                << "(1) Connect (Remote Control Mode)\n"
                << "(2) Connect (Contents Transfer Mode)\n"
                << "(3) Connect (Remote Transfer Mode)\n"
                // << "(f) Release Device \n"
                << "(x) Exit\n";

            cli::tout << "input> ";
            cli::text action;
            std::getline(cli::tin, action);
            cli::tout << '\n';

            if (action == TEXT("x")) { /* Exit app */
                bQuit = true;
                CameraDeviceList::const_iterator it = cameraList.begin();
                for (std::int32_t j = 0; it != cameraList.end(); ++j, ++it) {
                    if ((*it)->is_connected()) {
                        cli::tout << "Initiate disconnect sequence.\n";
                        auto disconnect_status = (*it)->disconnect();
                        if (!disconnect_status) {
                            // try again
                            disconnect_status = (*it)->disconnect();
                        }
                        if (!disconnect_status)
                            cli::tout << "Disconnect failed to initiate.\n";
                        else
                            cli::tout << "Disconnect successfully initiated!\n\n";
                    }
                    (*it)->release();
                }
                break; // exit loop-B
            }
            //else if (action == TEXT("f")) { /* ReleaseDevice */
            //    if (2 > cameraList.size()) {
            //        cli::tout << std::endl << "Does not execute if there is no camera to switch control after the release device of the specified camera." << std::endl;
            //    }
            //    else {
            //        CameraDeviceList::const_iterator it = cameraList.begin();
            //        for (; it != cameraList.end(); ++it) {
            //            if ((*it)->get_number() == camera->get_number()) {
            //                (*it)->release();
            //                cameraList.erase(it);
            //                break;
            //            }
            //        }
            //        it = cameraList.begin(); // switch to top of list
            //        camera = (*it);
            //        selectCamera = camera->get_number();
            //    }
            //}
            else if (action == TEXT("1")) { /* connect Remote */
                if (camera->is_connected()) {
                    cli::tout << "Please disconnect\n";
                }
                else {
                     camera->connect(SDK::CrSdkControlMode_Remote, SDK::CrReconnecting_ON);
                }
                break; // exit loop-B
            }
            else if (action == TEXT("2")) { /* connect Contents Transfer */
                if (camera->is_connected()) {
                    cli::tout << "Please disconnect\n";
                }
                else {
                    camera->connect(SDK::CrSdkControlMode_ContentsTransfer, SDK::CrReconnecting_ON);
                }
                break; // exit loop-B
            }
            else if (action == TEXT("3")) { /* connect Remote Transfer */
                if (camera->is_connected()) {
                    cli::tout << "Please disconnect\n";
                }
                else {
                    camera->connect(SDK::CrSdkControlMode_RemoteTransfer, SDK::CrReconnecting_ON);
                }
                break; // exit loop-B
            }
            cli::tout << std::endl;

        } // end of loop-B

        // check quit
        if (bQuit) break; // exit loop-A

        // ------------------------
        // Remote
        // ------------------------
        if ((SDK::CrSdkControlMode_Remote == camera->get_sdkmode()) ||
            (SDK::CrSdkControlMode_RemoteTransfer == camera->get_sdkmode()))
        {
            if(SDK::CrSdkControlMode_Remote == camera->get_sdkmode()){
                cli::tout << TEXT("Remote Control Mode\n");
            }else if(SDK::CrSdkControlMode_RemoteTransfer == camera->get_sdkmode()){
                cli::tout << TEXT("Remote Transfer Mode\n");
            }

            // loop-C
            while (true)
            {
                if ((SDK::CrSSHsupportValue::CrSSHsupport_ON == camera->get_sshsupport()) && (false == camera->is_getfingerprint()))
                {
                    // Fingerprint is incorrect
                    break;
                }
                cli::tout << "<< REMOTE-MENU >>\nWhat would you like to do? Enter the corresponding number.\n";
                cli::tout
                    << "(s) Status display and camera switching \n"
                    << "(0) Disconnect and return to the top menu\n"
                    << "(1) Shutter/Rec Operation Menu \n"
                    << "(2) Shooting Menu \n"
                    << "(3) Exposure/Color Menu \n"
                    << "(4) Focus Menu \n"
                    << "(5) Setup Menu \n"
                    << "(6) Contents Operation Menu \n"
                    << "(7) Other Menu \n"
                    ;

                cli::tout << "input> ";
                cli::text action;
                std::getline(cli::tin, action);
                cli::tout << '\n';

                if (action == TEXT("s")) { /* status display and device selection */
                    cli::tout << "Status display and camera switching.\n";
#if defined(LIVEVIEW_ENB)
                    cli::tout << "number - connected - lvEnb - model - id\n";
#else
                    cli::tout << "number - connected - model - id\n";
#endif
                    CameraDeviceList::const_iterator it = cameraList.begin();
                    for (std::int32_t i = 0; it != cameraList.end(); ++i, ++it)
                    {
                        cli::text model = (*it)->get_model();
                        if (model.size() < 10) {
                            int32_t appendCnt = 10 - (int32_t)model.size();
                            model.append(appendCnt, TEXT(' '));
                        }
                        cli::text id = (*it)->get_id();
                        std::uint32_t num = (*it)->get_number();
                        if (selectCamera == num) { cli::tout << "* "; }
                        else { cli::tout << "  "; }
                        cli::tout << std::setfill(TEXT(' ')) << std::setw(4) << std::left << num
                            << " - " << ((*it)->is_connected() ? "true " : "false")
#if defined(LIVEVIEW_ENB)
                            << " - " << ((*it)->is_live_view_enable() ? "true " : "false")
#endif
                            << " - " << model.data()
                            << " - " << id.data() << std::endl;

                        (*it)->printOtherInfo();
                    }

                    cli::tout << std::endl << "Selected camera number = [" << selectCamera << "]" << std::endl << std::endl;

                    cli::tout << "Choose a number:\n";
                    cli::tout << "[-1] Cancel input\n";
#ifdef MSEARCH_ENB
                    cli::tout << "[0]  Create new CameraDevice\n";
#endif
                    cli::tout << "[1]  Switch cameras for controls\n";
                    cli::tout << std::endl << "input> ";

                    cli::text input;
                    std::getline(cli::tin, input);
                    cli::text_stringstream ss(input);
                    int selected_index = 0;
                    ss >> selected_index;

#ifdef MSEARCH_ENB
                    if (selected_index < 0 || 1 < selected_index) {
                        cli::tout << "Input cancelled.\n";
                    }
#else
                    if (selected_index < 1 || 1 < selected_index) {
                        cli::tout << "Input cancelled.\n";
                    }
#endif
#ifdef MSEARCH_ENB
                    // new camera connect
                    if (0 == selected_index) 
                    {
                        enum_status = SDK::EnumCameraObjects(&camera_list);
                        if (CR_FAILED(enum_status) || camera_list == nullptr) {
                            cli::tout << "No cameras detected. Connect a camera and retry.\n";
                        }
                        else
                        {
                            cli::tout << "[-1] Cancel input\n";
                            ncams = camera_list->GetCount();
                            for (CrInt32u i = 0; i < ncams; ++i) {
                                auto camera_info = camera_list->GetCameraObjectInfo(i);
                                cli::text conn_type(camera_info->GetConnectionTypeName());
                                cli::text model(camera_info->GetModel());
                                cli::text id = TEXT("");
                                if (TEXT("IP") == conn_type) {
                                    //id.append((TCHAR*)camera_info->GetMACAddressChar());
                                    // or
                                    id.append((TCHAR*)camera_info->GetMACAddressChar(), (size_t)camera_info->GetMACAddressCharSize());
                                }
                                else id.append((TCHAR*)camera_info->GetId());
                                cli::tout << '[' << i + 1 << "] " << model.data() << " (" << id.data() << ") ";
                                CameraDeviceList::const_iterator it = cameraList.begin();
                                for (std::int32_t j = 0; it != cameraList.end(); ++j, ++it){
                                    cli::text alreadyId = (*it)->get_id();
                                    if (0 == id.compare(alreadyId)) {
                                        cli::tout << "*";
                                        break;
                                    }
                                }
                                cli::tout << std::endl;
                            }

                            cli::tout << std::endl << "idx input> ";
                            std::getline(cli::tin, input);
                            cli::text_stringstream ss2(input);
                            int selected_no = 0;
                            ss2 >> selected_no;

                            if (selected_no < 1 || (std::int32_t)ncams < selected_no) {
                                cli::tout << "Input cancelled.\n";
                            }
                            else {
                                camera_info = camera_list->GetCameraObjectInfo(selected_no - 1);
                                cli::text conn_type(camera_info->GetConnectionTypeName());
                                cli::text model_select(camera_info->GetModel());
                                cli::text id_select = TEXT("");
                                if (TEXT("IP") == conn_type) {
                                    //id_select.append((TCHAR*)camera_info->GetMACAddressChar());
                                    // or
                                    id_select.append((TCHAR*)camera_info->GetMACAddressChar(), (size_t)camera_info->GetMACAddressCharSize());
                                }
                                else id_select.append((TCHAR*)camera_info->GetId());
                                bool findAlready = false;
                                CameraDeviceList::const_iterator it = cameraList.begin();
                                for (std::int32_t j = 0; it != cameraList.end(); ++j, ++it) {
                                    if ((0 == (*it)->get_model().compare(model_select)) &&
                                        (0 == (*it)->get_id().compare(id_select))) {
                                        findAlready = true;
                                        cli::tout << "Already connected!\n";
                                        break;
                                    }
                                }
                                if (false == findAlready) {
                                    std::int32_t newNum = cameraNumUniq + 1;
                                    CameraDevicePtr newCam = CameraDevicePtr(new cli::CameraDevice(newNum, camera_info));
                                    cameraNumUniq = newNum;
                                    cameraList.push_back(newCam); // add
                                    camera = newCam; // switch target
                                    selectCamera = cameraNumUniq; // latest
                                    break; // exit loop-C
                                }
                            }
                            camera_list->Release();
                        }
                    }
#endif
                    // switch device
                    else if (1 == selected_index) {
                        cli::tout << std::endl << "number input> ";
                        std::getline(cli::tin, input);
                        cli::text_stringstream ss3(input);
                        int input_no = 0;
                        ss3 >> input_no;

                        if (input_no < 1) {
                            cli::tout << "Input cancelled.\n";
                        }
                        else {
                            bool findTarget = false;
                            CameraDeviceList::const_iterator it = cameraList.begin();
                            for (; it != cameraList.end(); ++it) {
                                if ((*it)->get_number() == input_no) {
                                    findTarget = true;
                                    camera = (*it);
                                    selectCamera = input_no;
                                    break;
                                }
                            }
                            if (!findTarget) {
                                cli::tout << "The specified camera cannot be found!\n";
                            }
                        }
                    }
                } // end menu-s

                else if (action == TEXT("0")) { /* Return top menu */
                    if (camera->is_connected()) {
                        camera->disconnect();
                    }
                    break; // exit loop-B
                }
                else if (action == TEXT("1")) { /* Shutter/Rec Operation Menu */
                    // loop-Menu1
                    while (true) {
                        cli::tout << "<< Shutter/Rec Operation Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                        cli::tout
                            << "(0) Return to REMOTE-MENU\n"
                            << "(1) Shutter Release \n"
                            << "(2) Shutter Half Release in AF mode \n"
                            << "(3) Shutter Half and Full Release in AF mode \n"
                            << "(4) Continuous Shooting \n"
                            << "(5) Focus Bracket Shot \n"
                            << "(6) Movie Rec Button \n"
                            << "(7) Movie Rec Button(Toggle) \n"
                            ;

                        cli::tout << "input> ";
                        cli::text select;
                        std::getline(cli::tin, select);
                        cli::tout << '\n';
                        if (select == TEXT("1")) { /* Take photo */
                            camera->capture_image();
                        }
                        else if (select == TEXT("2")) { /* S1 Shooting */
                            camera->s1_shooting();
                        }
                        else if (select == TEXT("3")) { /* AF Shutter */
                            camera->af_shutter();
                        }
                        else if (select == TEXT("4")) { /* Continuous Shooting */
                            camera->continuous_shooting();
                        }
                        else if (select == TEXT("5")) { /* Focus Bracket shooting */
                            camera->execute_focus_bracket();
                        }
                        else if (select == TEXT("6")) { /* Movie Rec Button */
                            camera->execute_movie_rec();
                        }
                        else if (select == TEXT("7")) { /* Movie Rec Button(Toggle) */
                            camera->execute_movie_rec_toggle();
                        }
                        else if (select == TEXT("0")) {
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }
                        cli::tout << std::endl;
                    } // end of loop-Menu1
                }
                else if (action == TEXT("2")) { /* Shooting Menu */
                    // loop-Menu2
                    while (true) {
                        cli::tout << "<< Shooting Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                        cli::tout
                            << "(0) Return to REMOTE-MENU\n"
                            << "(1) Still Capture Mode(Drive mode) \n"
                            << "(2) Zoom Operation \n"
                            << "(3) Zoom Speed Type \n"
                            << "(4) Preset Focus and Zoom \n"
                            << "(5) APS-C/FULL setting \n"
                            << "(6) Set Focus Bracket Shot Num \n"
                            << "(7) Set Focus Bracket Focus Range \n"
                            << "(8) Recording Setting \n"
                            << "(9)  Image Stabilization Steady Shot(Still) \n"
                            << "(10) Image Stabilization Steady Shot(Movie) \n"
                            << "(11) Silent Mode \n"
                            << "(12) Silent Mode Aperture Drive in AF \n"
                            << "(13) Silent Mode Shutter When Power OFF \n"
                            << "(14) Silent Mode Auto Pixel Mapping \n"
                            << "(15) Shutter Type \n"
                            << "(16) Movie Shooting Mode \n"
                            << "(17) TimeShift Shooting \n"
                            << "(18) Recording Setting File Name \n"
                            ;

                        cli::tout << "input> ";
                        cli::text select;
                        std::getline(cli::tin, select);
                        cli::tout << '\n';
                        if (select == TEXT("1")) {  /* Still Capture Mode(Drive mode) */
                            camera->get_still_capture_mode();
                            camera->set_still_capture_mode();
                        }
                        else if (select == TEXT("2")) { /* Zoom Operation */
                            camera->get_zoom_operation();
                            camera->set_zoom_operation();
                        }
                        else if (select == TEXT("3")) { /* Remocon Zoom Speed Type */
                            camera->get_remocon_zoom_speed_type();
                            camera->set_remocon_zoom_speed_type();
                        }
                        else if (select == TEXT("4")) { /* Preset Focus and Zoom */
                            camera->execute_preset_focus();
                        }
                        else if (select == TEXT("5")) { /* APS-C or Full Switching */
                            camera->execute_APS_C_or_Full();
                        }
                        else if (select == TEXT("6")) { /* Set Focus Bracket Shot Num */
                            camera->set_focus_bracket_shot_num();
                        }
                        else if (select == TEXT("7")) { /* Set Focus Bracket Focus Range*/
                            camera->set_focus_bracket_focus_range();
                        }
                        else if (select == TEXT("8")) { /* Recording Setting */
                            camera->set_recording_setting();
                        }
                        else if (select == TEXT("9")) { /* Image Stabilization Steady Shot((Still) */
                            camera->set_image_stabilization_steady_shot();
                        }
                        else if (select == TEXT("10")) { /* Image Stabilization Steady Shot(Movie) */
                            camera->set_movie_image_stabilization_steady_shot();
                        }
                        else if (select == TEXT("11")) { /* Silent Mode */
                            camera->set_silent_mode();
                        }
                        else if (select == TEXT("12")) { /* Silent Mode Aperture Drive in AF */
                            camera->set_silent_mode_aperture_drive_in_af();
                        }
                        else if (select == TEXT("13")) { /* Silent Mode Shutter When Power OFF */
                            camera->set_silent_mode_shutter_when_power_off();
                        }
                        else if (select == TEXT("14")) { /* Silent Mode Auto Pixel Mapping */
                            camera->set_silent_mode_auto_pixel_mapping();
                        }
                        else if (select == TEXT("15")) { /* Shutter Type */
                            camera->set_shutter_type();
                        }
                        else if (select == TEXT("16")) { /* Movie Shooting Mode */
                            camera->set_movie_shooting_mode();
                        }
                        else if (select == TEXT("17")) { /* TimeShift Shooting */
                            camera->set_time_shift_shooting();
                        }
                        else if (select == TEXT("18")) { /* Recording Setting File Name */
                            camera->set_recording_setting_file_name();
                        }
                        else if (select == TEXT("0")) {
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }
                        cli::tout << std::endl;
                    } // end of loop-Menu2
                }
                else if (action == TEXT("3")) { /* Exposure/Color Menu */
                    // loop-Menu3
                    while (true) {
                        cli::tout << "<< Exposure/Color Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                        cli::tout
                            << "(0) Return to REMOTE-MENU\n"
                            << "(1) Aperture \n"
                            << "(2) Shutter Speed \n"
                            << "(3) Shutter Speed Value \n"
                            << "(4) Exposure Program Mode \n"
                            << "(5) FEL Lock \n"
                            << "(6) AWB Lock \n"
                            << "(7) White Balance \n"
                            << "(8) White Balance Tint \n"
                            << "(9) Custom WB \n"
                            << "(10) ISO \n"
                            << "(11) ISO Current Sensitivity \n"
                            << "(12) Gain Control Setting \n"
                            << "(13) Gain Base ISO Sensitivity \n"
                            << "(14) Gain Base Sensitivity \n"
                            << "(15) Gain dB Value \n"
                            << "(16) Exposure Index \n"
                            << "(17) BaseLook Value  \n"
                            << "(18) Exposure Control Type \n"
                            << "(19) Iris Mode Setting \n"
                            << "(20) Shutter Mode Setting \n"
                            << "(21) Custom WB Size Setting \n"
                            << "(22) Extended Shutter Speed \n"
                            ;

                        cli::tout << "input> ";
                        cli::text select;
                        std::getline(cli::tin, select);
                        cli::tout << '\n';
                        if (select == TEXT("1")) {  /* Aperture. */
                            camera->get_aperture();
                            camera->set_aperture();
                        }
                        else if (select == TEXT("2")) { /* Shutter Speed */
                            camera->get_shutter_speed();
                            camera->set_shutter_speed();
                        }
                        else if (select == TEXT("3")) { /* Shutter Speed Value */
                            camera->set_shutter_speed_value();
                        }
                        else if (select == TEXT("4")) { /* Exposure Program Mode */
                            camera->get_exposure_program_mode();
                            camera->set_exposure_program_mode();
                        }
                        else if (select == TEXT("5")) { /* FEL Lock */
                            cli::tout << "Flash device required.";
                            camera->execute_lock_property((CrInt16u)SDK::CrDevicePropertyCode::CrDeviceProperty_FEL);
                        }
                        else if (select == TEXT("6")) { /* AWB Lock */
                            camera->execute_lock_property((CrInt16u)SDK::CrDevicePropertyCode::CrDeviceProperty_AWBL);
                        }
                        else if (select == TEXT("7")) { /* White Balance */
                            camera->get_white_balance();
                            camera->set_white_balance();
                        }
                        else if (select == TEXT("8")) { /* White Balance Tint */
                            camera->set_white_balance_tint();
                        }
                        else if (select == TEXT("9")) { /* Custom WB */
                            camera->get_custom_wb();
                            camera->set_custom_wb();
                        }
                        else if (select == TEXT("10")) { /* ISO */
                            camera->get_iso();
                            camera->set_iso();
                        }
                        else if (select == TEXT("11")) { /* ISO Current Sensitivity*/
                            camera->get_iso_current_sensitivity();
                        }
                        else if (select == TEXT("12")) { /* Gain Control Setting */
                            camera->set_gain_control_setting();
                        }
                        else if (select == TEXT("13")) { /* Gain Base ISO Sensitivity */
                            camera->set_gain_base_iso_sensitivity();
                        }
                        else if (select == TEXT("14")) { /* Gain Base Sensitivity */
                            camera->set_gain_base_sensitivity();
                        }
                        else if (select == TEXT("15")) { /* Gain dB Value */
                            camera->set_gain_db_value();
                        }
                        else if (select == TEXT("16")) { /* Exposure Index */
                            camera->set_exposure_index();
                        }
                        else if (select == TEXT("17")) {  /* BaseLook Value */
                            camera->set_baselook_value();
                        }
                        else if (select == TEXT("18")) { /* Exposure Ctrl Type */
                            camera->set_exposure_control_type();
                        }
                        else if (select == TEXT("19")) { /* Iris Mode Setting */
                            camera->set_iris_mode_setting();
                        }
                        else if (select == TEXT("20")) { /* Shutter Mode Setting */
                            camera->set_shutter_mode_setting();
                        }
                        else if (select == TEXT("21")) {  /* Custom WB Size Setting */
                            camera->set_custom_wb_size_setting();
                        }
                        else if (select == TEXT("22")) { /* Extended Shutter Speed */
                            camera->set_extended_shutter_speed();
                        }
                        else if (select == TEXT("0")) {
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }
                        cli::tout << std::endl;
                    } // end of loop-Menu3
                }
                else if (action == TEXT("4")) { /* Focus Menu */
                    // loop-Menu4
                    while (true)
                    {

                        cli::tout << "<< Focus Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                        cli::tout
                            << "(0) Return to REMOTE-MENU\n"
                            << "(1) Focus Mode \n"
                            << "(2) Focus Area \n"
                            << "(3) AF Area Position(x,y) \n"
                            << "(4) Focus Position Setting \n"
                            << "(5) Zoom Position Setting \n"
                            ;

                        cli::tout << "input> ";
                        cli::text select;
                        std::getline(cli::tin, select);
                        cli::tout << '\n';
                        if (select == TEXT("1")) {  /* Focus Mode */
                            camera->get_focus_mode();
                            camera->set_focus_mode();
                        }
                        else if (select == TEXT("2")) { /* Focus Area */
                            camera->get_focus_area();
                            camera->set_focus_area();
                        }
                        else if (select == TEXT("3")) { /* AF Area Position(x,y) */
                            camera->set_af_area_position();
                        }
                        else if (select == TEXT("4")) { /* Focus Position Setting */
                            camera->set_focus_position_setting();
                        }
                        else if (select == TEXT("5")) { /* Zoom Position Setting */
                            camera->set_zoom_position_setting();
                        }
                        else if (select == TEXT("0")) {
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }
                        cli::tout << std::endl;
                    } // end of loop-Menu4
                }
                else if (action == TEXT("5")) { /* Setup Menu */
                // loop-Menu5
                    while (true) {
                        cli::tout << "<< Setup Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                        cli::tout
                            << "(0) Return to REMOTE-MENU\n"
                            << "(1) DISP(Screen Disp) Setting \n"
                            << "(2) Monitor LUT Setting \n"
                            << "(3) Download the camera setting file to PC \n"
                            << "(4) Upload the camera setting file to Camera \n"
                            << "(5) Camera Setting Reset \n"
                            << "(6) RemoteFirmwareUpdate Menu \n"
                            ;

                        cli::tout << "input> ";
                        cli::text select;
                        std::getline(cli::tin, select);
                        cli::tout << '\n';
                        if (select == TEXT("1")) {  /* DisplayMode */
                            camera->set_dispmode();
                        }
                        else if (select == TEXT("2")) { /* Monitor LUT Setting */
                            camera->set_monitor_lut_setting();
                        }
                        else if (select == TEXT("3")) { /* Download camera setting file */
                            camera->do_download_camera_setting_file();
                        }
                        else if (select == TEXT("4")) { /* Upload camera setting file */
                            camera->do_upload_camera_setting_file();
                        }
                        else if (select == TEXT("5")) { /* Camera Setting Reset */
                            camera->execute_camera_setting_reset();
                        }
                        else if (select == TEXT("6")) { /* RemoteFirmwareUpdate Menu */
                            // loop-Menu6
                            while (true) {
                                cli::tout << "<< RemoteFirmwareUpdate Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                                cli::tout
                                    << "(0) Return to Setup menu\n"
                                    << "(1) Get Firmware Version \n"
                                    << "(2) Check Firmware Update Status \n"
                                    << "(3) Upload Firmware \n"
                                    << "(4) Get Upload Rate for Firmware File \n"
                                    << "(5) Cancel Upload Firmware \n"
                                    << "(6) Execute Firmware Update \n"
                                    << "(7) Get Firmware Updater Info \n"
                                    << "(8) Set Debug Mode \n"
                                    ;

                                cli::tout << "input> ";
                                cli::text select;
                                std::getline(cli::tin, select);
                                cli::tout << '\n';
                                if (select == TEXT("1")) { /* Get Firmware Version */
                                    camera->get_firmware_version();
                                }
                                else if (select == TEXT("2")) { /* Check Firmware Update Status */
                                    camera->check_firmware_update_status();
                                }
                                else if (select == TEXT("3")) { /* Upload Firmware */
                                    camera->upload_firmware();
                                }
                                else if (select == TEXT("4")) { /* Get Upload Rate for Firmware File */
                                    camera->get_firmware_upload_rate();
                                }
                                else if (select == TEXT("5")) { /* Cancel Upload Firmware */
                                    camera->cancel_upload_firmware();
                                }
                                else if (select == TEXT("6")) { /* Execute Firmware Update */
                                    camera->execute_update_firmware();
                                }
                                else if (select == TEXT("7")) { /* Get Firmware Updater Info */
                                    camera->get_firmware_updater_info();
                                }
                                else if(select == TEXT("8")) { /* Set Debug Mode */
                                    camera->set_debug_mode();
                                }
                                else if (select == TEXT("0")) {
                                    cli::tout << "Return to Setup menu.\n";
                                    break;
                                }
                            }
                            // end of loop-Menu6
                        }
                        else if (select == TEXT("0")) {
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }
                        cli::tout << std::endl;
                    } // end of loop-Menu5
                }
                else if (action == TEXT("6")) { /* Contents Operation Menu */
                    // loop-Menu6
                    while (true) {
                        if(SDK::CrSdkControlMode_RemoteTransfer != camera->get_sdkmode()){
                            cli::tout << "Remote Control Mode is Not Support\n";
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }

                        cli::tout << "<< Contents Operation Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                        cli::tout
                            << "(0) Return to REMOTE-MENU\n"
                            << "(1) Get Contents Data \n"
                            << "(2) Movie Playback \n"
                            ;

                        cli::tout << "input> ";
                        cli::text select;
                        std::getline(cli::tin, select);
                        cli::tout << '\n';
                        if (select == TEXT("1")) { /* Get Contents Data */
                            camera->get_remote_transfer_contentsdata();                   
                        }
                        else if(select == TEXT("2")) { /* Movie Playback */                      
                            if (false == camera->isMoviePlaybackFunctionSupport()) {
                                cli::tout << "Movie playback function cannot be used with this device.\n";
                            }
                            else {
                                while (true) {
                                    cli::tout << "<< Movie Playback Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                                    cli::tout
                                        << "(0) Return to Contents Operation Menu.\n"
                                        << "(1) Set Movie Playback Setting \n"
                                        << "(2) Get Movie Playback Setting \n"
                                        << "(3) Control Movie Playback \n"
                                        << "(4) Request Movie Playback Status \n"
                                        << "(5) Get Movie Playback Status \n"
                                        ;
                                    cli::tout << "input> ";
                                    cli::text select;
                                    std::getline(cli::tin, select);
                                    cli::tout << '\n';

                                    if (select == TEXT("1")) { /* Set Movie Playback Setting */
                                        camera->setMoviePlaybackSetting();
                                    }
                                    else if (select == TEXT("2")) { /* Get Movie Playback Setting */
                                        camera->getMoviePlaybackSetting();
                                    }
                                    else if (select == TEXT("3")) { /* Control Movie Playback */
                                        camera->controlMoviePlayback();
                                    }
                                    else if (select == TEXT("4")) { /* Request Movie Playback Status */
                                        camera->requestMoviePlaybackStatus();
                                    }
                                    else if (select == TEXT("5")) { /* Get Movie Playback Status */
                                        camera->getMoviePlaybackStatus();
                                    }
                                    else if (select == TEXT("0")) {
                                        cli::tout << "Return to Contents Operation Menu.\n" << std::endl;
                                        break;
                                    }
                                }
                            }
                        }
                        else if (select == TEXT("0")) {
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }
                    }
                    // end of loop-Menu6
                }
                else if (action == TEXT("7")) { /* Other Menu */
                // loop-Menu6
                    while (true) {
                        cli::tout << "<< Other Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                        cli::tout
                            << "(0) Return to REMOTE-MENU\n"
                            << "(lv) LiveView Enable \n"
                            << "(1) Live View \n"
                            << "(2) Live View Image Quality \n"
                            << "(3) Position Key Setting \n"
                            << "(4) Get Media Profile \n"
                            << "(5) Playback Media \n"
                            << "(6) Selected Media Format \n"
                            << "(7) Request Display String List \n"
                            << "(8) Get Display String Type \n"
                            << "(9) Get Display String List \n"
                            << "(10) Monitoring \n"
                            << "(11) Request Zoom And Focus Preset \n"
                            << "(12) Get Zoom And Focus Preset \n"
                            << "(13) Get OSD Image Enable Setting\n"
                            << "(14) Get OSD Image \n"
                            << "(15) Remote Key Control \n"
                            << "(16) TimeZone Setting \n"
                            << "(17) Live View Protocol \n"
                            << "(18) Control PTZF\n"
                            ;

                        cli::tout << "input> ";
                        cli::text select;
                        std::getline(cli::tin, select);
                        cli::tout << '\n';
                        if (select == TEXT("1")) { /* Live View */
                            camera->get_live_view();
                        }
                        else if (select == TEXT("2")) { /* Live View Image Quality */
                            camera->get_live_view_image_quality();
                            camera->set_live_view_image_quality();
                        }
                        else if (select == TEXT("3")) {/* Position Key Setting */
                            camera->get_position_key_setting();
                            camera->set_position_key_setting();
                        }
                        else if (select == TEXT("4")) { /* Get MediaProfile */
                            camera->get_mediaprofile();
                        }
                        else if (select == TEXT("5")) { /* Playback Media */
                            camera->set_playback_media();
                        }
                        else if (select == TEXT("6")) {  /* Selected Media Format */
                            camera->get_select_media_format();
                            camera->set_select_media_format();
                        }
                        else if (select == TEXT("7")) { /* Display String List */
                            camera->execute_request_displaystringlist();
                        }
                        else if (select == TEXT("8")) { /* Display String List */
                            camera->execute_get_displaystringtypes();
                        }
                        else if (select == TEXT("9")) {  /* Display String List  */
                            camera->execute_get_displaystringlist();
                        }
                        else if (select == TEXT("10")) {
                            if (false == camera->isMonitoringFunctionSupport()) {
                                cli::tout << "Monitoring function cannot be used with this device.\n";
                            }
                            else {
                                // loop start
                                while (true) {

                                    cli::tout << "<< Monitoring Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                                    cli::tout
                                        << "(0) Return to Other menu.\n"
                                        << "(1) Set Monitoring Delivery Setting\n"
                                        << "(2) Start Monitoring\n"
                                        << "(3) Stop Monitoring\n"
                                        ;

                                    cli::tout << "input> ";
                                    cli::text select;
                                    std::getline(cli::tin, select);
                                    cli::tout << "\n";
                                    if (select == TEXT("1")) { /* Set Monitoring Delivery Setting */
                                        camera->setMonitoringDeriverySetting();
                                    }
                                    else if (select == TEXT("2")) { /* Start Monitoring */
                                        camera->startMonitoring();
                                    }
                                    else if (select == TEXT("3")) { /* Stop Monitoring */
                                        camera->stopMonitoring();
                                    }
                                    else if (select == TEXT("0")) {
                                        cli::tout << "Return to Other menu.\n";
                                        break;
                                    }
                                    cli::tout << std::endl;
                                } // loop end
                            }
                        }
                        else if (select == TEXT("11")) { /* Request Zoom And Focus Preset */
                            camera->execute_request_zoom_and_focus_preset();
                        }
                        else if (select == TEXT("12")) { /* Get Zoom And Focus Preset */
                            camera->execute_get_zoom_and_focus_preset();
                        }
                        else if (select == TEXT("13")) {
                            camera->set_enable_get_osd_image();
                        }
                        else if (select == TEXT("14")) {
                            camera->get_osd_image();
                        }
                        else if (select == TEXT("15")) {
                            // loop start
                            while (true) {

                                cli::tout << "<< Remote Key Control Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                                cli::tout
                                    << "(0) Return to Other menu.\n"
                                    << "(1) Button Function\n"
                                    << "(2) Button Function Multi \n"
                                    << "(3) Dial Function \n"
                                    << "(4) Lever Function \n"
                                    ;

                                cli::tout << "input> ";
                                cli::text select;
                                std::getline(cli::tin, select);
                                cli::tout << "\n";
                                if (select == TEXT("1")) { /* Button Function */
                                    camera->camera_button_function(false);
                                }
                                else if (select == TEXT("2")) { /* Button Function Multi */
                                    camera->camera_button_function(true);
                                }
                                else if (select == TEXT("3")) { /* Dial Function */
                                    camera->camera_dial_function();
                                }
                                else if (select == TEXT("4")) { /* Lever Function */
                                    camera->camera_lever_function();
                                }
                                else if (select == TEXT("0")) {
                                    cli::tout << "Return to Other menu.\n";
                                    break;
                                }
                                cli::tout << std::endl;
                            } // loop end
                        }
                        else if (select == TEXT("16")) { /* TimeZone Setting */
                            // loop start
                            while (true) {

                                cli::tout << "<< TimeZone Setting Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                                cli::tout
                                    << "(0) Return to Other menu.\n"
                                    << "(1) Set TimeZone Setting\n"
                                    << "(2) Request TimeZone Setting \n"
                                    << "(3) Get TimeZone Setting \n"
                                    ;

                                cli::tout << "input> ";
                                cli::text select;
                                std::getline(cli::tin, select);
                                cli::tout << "\n";
                                if (select == TEXT("1")) { /* Set TimeZone Setting */
                                    camera->set_timezone_setting();
                                }
                                else if (select == TEXT("2")) { /* Request TimeZone Setting */
                                    camera->request_timezone_setting();
                                }
                                else if (select == TEXT("3")) { /* Get TimeZone Setting */
                                    camera->get_timezone_setting();
                                }
                                else if (select == TEXT("0")) {
                                    cli::tout << "Return to Other menu.\n";
                                    break;
                                }
                                cli::tout << std::endl;
                            } // loop end
                        }
                        else if (select == TEXT("17")) { /* Live View Protocol */
                            camera->change_live_view_protocol();
                        }
                        else if (select == TEXT("18")) { /* Control PTZF */
                            // loop start
                            while (true) {

                                cli::tout << "<< PTZF Control Menu >>\nWhat would you like to do? Enter the corresponding number.\n";
                                cli::tout
                                    << "(0) Return to Other menu.\n"
                                    << "(1) Control to Home/Reset/Cancel\n"
                                    << "(2) Control to Other type\n"
                                    << "(3) Clear Preset\n"
                                    << "(4) Set Preset\n"
                                    ;

                                cli::tout << "intput> ";
                                cli::text select;
                                std::getline(cli::tin, select);
                                cli::tout << "\n";
                                if (select == TEXT("1")) { /* Control to Home/Reset/Cancel */
                                    camera->control_ptzf_home_reset_cancel();
                                }
                                else if (select == TEXT("2")) { /* Control to Other type */
                                    camera->control_ptzf();
                                }
                                else if (select == TEXT("3")) { /* Clear Preset */
                                    camera->clear_ptzf_preset();
                                }
                                else if (select == TEXT("4")) { /* Set Preset */
                                    camera->set_ptzf_preset();
                                }
                                else if (select == TEXT("0")) {
                                    cli::tout << "Return to Other menu.\n";
                                    break;
                                }
                                cli::tout << std::endl;
                            } // loop end
                        }
                        else if (select == TEXT("lv")) { /* LiveView Enable */
                            camera->change_live_view_enable();
                        }
                        else if (select == TEXT("0")) {
                            cli::tout << "Return to REMOTE-MENU.\n";
                            break;
                        }
                        cli::tout << std::endl;
                    }// end of loop-Menu6
                }
                cli::tout << std::endl;
            } // end of loop-C
            cli::tout << std::endl;
        }
        // ------------------------
        // Contents Transfer
        // ------------------------
        else
        {
            cli::tout << TEXT("Contents Transfer Mode\n");
            // loop-D
            while (true)
            {
                cli::tout << "<< CONTENTS-MENU >>\nWhat would you like to do? Enter the corresponding number.\n";
                cli::tout
                    << "(0) Disconnect and return to the top menu\n"
                    << "(1) Get contents list \n";
                cli::tout << "input> ";
                cli::text action;
                std::getline(cli::tin, action);
                cli::tout << '\n';

                if (action == TEXT("0")) { /* Return top menu */
                    if (camera->is_connected()) {
                        camera->disconnect();
                    }
                    break; // exit loop-D
                }
                else if (action == TEXT("1")) { /* GetContentsList() */
                    if (camera->is_connected()) {
                        camera->getContentsList();
                    }
                    else
                    {
                        cli::tout << "Disconnected\n";
                        break;
                    }
                }
                if (!camera->is_connected()) {
                    break;
                }
                cli::tout << std::endl;
            } // end of loop-D
            cli::tout << std::endl;
        }

    }// end of loop-A

    cli::tout << "Release SDK resources.\n";
    SDK::Release();

    cli::tout << "Exiting application.\n";
    std::exit(EXIT_SUCCESS);
}
