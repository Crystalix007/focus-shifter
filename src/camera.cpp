#include "camera.hpp"
#include "CRSDK/CrCommandData.h"
#include "CRSDK/CrTypes.h"
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <unistd.h>

Camera::Camera() : connectionSemaphore{ 0 } {
	auto initSuccess = SCRSDK::Init();
	if (!initSuccess) {
		SCRSDK::Release();
		throw std::runtime_error{ "Failed to initialise camera SDK" };
	}

	CrInt32u version = SCRSDK::GetSDKVersion();
	int major = (version & 0xFF000000) >> 24;
	int minor = (version & 0x00FF0000) >> 16;
	int patch = (version & 0x0000FF00) >> 8;
	int reserved = (version & 0x000000FF);

	std::cout << "Remote SDK version: ";
	std::cout << major << "." << minor << "." << std::setfill('0') << std::setw(2) << patch << "-"
	          << reserved << "\n";

	SCRSDK::ICrEnumCameraObjectInfo* enumCameraObjects = nullptr;
	auto sdkError = SCRSDK::EnumCameraObjects(&enumCameraObjects);

	if (CR_FAILED(sdkError) || enumCameraObjects == nullptr) {
		SCRSDK::Release();
		throw std::runtime_error{ "Failed to find attached cameras" };
	}

	CrInt32 cameraCount = enumCameraObjects->GetCount();

	if (cameraCount == 0) {
		throw std::runtime_error{ "Zero attached cameras found" };
	}
	if (cameraCount > 1) {
		throw std::runtime_error{ "Too many (more than one) attached cameras found" };
	}

	// auto cameraInfo = duplicate_camera_object_info(enumCameraObjects->GetCameraObjectInfo(0));
	auto* cameraInfo = (SCRSDK::ICrCameraObjectInfo*)(enumCameraObjects->GetCameraObjectInfo(0));

	sdkError = SCRSDK::Connect(cameraInfo, this, &this->deviceHandle);

	this->connectionSemaphore.release();

	if (CR_FAILED(sdkError)) {
		throw std::runtime_error{ "Unable to connect to camera" };
	}

	SCRSDK::CrDeviceProperty stillImageStoreDest;
	stillImageStoreDest.SetCode(
	    SCRSDK::CrDevicePropertyCode::CrDeviceProperty_StillImageStoreDestination);
	stillImageStoreDest.SetCurrentValue(
	    SCRSDK::CrStillImageStoreDestination::CrStillImageStoreDestination_MemoryCard);
	stillImageStoreDest.SetValueType(SCRSDK::CrDataType::CrDataType_UInt16);

	sdkError = SCRSDK::SetDeviceSetting(this->deviceHandle,
	                                    SCRSDK::SettingKey::Setting_Key_EnableLiveView, 0);

	if (CR_FAILED(sdkError)) {
		throw std::runtime_error{ "Failed to disable live view" };
	}

	// sdkError = SCRSDK::SetDeviceProperty(this->deviceHandle, &stillImageStoreDest);

	// if (CR_FAILED(sdkError)) {
	// 	throw std::runtime_error{ "Failed to change image store destination" };
	// }

	this->capture_frame();

	enumCameraObjects->Release();
}

Camera::~Camera() {
	SCRSDK::Disconnect(this->deviceHandle);
	SCRSDK::Release();
}

void Camera::capture_frame() const {
	auto sdkError = SCRSDK::SendCommand(this->deviceHandle, SCRSDK::CrCommandId::CrCommandId_Release,
	                                    SCRSDK::CrCommandParam::CrCommandParam_Down);

	if (CR_FAILED(sdkError)) {
		throw std::runtime_error{ "Failed to send shutter press" };
	}

	usleep(this->shutterReleaseDelay);
	SCRSDK::SendCommand(this->deviceHandle, SCRSDK::CrCommandId::CrCommandId_Release,
	                    SCRSDK::CrCommandParam::CrCommandParam_Up);

	if (CR_FAILED(sdkError)) {
		throw std::runtime_error{ "Failed to send shutter release" };
	}
}

// NOLINTNEXTLINE
void Camera::OnConnected(SCRSDK::DeviceConnectionVersioin version) {
	this->version = version;

	// Wait for us to know the device handle
	this->connectionSemaphore.acquire();
}

void Camera::OnDisconnected(CrInt32u error) {}
void Camera::OnPropertyChanged() {}
void Camera::OnLvPropertyChanged() {}
void Camera::OnCompleteDownload(CrChar* filename) {}
void Camera::OnWarning(CrInt32u warning) {}
void Camera::OnError(CrInt32u error) {}

[[nodiscard]] std::uint32_t Camera::get_shutter_release_delay() const {
	return this->shutterReleaseDelay;
}

void Camera::set_shutter_release_delay(std::uint32_t newShutterReleaseDelay) {
	this->shutterReleaseDelay = newShutterReleaseDelay;
}

Camera::CameraObjectInfo
Camera::duplicate_camera_object_info(const SCRSDK::ICrCameraObjectInfo* const info) {
	// clang-format off
	SCRSDK::ICrCameraObjectInfo* duplicateInfo = SCRSDK::CreateCameraObjectInfo(
		info->GetName(),
		info->GetModel(),
		info->GetUsbPid(),
		info->GetIdType(),
		info->GetIdSize(),
		info->GetId(),
		info->GetConnectionTypeName(),
		info->GetAdaptorName(),
		info->GetPairingNecessity()
	);
	// clang-format on

	auto deleter = [](SCRSDK::ICrCameraObjectInfo* cameraInfo) { cameraInfo->Release(); };

	return CameraObjectInfo{ duplicateInfo, deleter };
}
