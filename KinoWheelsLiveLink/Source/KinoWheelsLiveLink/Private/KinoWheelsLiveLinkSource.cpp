// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "KinoWheelsLiveLinkSource.h"

#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"
//#include "Roles/LiveLinkAnimationRole.h"
//#include "Roles/LiveLinkAnimationTypes.h"

#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"

#include "Async/Async.h"
#include "Common/UdpSocketBuilder.h"
#include "HAL/RunnableThread.h"
#include "Json.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

#include "Math/UnrealMathUtility.h"

#define LOCTEXT_NAMESPACE "KinoWheelsLiveLinkSource"

#define RECV_BUFFER_SIZE 1024 * 1024

FKinoWheelsLiveLinkSource::FKinoWheelsLiveLinkSource(FIPv4Endpoint InEndpoint)
: Socket(nullptr)
, Stopping(false)
, Thread(nullptr)
, WaitTime(FTimespan::FromMilliseconds(100))
{
	// defaults
	DeviceEndpoint = InEndpoint;

	SourceStatus = LOCTEXT("SourceStatus_DeviceNotFound", "Device Not Found");
	SourceType = LOCTEXT("KinoWheelsLiveLinkSourceType", "KinoWheels LiveLink");
	SourceMachineName = LOCTEXT("KinoWheelsLiveLinkSourceMachineName", "localhost");

	//setup socket
	if (DeviceEndpoint.Address.IsMulticastAddress())
	{
		Socket = FUdpSocketBuilder(TEXT("KINOWHEELSSOCKET"))
			.AsNonBlocking()
			.AsReusable()
			.BoundToPort(DeviceEndpoint.Port)
			.WithReceiveBufferSize(RECV_BUFFER_SIZE)
			.BoundToAddress(FIPv4Address::Any)
			.JoinedToGroup(DeviceEndpoint.Address)
			.WithMulticastLoopback()
			.WithMulticastTtl(2);
	}
	else
	{
		Socket = FUdpSocketBuilder(TEXT("KINOWHEELSSOCKET"))
			.AsNonBlocking()
			.AsReusable()
			.BoundToAddress(DeviceEndpoint.Address)
			.BoundToPort(DeviceEndpoint.Port)
			.WithReceiveBufferSize(RECV_BUFFER_SIZE);
	}

	RecvBuffer.SetNumUninitialized(RECV_BUFFER_SIZE);

	if ((Socket != nullptr) && (Socket->GetSocketType() == SOCKTYPE_Datagram))
	{
		SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		Start();

		SourceStatus = LOCTEXT("SourceStatus_Receiving", "Receiving");
	}

}

FKinoWheelsLiveLinkSource::~FKinoWheelsLiveLinkSource()
{
	Stop();
	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
	if (Socket != nullptr)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
}

void FKinoWheelsLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}


bool FKinoWheelsLiveLinkSource::IsSourceStillValid() const
{
	// Source is valid if we have a valid thread and socket
	bool bIsSourceValid = !Stopping && Thread != nullptr && Socket != nullptr;
	return bIsSourceValid;
}


bool FKinoWheelsLiveLinkSource::RequestSourceShutdown()
{
	Stop();

	return true;
}
// FRunnable interface

void FKinoWheelsLiveLinkSource::Start()
{
	// Send some UDP Data to the device to wake it up:
	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool bIsValid = false;
	Addr->SetIp(TEXT("192.168.0.88"), bIsValid); // Destination IP
	Addr->SetPort(8887); // Destination port

	FString Message = "{\"SendData\": true,\"ResetData\": false}";
	int32 DataSize;

	Socket->SendTo((uint8*)TCHAR_TO_UTF8(Message.GetCharArray().GetData()), Message.GetCharArray().Num(), DataSize, *Addr);
	// End Send intitial data

	ThreadName = "KinoWheels UDP Receiver ";
	ThreadName.AppendInt(FAsyncThreadIndex::GetNext());
	
	Thread = FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FKinoWheelsLiveLinkSource::Stop()
{
	Stopping = true;
}

uint32 FKinoWheelsLiveLinkSource::Run()
{
	TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
	
	while (!Stopping)
	{
		if (Socket->Wait(ESocketWaitConditions::WaitForRead, WaitTime))
		{
			uint32 Size;

			while (Socket->HasPendingData(Size))
			{
				int32 Read = 0;

				if (Socket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), Read, *Sender))
				{
					if (Read > 0)
					{
						TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
						ReceivedData->SetNumUninitialized(Read);
						memcpy(ReceivedData->GetData(), RecvBuffer.GetData(), Read);
						AsyncTask(ENamedThreads::GameThread, [this, ReceivedData]() { HandleReceivedCameraData(ReceivedData); });
					}
				}
			}
		}
	}
	return 0;
}


void FKinoWheelsLiveLinkSource::HandleReceivedCameraData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FString JsonString;
	JsonString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		JsonString += TCHAR(Byte);
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		for (TPair<FString, TSharedPtr<FJsonValue>>& JsonField : JsonObject->Values)
		{	

			FName SubjectName(*JsonField.Key);
			
			const TSharedPtr<FJsonObject> CameraObject = JsonField.Value->AsObject();			

			bool bCreateSubject = !EncounteredSubjects.Contains(SubjectName);
			if (bCreateSubject)
			{				
						
				FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct(FLiveLinkCameraStaticData::StaticStruct());
				FLiveLinkCameraStaticData& StaticData = *StaticDataStruct.Cast<FLiveLinkCameraStaticData>();
								
				StaticData.PropertyNames.SetNumUninitialized(1);// Create an entry for it to pupulate
				
				FString CamObject;
				if (CameraObject->TryGetStringField(TEXT("Name"), CamObject))
				{					
					StaticData.PropertyNames[0] = FName(*CamObject);
				}
				else
				{
					// Invalid Json Format
					UE_LOG(LogTemp, Error, TEXT("LiveLinkKinoWheels: Warning Invalid Static JSON Reference"));
					return;
				}
				Client->PushSubjectStaticData_AnyThread({ SourceGuid, SubjectName }, ULiveLinkCameraRole::StaticClass(), MoveTemp(StaticDataStruct));
				EncounteredSubjects.Add(SubjectName);
			}
			
			FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct(FLiveLinkCameraFrameData::StaticStruct());
			FLiveLinkCameraFrameData& FrameData = *FrameDataStruct.Cast<FLiveLinkCameraFrameData>();
			

			const TArray<TSharedPtr<FJsonValue>>* RotationArray;
			FQuat Rotation;
			FVector RotationVector;
			//if (BoneObject->TryGetArrayField(TEXT("Location"), LocationArray)
			if (CameraObject->TryGetArrayField(TEXT("Rot"), RotationArray)
				&& RotationArray->Num() == 3) // X, Y, Z //->TryGetArrayField(TEXT("Rot"), RotationArray)
			{
				double X = (*RotationArray)[0]->AsNumber();
				double Y = (*RotationArray)[1]->AsNumber();
				double Z = (*RotationArray)[2]->AsNumber();
				RotationVector = FVector(X, Y, Z);
				Rotation = FQuat::MakeFromEuler(RotationVector);
			}
			else
			{
				// Invalid Json Format
				UE_LOG(LogTemp, Error, TEXT("LiveLinkKinoWheels: Warning Invalid Dynamic JSON Reference"));
				return;
			}			
			
			FVector Translation = FVector(FMath::RandRange(0,100), FMath::RandRange(0, 100), FMath::RandRange(0, 100));
			//Rotation = FQuat::MakeFromEuler(FVector(FMath::FRand()*360, FMath::FRand()*360, FMath::FRand()*360)); //Test To send / overwrite with random data
			FQuat R = FQuat::MakeFromEuler(FVector(FMath::RandRange(0, 100), FMath::RandRange(0, 100), FMath::RandRange(0, 100)));
			FrameData.Transform = FTransform(R, Translation, FVector::OneVector);			
			Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName }, MoveTemp(FrameDataStruct));			
		}
		
	}
}

#undef LOCTEXT_NAMESPACE
