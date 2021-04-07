#pragma once

#include "semaphore.hpp"
#include <CRSDK/CameraRemote_SDK.h>
#include <CRSDK/IDeviceCallback.h>
#include <memory>

class Camera : public SCRSDK::IDeviceCallback {
public:
	Camera();
	virtual ~Camera();

	void capture_frame() const;

	// Inherited from SCRSDK::IDeviceCallback
	[[maybe_unused]] virtual void
	OnConnected(SCRSDK::DeviceConnectionVersioin version) override;              // NOLINT
	[[maybe_unused]] virtual void OnDisconnected(CrInt32u error) override;       // NOLINT
	[[maybe_unused]] virtual void OnPropertyChanged() override;                  // NOLINT
	[[maybe_unused]] virtual void OnLvPropertyChanged() override;                // NOLINT
	[[maybe_unused]] virtual void OnCompleteDownload(CrChar* filename) override; // NOLINT
	[[maybe_unused]] virtual void OnWarning(CrInt32u warning) override;          // NOLINT
	[[maybe_unused]] virtual void OnError(CrInt32u error) override;              // NOLINT

	[[maybe_unused]] [[nodiscard]] std::uint32_t get_shutter_release_delay() const;
	[[maybe_unused]] void set_shutter_release_delay(std::uint32_t newShutterReleaseDelay);

	const static std::uint32_t DEFAULT_SHUTTER_RELEASE_DELAY = 350000;

protected:
	/**
	 * @shutterReleaseDelay delay in microseconds between shutter press and release
	 */
	std::uint32_t shutterReleaseDelay = DEFAULT_SHUTTER_RELEASE_DELAY;
	SCRSDK::CrDeviceHandle deviceHandle{ 0 };
	SCRSDK::DeviceConnectionVersioin version;
	POSIXSemaphore connectionSemaphore;

	using CameraObjectInfo =
	    std::unique_ptr<SCRSDK::ICrCameraObjectInfo, void (*)(SCRSDK::ICrCameraObjectInfo*)>;

	static CameraObjectInfo duplicate_camera_object_info(const SCRSDK::ICrCameraObjectInfo* info);
};
