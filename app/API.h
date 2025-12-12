#ifndef API_H
#define API_H

#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include "CameraDevice.h"
#include "CRSDK/CameraRemote_SDK.h"

namespace api
{

enum class RecordingState
{
    NOT_RECORDING,  // null
    RECORDING,      // true
    PAUSED          // false
};

struct CameraInfo
{
    std::string serial;
    std::string ip;
    std::string model;
    RecordingState recording_state;
    std::shared_ptr<cli::CameraDevice> device;
};

class API
{
public:
    API(int port = 8080);
    ~API();

    // Start the server (blocks)
    void start();

    // Stop the server
    void stop();

    // Enable a fake testing camera
    void enable_fake_camera();

private:
    // REST endpoint handlers
    std::string handle_connect_camera(const std::string& ip, const std::string& password);
    std::string handle_get_cameras();
    std::string handle_toggle_recording(const std::string& serial);
    std::string handle_toggle_pause(const std::string& serial);
    std::string handle_get_recording_state(const std::string& serial);

    // Helper methods
    CameraInfo* find_camera_by_serial(const std::string& serial);
    nlohmann::json recording_state_to_json(RecordingState state);

    int m_port;
    std::map<std::string, CameraInfo> m_cameras;
    std::mutex m_cameras_mutex;
    bool m_running;
    bool m_fakecam;
};

} // namespace api

#endif // API_H