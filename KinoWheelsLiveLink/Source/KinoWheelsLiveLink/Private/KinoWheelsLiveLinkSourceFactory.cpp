// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#include "KinoWheelsLiveLinkSourceFactory.h"
#include "KinoWheelsLiveLinkSource.h"
#include "SKinoWheelsLiveLinkSourceFactory.h"

#define LOCTEXT_NAMESPACE "KinoWheelsLiveLinkSourceFactory"

FText UKinoWheelsLiveLinkSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("SourceDisplayName", "KinoWheels LiveLink");
}

FText UKinoWheelsLiveLinkSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("SourceTooltip", "Creates a connection to a KinoWheels UDP Stream");
}

TSharedPtr<SWidget> UKinoWheelsLiveLinkSourceFactory::BuildCreationPanel(FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	return SNew(SKinoWheelsLiveLinkSourceFactory)
		.OnOkClicked(SKinoWheelsLiveLinkSourceFactory::FOnOkClicked::CreateUObject(this, &UKinoWheelsLiveLinkSourceFactory::OnOkClicked, InOnLiveLinkSourceCreated));
}

TSharedPtr<ILiveLinkSource> UKinoWheelsLiveLinkSourceFactory::CreateSource(const FString& InConnectionString) const
{
	FIPv4Endpoint DeviceEndPoint;
	if (!FIPv4Endpoint::Parse(InConnectionString, DeviceEndPoint))
	{
		return TSharedPtr<ILiveLinkSource>();
	}

	return MakeShared<FKinoWheelsLiveLinkSource>(DeviceEndPoint);
}

void UKinoWheelsLiveLinkSourceFactory::OnOkClicked(FIPv4Endpoint InEndpoint, FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	InOnLiveLinkSourceCreated.ExecuteIfBound(MakeShared<FKinoWheelsLiveLinkSource>(InEndpoint), InEndpoint.ToString());
}

#undef LOCTEXT_NAMESPACE