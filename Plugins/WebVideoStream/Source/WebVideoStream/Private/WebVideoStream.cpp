// Copyright ood11611doo. All right reserved.


#include "WebVideoStream.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "FWebVideoStreamModule"

void FWebVideoStreamModule::StartupModule()
{
	// 1. Get the base directory of this plugin
	FString PluginBaseDir = IPluginManager::Get().FindPlugin("WebVideoStream")->GetBaseDir();

	// 2. Add the VLC Lib/Plugins directory to the DLL search path
	// This ensures libvlccore.dll and plugins are found by libvlc.dll
	FString VlcLibPath = FPaths::Combine(*PluginBaseDir, TEXT("ThirdParty/LibVLC/Lib"));
    
	FPlatformProcess::AddDllDirectory(*VlcLibPath);

	// 3. Explicitly load the main VLC DLL
	FString LibVlcPath = FPaths::Combine(*VlcLibPath, TEXT("libvlc.dll"));
	VlcDllHandle = !LibVlcPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibVlcPath) : nullptr;

	if (!VlcDllHandle)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("VlcError", "Failed to load libvlc.dll. Check ThirdParty folder!"));
	}
}

void FWebVideoStreamModule::ShutdownModule()
{
	// Clean up the DLL handle on shutdown
	if (VlcDllHandle)
	{
		FPlatformProcess::FreeDllHandle(VlcDllHandle);
		VlcDllHandle = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWebVideoStreamModule, WebVideoStream)