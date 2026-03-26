// Copyright ood11611doo. All right reserved.


#pragma once

#include "Modules/ModuleManager.h"

class FWebVideoStreamModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Handle to the vlc dll we will load */
	void* VlcDllHandle;
};