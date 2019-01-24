#pragma once
#include <Windows.h>
#include <stdint.h>
namespace Tracker {
	typedef int(*DevicesFunc)();
	typedef int(*SZVR_GetHMDConnectionStatusFunc)(bool& result);
	typedef int(*SZVR_GetHMDDevNameFunc)(char* pstrDevname);
	typedef int(*SZVR_GetHMDDevIPDFunc)(uint8_t& value);
	typedef int(*SZVR_GetHMDPresentFunc)(bool& result);
	typedef int(*SZVR_GetHMDTouchpadFunc)(uint8_t* result);
	typedef int(*SZVR_GetHMDRotateFunc)(float* rotate);
	typedef int(*SZVR_GetHMDPosFunc)(float* pos);
	typedef int(*SZVR_GetHMDDevSerialNumberFunc)(char* pstrSerialNumber);
	typedef int(*SZVR_GetHMDMenuButtonFunc)(bool& result);
	typedef int(*SZVR_GetHMDExitButtonFunc)(bool& result);
	typedef int(*SZVR_GetWandConnectionStatusFunc)(bool status[]);
	typedef int(*SZVR_GetWandRotateFunc)(float* rotate);
	typedef int(*SZVR_GetWandPosFunc)(float* pos);
	typedef int(*SZVR_GetWandButtonFunc)(bool result[]);
	typedef int(*SZVR_GetWandTriggerProcessFunc)(uint8_t result[]);
	typedef int(*SZVR_GetWandStickFunc)(uint8_t result[]);
	typedef int(*SZVR_SetVibratorFunc)(uint32_t wand_index, uint16_t ucPower);

	extern SZVR_GetHMDConnectionStatusFunc SZVR_GetHMDConnectionStatus;
	extern SZVR_GetHMDDevNameFunc SZVR_GetHMDDevName;
	extern SZVR_GetHMDDevIPDFunc SZVR_GetHMDDevIPD;
	extern SZVR_GetHMDPresentFunc SZVR_GetHMDPresent;
	extern SZVR_GetHMDTouchpadFunc SZVR_GetHMDTouchpad;
	extern SZVR_GetHMDRotateFunc SZVR_GetHMDRotate;
	extern SZVR_GetHMDPosFunc SZVR_GetHMDPos;
	extern SZVR_GetHMDDevSerialNumberFunc SZVR_GetHMDDevSerialNumber;
	extern SZVR_GetHMDMenuButtonFunc SZVR_GetHMDMenuButton;
	extern SZVR_GetHMDExitButtonFunc SZVR_GetHMDExitButton;
	extern SZVR_GetWandConnectionStatusFunc SZVR_GetWandConnectionStatus;
	extern SZVR_GetWandRotateFunc SZVR_GetWandRotate;
	extern SZVR_GetWandPosFunc SZVR_GetWandPos;
	extern SZVR_GetWandButtonFunc SZVR_GetWandButton;
	extern SZVR_GetWandTriggerProcessFunc SZVR_GetWandTriggerProcess;
	extern SZVR_GetWandStickFunc SZVR_GetWandStick;
	extern SZVR_SetVibratorFunc SZVR_SetVibrator;
	extern bool LoadTrackDll();
}