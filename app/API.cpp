#include "API.h"
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>
#include <cstdlib>

using json = nlohmann::json;

namespace api
{

API::API(int port)
    : m_port(port)
    , m_running(false)
{
}

API::~API()
{
    stop();
}

void API::start()
{

    httplib::Server server;

    // POST /v1/camera - Connect a camera
    server.Post("/v1/camera", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string ip = j.at("ip");
            std::string password = j.value("password", "");

            std::string result = handle_connect_camera(ip, password);
            
            if (!result.empty()) {
                res.status = 201;
                res.set_content(result, "application/json");
            } else {
                res.status = 400;
                res.set_content("{\"error\": \"Failed to connect camera\"}", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            json error_response = {{"error", e.what()}};
            res.set_content(error_response.dump(), "application/json");
        }
    });

    // GET /v1/camera - List cameras
    server.Get("/v1/camera", [this](const httplib::Request& req, httplib::Response& res) {
        std::string result = handle_get_cameras();
        res.status = 200;
        res.set_content(result, "application/json");
    });

    // POST /v1/camera/{serial}/recording - Toggle recording
    server.Post(R"(/v1/camera/([^/]+)/recording)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string serial = req.matches[1];
        std::string result = handle_toggle_recording(serial);
        
        if (!result.empty()) {
            res.status = 200;
            res.set_content(result, "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"error\": \"Camera not found\"}", "application/json");
        }
    });

    // PATCH /v1/camera/{serial}/recording - Toggle pause/resume
    server.Patch(R"(/v1/camera/([^/]+)/recording)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string serial = req.matches[1];
        std::string result = handle_toggle_pause(serial);
        
        if (!result.empty()) {
            res.status = 200;
            res.set_content(result, "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"error\": \"Camera not found or not recording\"}", "application/json");
        }
    });

    // GET /v1/camera/{serial}/recording - Get recording state
    server.Get(R"(/v1/camera/([^/]+)/recording)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string serial = req.matches[1];
        std::string result = handle_get_recording_state(serial);
        
        if (!result.empty()) {
            res.status = 200;
            res.set_content(result, "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"error\": \"Camera not found\"}", "application/json");
        }
    });

    m_running = true;
    std::cout << "REST API Server starting on port " << m_port << std::endl;
    server.listen("0.0.0.0", m_port);
}

void API::stop()
{
    std::lock_guard<std::mutex> lock(m_cameras_mutex);
    
    // Disconnect all cameras
    for (auto& pair : m_cameras) {
        if (pair.second.device && pair.second.device->is_connected()) {
            pair.second.device->disconnect();
        }
    }
    
    m_cameras.clear();
    m_running = false;
}
void API::enable_fake_camera(void)
{
    m_fakecam = true;
}

bool is_ip_online(const std::string& ip)
{
    std::string cmd = "ping -c 1 " + ip + " -w 1 1>/dev/null 2>/dev/null";

    int result = system(cmd.c_str());
    return (result == 0);
}

std::string API::handle_connect_camera(const std::string& ip, const std::string& password)
{
    std::lock_guard<std::mutex> lock(m_cameras_mutex);

    // Parse IP address string to CrInt32u format
    std::string inputIp = ip;
    std::vector<std::string> segments;
    size_t pos = 0;
    while ((pos = inputIp.find('.')) != std::string::npos) {
        segments.push_back(inputIp.substr(0, pos));
        inputIp.erase(0, pos + 1);
    }
    if (segments.size() < 4) segments.push_back(inputIp); // last segment
    
    CrInt32u ipAddr = 0;
    int cntSeg = 0;
    for (const auto& seg : segments) {
        if (cntSeg < 4) {
            ipAddr += (std::stoi(seg) << (8 * cntSeg++));
        }
    }

    if (ip == "192.0.2.123" && m_fakecam)
    {
        // you already know it's the fake cam, don't worry about connecting for real.

        // Check if already connected
        CameraInfo* info = find_camera_by_serial("46:41:4B:49:4E:47");
        if(!info){
            // create a fake camera
            auto fakecam = std::make_shared<cli::CameraDevice>(1,true);
            if(password != "")
            {
                fakecam->set_userpassword(password);
            }
            fakecam->connect(
                SCRSDK::CrSdkControlMode_Remote,
                SCRSDK::CrReconnecting_ON
            );
            info = new CameraInfo();
            info->serial = "46:41:4B:49:4E:47";
            info->ip = "192.0.2.123";
            info->model = "fake camera";
            info->recording_state = RecordingState::NOT_RECORDING;
            info->device = fakecam;
            m_cameras["46:41:4B:49:4E:47"] = *info;
        }
        json response = {
            {"serial",info->serial},
            {"ip",info->ip},
            {"model",info->model},
            {"recording",RecordingState::NOT_RECORDING}
        };
        return response.dump();
    }

    // Dummy MAC address
    CrInt8u macAddr[6] = {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC};
    
    // Create camera object via direct IP connection
    SCRSDK::ICrCameraObjectInfo* pCam = nullptr;
    SCRSDK::CrCameraDeviceModelList model = SCRSDK::CrCameraDeviceModelList::CrCameraDeviceModel_HXR_NX800;
    CrInt32u sshSupport = 0; // No SSH
    
    if(!is_ip_online(ip))
    {
        std::cerr << "Host at " << ip << " appears to be offline." << std::endl;
        return "";
    }

    auto err = SCRSDK::CreateCameraObjectInfoEthernetConnection(&pCam, model, ipAddr, macAddr, sshSupport);
    if (err != 0 || pCam == nullptr) {
        std::cerr << "Failed to create camera object for IP: " << ip << std::endl;
        return "";
    }

    fprintf(stdout, "[DEBUG] Camera Info:\n    Name: %s\n    Model: %s\n    UsbPid: %d\n    Id: %s\n    ConnectionType: %s\n    Adaptor Name: %s\n    Pairing Necessity: %s\n    SSHSupport: %b\n",
        pCam->GetName(),
        pCam->GetModel(),
        pCam->GetUsbPid(),
        pCam->GetId(),
        pCam->GetConnectionTypeName(),
        pCam->GetAdaptorName(),
        pCam->GetPairingNecessity(),
        (pCam->GetSSHsupport() == SCRSDK::CrSSHsupportValue::CrSSHsupport_ON)
    );
    
    // Create camera device and connect
    auto camera_device = std::make_shared<cli::CameraDevice>(1, pCam);
    if(password != "")
    {
        camera_device->set_userpassword(password);
    }
    bool connect_result = camera_device->connect(
        SCRSDK::CrSdkControlMode_Remote,
        SCRSDK::CrReconnecting_ON
    );
    
    if (!connect_result) {
        std::cerr << "Failed to connect to camera at IP: " << ip << std::endl;
        return "";
    }
    
    // Get camera info
    cli::text id_text = camera_device->get_id();
    cli::text model_text = camera_device->get_model();
    std::string serial = id_text;
    std::string model_str = model_text;
    
    // Store camera info
    CameraInfo info;
    info.serial = serial;
    info.ip = ip;
    info.model = model_str;
    info.recording_state = RecordingState::NOT_RECORDING;
    info.device = camera_device;
    
    m_cameras[serial] = info;

    // Return camera info as JSON
    json response = {
        {"serial", serial},
        {"ip", ip},
        {"model", model_str},
        {"recording", nullptr}
    };
    
    return response.dump();
}

std::string API::handle_get_cameras()
{
    std::lock_guard<std::mutex> lock(m_cameras_mutex);
    json cameras_array = json::array();

    for (const auto& pair : m_cameras) {
        const CameraInfo& info = pair.second;
        
        json camera_obj = {
            {"serial", info.serial},
            {"ip", info.ip},
            {"model", info.model},
            {"recording", recording_state_to_json(info.recording_state)}
        };

        cameras_array.push_back(camera_obj);
    }

    json response = {
        {"cameras", cameras_array}
    };

    return response.dump();
}

std::string API::handle_toggle_recording(const std::string& serial)
{
    std::lock_guard<std::mutex> lock(m_cameras_mutex);

    CameraInfo* info = find_camera_by_serial(serial);
    if (!info) {
        return "";
    }

    // Toggle recording state
    if (info->recording_state == RecordingState::NOT_RECORDING) {
        info->device->execute_movie_rec();
        info->recording_state = RecordingState::RECORDING;
    } else {
        info->device->execute_movie_rec();
        info->recording_state = RecordingState::NOT_RECORDING;
    }

    json response = {
        {"recording", recording_state_to_json(info->recording_state)}
    };

    return response.dump();
}

std::string API::handle_toggle_pause(const std::string& serial)
{
    std::lock_guard<std::mutex> lock(m_cameras_mutex);

    CameraInfo* info = find_camera_by_serial(serial);
    if (!info) {
        return "";
    }

    // Can only pause if currently recording or already paused
    if (info->recording_state == RecordingState::NOT_RECORDING) {
        return "";
    }

    // Toggle between recording and paused
    if (info->recording_state == RecordingState::RECORDING) {
        // Pause - this might require a specific API call depending on the camera
        // For now, we'll track state but the actual pause command may vary
        info->recording_state = RecordingState::PAUSED;
    } else {
        // Resume
        info->recording_state = RecordingState::RECORDING;
    }

    json response = {
        {"recording", recording_state_to_json(info->recording_state)}
    };

    return response.dump();
}

std::string API::handle_get_recording_state(const std::string& serial)
{
    std::lock_guard<std::mutex> lock(m_cameras_mutex);

    CameraInfo* info = find_camera_by_serial(serial);
    if (!info) {
        return "";
    }

    json response = {
        {"recording", recording_state_to_json(info->recording_state)}
    };

    return response.dump();
}

CameraInfo* API::find_camera_by_serial(const std::string& serial)
{
    auto it = m_cameras.find(serial);
    if (it != m_cameras.end()) {
        return &it->second;
    }
    return nullptr;
}

json API::recording_state_to_json(RecordingState state)
{
    switch (state) {
        case RecordingState::NOT_RECORDING:
            return nullptr;  // null - not recording
        case RecordingState::RECORDING:
            return true;     // true - recording (live)
        case RecordingState::PAUSED:
            return false;    // false - paused
        default:
            return nullptr;
    }
}

} // namespace api