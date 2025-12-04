#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#if defined(__linux__)
// Use errno_t if available, define it otherwise
#include <cstring>
#ifndef __STDC_LIB_EXT1__
typedef int errno_t;
#else
#define  __STDC_WANT_LIB_EXT1__ 1
#endif

// End Linux definitions
#endif

#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <mutex>
#include "CRSDK/CameraRemote_SDK.h"
#include "CRSDK/IDeviceCallback.h"
#include "ConnectionInfo.h"
#include "PropertyValueTable.h"
#include "Text.h"
#include "MessageDefine.h"

namespace cli
{

class CRFolderInfos
{
public:
    CRFolderInfos(SCRSDK::CrMtpFolderInfo* info, int32_t nums)
        : pFolder(info)
        , numOfContents(nums)
    {};
    ~CRFolderInfos()
    {
        delete[] pFolder->folderName;
        pFolder->folderName = NULL;
        delete pFolder;
        pFolder = NULL;
    };
public:
    SCRSDK::CrMtpFolderInfo* pFolder;
    int32_t numOfContents;
};

typedef std::vector<CRFolderInfos*> MtpFolderList;
typedef std::vector<SCRSDK::CrMtpContentsInfo*> MtpContentsList;
typedef std::vector<SCRSDK::CrMediaProfileInfo*> MediaProfileList;

class CameraDevice : public SCRSDK::IDeviceCallback
{
public:
    CameraDevice() = delete;
    CameraDevice(std::int32_t no, SCRSDK::ICrCameraObjectInfo const* camera_info);
    ~CameraDevice();

    // Get fingerprint
    bool getfingerprint();

    // Try to connect to the device
    bool connect(SCRSDK::CrSdkControlMode openMode, SCRSDK::CrReconnectingSet reconnect);

    // Disconnect from the device
    bool disconnect();

    // Release from the device
    bool release();

    /*** Shooting operations ***/

    void capture_image() const;
    void s1_shooting() const;
    void af_shutter() const;
    void continuous_shooting();

    /*** Property operations ***/
    // Should be const functions, but requires load property, which is not

    void get_aperture();
    void get_iso();
    void get_shutter_speed();
    bool get_extended_shutter_speed();
    void get_position_key_setting();
    void get_exposure_program_mode();
    void get_still_capture_mode();
    void get_focus_mode();
    void get_focus_area();
    void get_live_view();
    void get_live_view_only();
    void get_live_view_and_OSD();
    void get_live_view_image_quality();
    void get_af_area_position();
    void get_select_media_format();
    void get_white_balance();
    bool get_custom_wb();
    void get_zoom_operation();
    void get_remocon_zoom_speed_type();
    bool get_aps_c_or_full_switching_setting();
    bool get_camera_setting_saveread_state();
    bool get_playback_media();
    bool get_gain_base_sensitivity();
    bool get_gain_base_iso_sensitivity();
    bool get_monitor_lut_setting();
    bool get_exposure_index();
    bool get_baselook_value();
    bool get_iris_mode_setting();
    bool get_shutter_mode_setting();
    void get_iso_current_sensitivity();
    bool get_gain_control_setting();
    bool get_exposure_control_type();
    bool get_recording_setting();
    bool get_gain_db_value();
    bool get_shutter_speed_value();
    bool get_white_balance_tint();
    void get_media_slot_status();
    bool get_movie_rec_button_toggle_enable_status();
    bool get_focus_bracket_shot_num();
    bool get_focus_bracket_focus_range();
    bool get_image_stabilization_steady_shot();
    bool get_movie_image_stabilization_steady_shot();
    bool get_silent_mode();
    bool get_silent_mode_aperture_drive_in_af();
    bool get_silent_mode_shutter_when_power_off();
    bool get_silent_mode_auto_pixel_mapping();
    bool get_shutter_type();
    bool get_focus_position_setting();
    bool get_custom_wb_size_setting();
    bool get_time_shift_shooting();
    void get_osd_image();
    bool get_zoom_position_setting();

    void set_aperture();
    void set_iso();
    bool set_save_info() const;
    void set_shutter_speed();
    void set_extended_shutter_speed();
    void set_position_key_setting();
    void set_exposure_program_mode();
    void set_still_capture_mode();
    void set_focus_mode();
    void set_focus_area();
    void set_live_view_image_quality();
    void set_af_area_position();
    void set_white_balance();
    void set_custom_wb();
    void set_zoom_operation();
    void set_remocon_zoom_speed_type();
    bool set_drive_mode(CrInt64u value);
    void execute_camera_setting_reset();
    void set_playback_media();

    void set_gain_base_iso_sensitivity();
    void set_gain_base_sensitivity();
    void set_monitor_lut_setting();
    void set_exposure_index();
    void set_baselook_value();
    void set_iris_mode_setting();
    void set_shutter_mode_setting();
    void set_gain_control_setting();
    void set_exposure_control_type();
    void set_recording_setting();
    void set_dispmode();
    void set_gain_db_value();
    void set_white_balance_tint();
    void set_shutter_speed_value();
    void set_focus_bracket_focus_range();
    void set_focus_bracket_shot_num();
    void set_movie_image_stabilization_steady_shot();
    void set_image_stabilization_steady_shot();
    void set_silent_mode();
    void set_silent_mode_aperture_drive_in_af();
    void set_silent_mode_shutter_when_power_off();
    void set_silent_mode_auto_pixel_mapping();
    void set_shutter_type();
    void set_focus_position_setting();
    void set_custom_wb_size_setting();
    void set_time_shift_shooting();
    void set_recording_setting_file_name();
    void set_enable_get_osd_image();

    void execute_lock_property(CrInt16u code);
    void set_select_media_format();
    void execute_movie_rec();
    void execute_downup_property(CrInt16u code);
    void execute_pos_xy(CrInt16u code);
    void change_live_view_enable();
    bool is_live_view_enable() { return m_lvEnbSet; };
    void execute_preset_focus();
    void execute_APS_C_or_Full();
    void execute_movie_rec_toggle();
    void do_download_camera_setting_file();
    void do_upload_camera_setting_file();

    void execute_request_displaystringlist();
    void execute_get_displaystringtypes();
    void execute_get_displaystringlist();
    void execute_focus_bracket();
    bool execute_focus_position_cancel();

    void get_mediaprofile();

    bool get_movie_shooting_mode();
    void set_movie_shooting_mode();

    void execute_request_zoom_and_focus_preset();
    void execute_get_zoom_and_focus_preset();
    bool execute_zoom_position_cancel();

    void format_focus_position_value(uint16_t value);

    void camera_button_function(bool multiFlg);
    void camera_dial_function();
    void camera_lever_function();
    SCRSDK::CrError display_support_camera_key_values(SCRSDK::CrDisplayStringType type, const std::vector<CrInt32u>& values);

    void set_zoom_position_setting();

    std::int32_t get_number() { return m_number; }
    text get_model() { return text(m_info->GetModel()); }
    text get_id();

    // Check if this device is connected
    bool is_connected() const;

    void getContentsList();
    void getFileNames(std::vector<text> &file_names);
    void pullContents(SCRSDK::CrContentHandle content);
    void getScreennail(SCRSDK::CrContentHandle content);
    void getThumbnail(SCRSDK::CrContentHandle content);

    SCRSDK::CrSdkControlMode get_sdkmode();

    CrInt32u get_sshsupport();
    bool is_getfingerprint() { return !m_fingerprint.empty(); };
    bool is_setpassword() { return !m_userPassword.empty(); };

    // Monitoring
    bool isMonitoringFunctionSupport();
    void setMonitoringDeriverySetting();
    void startMonitoring();
    void stopMonitoring();

    void printOtherInfo();
    text getCurrentStr(SCRSDK::CrDeviceProperty* prop);

    // RemoteTransfer Mode
    void get_remote_transfer_contentsdata();
    void show_contents_data_detail(SCRSDK::CrContentsInfo& contentsInfo, SCRSDK::CrContentsFile& contentsFile);
    void release_contents_info(int slotIndex);
    void execute_movie_rec_and_get_contentsdata();

    // RemoteFirmwareUpdate Mode
    void get_firmware_version();
    void check_firmware_update_status();
    void upload_firmware();
    void get_firmware_upload_rate();
    void cancel_upload_firmware();
    void execute_update_firmware();
    void get_firmware_updater_info();
    void set_debug_mode();

    void setMoviePlaybackSetting();
    bool getContentsInfoMovie(SCRSDK::CrSlotNumber* slotId, CrInt32u* contentsId, CrInt32u* fileId);
    void getMoviePlaybackSetting();
    void controlMoviePlayback();
    bool isMoviePlaybackFunctionSupport();
    void requestMoviePlaybackStatus();
    void getMoviePlaybackStatus();

    // TimeZone Setting
    void set_timezone_setting();
    void request_timezone_setting();
    void get_timezone_setting();

    void change_live_view_protocol();

    // Control PTZF
    void control_ptzf_home_reset_cancel();
    void control_ptzf();
    void clear_ptzf_preset();
    void set_ptzf_preset();

public:
    // Inherited via IDeviceCallback
    virtual void OnConnected(SCRSDK::DeviceConnectionVersioin version) override;
    virtual void OnDisconnected(CrInt32u error) override;
    virtual void OnPropertyChanged() override;
    virtual void OnLvPropertyChanged() override;
    virtual void OnCompleteDownload(CrChar* filename, CrInt32u type) override;
    virtual void OnWarning(CrInt32u warning) override;
    virtual void OnWarningExt(CrInt32u warning, CrInt32 param1, CrInt32 param2, CrInt32 param3) override;
    virtual void OnError(CrInt32u error) override;
    virtual void OnPropertyChangedCodes(CrInt32u num, CrInt32u* codes) override;
    virtual void OnLvPropertyChangedCodes(CrInt32u num, CrInt32u* codes) override;
    virtual void OnNotifyContentsTransfer(CrInt32u notify, SCRSDK::CrContentHandle contentHandle, CrChar* filename) override;
    virtual void OnNotifyFTPTransferResult(CrInt32u notify, CrInt32u numOfSuccess, CrInt32u numOfFail) override;
    virtual void OnNotifyRemoteTransferResult(CrInt32u notify, CrInt32u per, CrChar* filename) override;
    virtual void OnNotifyRemoteTransferResult(CrInt32u notify, CrInt32u per, CrInt8u* data, CrInt64u size) override;
    virtual void OnNotifyRemoteTransferContentsListChanged(CrInt32u notify, CrInt32u slotNumber, CrInt32u addSize) override;
    virtual void OnReceivePlaybackTimeCode(CrInt32u timeCode) override;
    virtual void OnReceivePlaybackData(CrInt8u mediaType, CrInt32 dataSize, CrInt8u* data, CrInt64 pts, CrInt64 dts, CrInt32 param1, CrInt32 param2) override;
    virtual void OnNotifyRemoteFirmwareUpdateResult(CrInt32u notify, const void* param) override;

private:
    void load_properties(CrInt32u num = 0, CrInt32u* codes = nullptr);
    void get_property(SCRSDK::CrDeviceProperty& prop) const;
    bool set_property(SCRSDK::CrDeviceProperty& prop) const;
    text format_dispstrlist(SCRSDK::CrDisplayStringListInfo list);
    text format_display_string_type(SCRSDK::CrDisplayStringType type);
    void check_monitoringstatus();

private:
    std::int32_t m_number;
    SCRSDK::ICrCameraObjectInfo* m_info;
    std::int64_t m_device_handle;
    std::atomic<bool> m_connected;
    ConnectionType m_conn_type;
    PropertyValueTable m_prop;
    bool m_lvEnbSet;
    SCRSDK::CrSdkControlMode m_modeSDK;
    MtpFolderList   m_foldList;
    MtpContentsList m_contentList;
    bool m_spontaneous_disconnection;
    std::atomic<bool> m_media_formatComplete;
    // DispStrList
    std::vector<SCRSDK::CrDisplayStringType> m_dispStrTypeList; // Information returned as a result of GetDisplayStringTypes
    MediaProfileList m_mediaprofileList;
    std::string m_fingerprint;
    std::string m_userPassword;
    SCRSDK::CrDeviceProperty* m_bodySerialNumberProp;
    SCRSDK::CrDeviceProperty* m_lensModelNameProp;
    SCRSDK::CrDeviceProperty* m_recordingSettingFileNameProp;
    SCRSDK::CrDeviceProperty* m_modelNameProp;
    SCRSDK::CrCaptureDate* m_captureDateList[2];
    SCRSDK::CrContentsInfo* m_contentsInfoList[2];
    bool m_getContentsDataStartFlg;
    bool m_getContentsDataMovieFlg;
    std::mutex m_getContentsDataMtx;
    std::condition_variable m_getContentsDataMovieCv;
    std::mutex m_lockgetContentsData;
    CrInt32u m_getContentsData_notify;
    CrInt32u m_getContentsData_per;

#if defined(_UNICODE) || defined(UNICODE)
    std::wstring m_getContentsData_fileName;
#else
    std::string m_getContentsData_fileName;
#endif
    std::mutex m_dispCameraKeyMutex;
    std::condition_variable m_dispCameraKeyCV;

    // RemoteFirmwareUpdate
    CrInt32u m_latestFirmwareUploadRate;
};
} // namespace cli


inline errno_t MemCpyEx(void* result, const void* source, size_t type_size)
{
#if (defined(_WIN32) || defined(_WIN64))
    return memcpy_s(result, type_size, source, type_size);
#else
    std::memcpy(result, source, type_size);
    return 0;
#endif
}

#define MAC_MAX_PATH 255

#endif // !CAMERADEVICE_H
