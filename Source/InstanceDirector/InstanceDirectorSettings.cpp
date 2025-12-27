#include "InstanceDirectorSettings.h"

UInstanceDirectorSettings::UInstanceDirectorSettings()
{
	bEnableSingleInstanceCheck = true;
	PortNumber = 64321;
	
	URIScheme = TEXT("");
	URISchemeFriendlyName = TEXT("Instance Director Application");
	bRegisterURISchemeOnStartup = false;
}
