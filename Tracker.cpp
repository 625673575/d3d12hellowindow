#include "Tracker.h"

namespace Tracker {
	SZVR_GetHMDConnectionStatusFunc SZVR_GetHMDConnectionStatus;
	SZVR_GetHMDDevNameFunc SZVR_GetHMDDevName;
	SZVR_GetHMDDevIPDFunc SZVR_GetHMDDevIPD;
	SZVR_GetHMDPresentFunc SZVR_GetHMDPresent;
	SZVR_GetHMDTouchpadFunc SZVR_GetHMDTouchpad;
	SZVR_GetHMDRotateFunc SZVR_GetHMDRotate;
	SZVR_GetHMDPosFunc SZVR_GetHMDPos;
	SZVR_GetHMDDevSerialNumberFunc SZVR_GetHMDDevSerialNumber;
	SZVR_GetHMDMenuButtonFunc SZVR_GetHMDMenuButton;
	SZVR_GetHMDExitButtonFunc SZVR_GetHMDExitButton;
	SZVR_GetWandConnectionStatusFunc SZVR_GetWandConnectionStatus;
	SZVR_GetWandRotateFunc SZVR_GetWandRotate;
	SZVR_GetWandPosFunc SZVR_GetWandPos;
	SZVR_GetWandButtonFunc SZVR_GetWandButton;
	SZVR_GetWandTriggerProcessFunc SZVR_GetWandTriggerProcess;
	SZVR_GetWandStickFunc SZVR_GetWandStick;
	SZVR_SetVibratorFunc SZVR_SetVibrator;
	bool LoadTrackDll()
	{

		auto TrackerParh = TEXT("E:\\UnityProject\\ThreeGlassesSDK\\Assets\\ThreeGlasses\\Plugins\\x86_64\\3GlassesTracker.dll");
		auto hModule = LoadLibrary(TrackerParh);
		if (hModule)
		{
			SZVR_GetHMDConnectionStatus = (SZVR_GetHMDConnectionStatusFunc)GetProcAddress(hModule, "SZVR_GetHMDConnectionStatus");
			SZVR_GetHMDDevIPD = (SZVR_GetHMDDevIPDFunc)GetProcAddress(hModule, "SZVR_GetHMDDevIPD");
			SZVR_GetHMDDevName = (SZVR_GetHMDDevNameFunc)GetProcAddress(hModule, "SZVR_GetHMDDevName");
			SZVR_GetHMDPresent = (SZVR_GetHMDPresentFunc)GetProcAddress(hModule, "SZVR_GetHMDPresent");
			SZVR_GetHMDTouchpad = (SZVR_GetHMDTouchpadFunc)GetProcAddress(hModule, "SZVR_GetHMDTouchpad");
			SZVR_GetHMDRotate = (SZVR_GetHMDRotateFunc)GetProcAddress(hModule, "SZVR_GetHMDRotate");
			SZVR_GetHMDPos = (SZVR_GetHMDPosFunc)GetProcAddress(hModule, "SZVR_GetHMDPos");
			SZVR_GetHMDDevSerialNumber = (SZVR_GetHMDDevSerialNumberFunc)GetProcAddress(hModule, "SZVR_GetHMDDevSerialNumber");
			SZVR_GetHMDMenuButton = (SZVR_GetHMDMenuButtonFunc)GetProcAddress(hModule, "SZVR_GetHMDMenuButton");
			SZVR_GetHMDExitButton = (SZVR_GetHMDExitButtonFunc)GetProcAddress(hModule, "SZVR_GetHMDExitButton");
			SZVR_GetWandConnectionStatus = (SZVR_GetWandConnectionStatusFunc)GetProcAddress(hModule, "SZVR_GetWandConnectionStatus");
			SZVR_GetWandRotate = (SZVR_GetWandRotateFunc)GetProcAddress(hModule, "SZVR_GetWandRotate");
			SZVR_GetWandPos = (SZVR_GetWandPosFunc)GetProcAddress(hModule, "SZVR_GetWandPos");
			SZVR_GetWandButton = (SZVR_GetWandButtonFunc)GetProcAddress(hModule, "SZVR_GetWandButton");
			SZVR_GetWandTriggerProcess = (SZVR_GetWandTriggerProcessFunc)GetProcAddress(hModule, "SZVR_GetWandTriggerProcess");
			SZVR_GetWandStick = (SZVR_GetWandStickFunc)GetProcAddress(hModule, "SZVR_GetWandStick");
			SZVR_SetVibrator = (SZVR_SetVibratorFunc)GetProcAddress(hModule, "SZVR_SetVibrator");
			DevicesFunc InitDevices = (DevicesFunc)GetProcAddress(hModule, "SZVR_InitDevices");
			typedef int(*Func)(void*, void*, void*, void*);
			Func StartTracking = (Func)GetProcAddress(hModule, "SZVR_StartTracking");
			InitDevices();
			StartTracking(nullptr, nullptr, nullptr, nullptr);
			return true;
		}

		return false;
	}
}