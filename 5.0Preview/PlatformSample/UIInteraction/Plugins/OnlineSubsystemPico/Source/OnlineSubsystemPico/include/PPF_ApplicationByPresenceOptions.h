#ifndef MATRIX_PPF_APPLICATIONBYPRESENCEOPTIONS_H
#define MATRIX_PPF_APPLICATIONBYPRESENCEOPTIONS_H

/// @file PPF_ApplicationByPresenceOptions.h
/// @ingroup Social

typedef struct ppfApplicationByPresenceOptions* ppfApplicationByPresenceOptionsHandle;

PPF_PUBLIC_FUNCTION(ppfApplicationByPresenceOptionsHandle) ppf_ApplicationByPresenceOptions_Create();
PPF_PUBLIC_FUNCTION(void) ppf_ApplicationByPresenceOptions_Destroy(ppfApplicationByPresenceOptionsHandle handle);

PPF_PUBLIC_FUNCTION(void) ppf_ApplicationByPresenceOptions_SetDeeplinkMessage(ppfApplicationByPresenceOptionsHandle handle, const char * value);
PPF_PUBLIC_FUNCTION(void) ppf_ApplicationByPresenceOptions_SetDestinationApiName(ppfApplicationByPresenceOptionsHandle handle, const char * value);
PPF_PUBLIC_FUNCTION(void) ppf_ApplicationByPresenceOptions_SetLobbySessionId(ppfApplicationByPresenceOptionsHandle handle, const char * value);
PPF_PUBLIC_FUNCTION(void) ppf_ApplicationByPresenceOptions_SetMatchSessionId(ppfApplicationByPresenceOptionsHandle handle, const char * value);
PPF_PUBLIC_FUNCTION(void) ppf_ApplicationByPresenceOptions_SetTrackingId(ppfApplicationByPresenceOptionsHandle handle, const char * value);
PPF_PUBLIC_FUNCTION(void) ppf_ApplicationByPresenceOptions_SetExtra(ppfApplicationByPresenceOptionsHandle handle, const char * value);


#endif //MATRIX_PPF_APPLICATIONBYPRESENCEOPTIONS_H
