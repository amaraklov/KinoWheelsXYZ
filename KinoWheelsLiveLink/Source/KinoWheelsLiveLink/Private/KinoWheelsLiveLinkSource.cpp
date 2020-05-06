// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "KinoWheelsLiveLinkSource.h"

#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"

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
	//DeviceEndpoint = InEndpoint; // Override default behavior
	DeviceEndpoint = InEndpoint;
	SelfEndpoint = FIPv4Endpoint::Any;

	SourceStatus = LOCTEXT("SourceStatus_DeviceNotFound", "Device Not Found");
	SourceType = LOCTEXT("KinoWheelsLiveLinkSourceType", "KinoWheels");	
	SourceMachineName = FText::FromString(DeviceEndpoint.ToString());

	//setup socket
	if (SelfEndpoint.Address.IsMulticastAddress())
	{
		Socket = FUdpSocketBuilder(TEXT("KINOWHEELSSOCKET_SELF"))
			.AsNonBlocking()
			.AsReusable()
			.BoundToPort(SelfEndpoint.Port)
			.WithReceiveBufferSize(RECV_BUFFER_SIZE)
			.BoundToAddress(FIPv4Address::Any)
			.JoinedToGroup(SelfEndpoint.Address)
			.WithMulticastLoopback()
			.WithMulticastTtl(2);
	}
	else
	{
		Socket = FUdpSocketBuilder(TEXT("KINOWHEELSSOCKET_SELF"))
			.AsNonBlocking()
			.AsReusable()
			.BoundToAddress(SelfEndpoint.Address)
			.BoundToPort(SelfEndpoint.Port)
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
		SendData("{\"SendData\": false,\"ResetData\": false}");
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
	SendData("{\"SendData\": false,\"ResetData\": false}");
	return true;
}
// FRunnable interface

void FKinoWheelsLiveLinkSource::SendData(FString Message) {
	TSharedRef<FInternetAddr> Addr = DeviceEndpoint.ToInternetAddr();
	int32 DataSize;
	Socket->SendTo((uint8*)TCHAR_TO_UTF8(Message.GetCharArray().GetData()), Message.GetCharArray().Num(), DataSize, *Addr);
	// Done Sending Wakup Data
}

void FKinoWheelsLiveLinkSource::Start()
{
	// Send some UDP Data to the device to wake it up:
	SendData("{\"SendData\": true,\"ResetData\": false}");
	// Done Sending Wakup Data
	

	ThreadName = "KinoWheels UDP Receiver ";
	ThreadName.AppendInt(FAsyncThreadIndex::GetNext());
	
	Thread = FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FKinoWheelsLiveLinkSource::Stop()
{	
	Stopping = true;
}

void FKinoWheelsLiveLinkSource::Reset() {
	SendData("{\"SendData\": true,\"ResetData\": true}");
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

			//bool bCreateSubject = !EncounteredSubjects.Contains(SubjectName);
			
			if (bCreateSubject)
			{				
						
				FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct(FLiveLinkCameraStaticData::StaticStruct());
				FLiveLinkCameraStaticData& StaticData = *StaticDataStruct.Cast<FLiveLinkCameraStaticData>();
								
				//StaticData.PropertyNames.SetNumUninitialized(1);// Create an entry for it to pupulate
				//
				//FString CamObject;
				//if (CameraObject->TryGetStringField(TEXT("Name"), CamObject))
				//{					
				//	StaticData.PropertyNames[0] = FName(*CamObject);
				//}
				//else
				//{
				//	// Invalid Json Format
				//	UE_LOG(LogTemp, Error, TEXT("LiveLinkKinoWheels: Warning Invalid Static JSON Reference"));
				//	return;
				//}
				SubjectName = "KinoWheels"; // Hard coded, since somehow parsing the JSON object seems to return different objects or something that confuses live link
				Client->PushSubjectStaticData_AnyThread({ SourceGuid, SubjectName }, ULiveLinkCameraRole::StaticClass(), MoveTemp(StaticDataStruct));
				EncounteredSubjects.Add(SubjectName);
				bCreateSubject = false;
			}
			
			if (!Stopping) { // Prevent this from generating data if it's shutting down.

				FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct(FLiveLinkCameraFrameData::StaticStruct());
				FLiveLinkCameraFrameData& FrameData = *FrameDataStruct.Cast<FLiveLinkCameraFrameData>();

				const TArray<TSharedPtr<FJsonValue>>* RotationArray;
				const TArray<TSharedPtr<FJsonValue>>* TranslationArray;
				FQuat Rotation(FQuat::Identity);
				FVector RotationVector(FVector::ZeroVector);
				FVector Translation(FVector::ZeroVector);
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

				if (CameraObject->TryGetArrayField(TEXT("Rot"), TranslationArray)
					&& TranslationArray->Num() == 3) // X, Y, Z //->TryGetArrayField(TEXT("Rot"), RotationArray)
				{
					double X = (*TranslationArray)[0]->AsNumber();
					double Y = (*TranslationArray)[1]->AsNumber();
					double Z = (*TranslationArray)[2]->AsNumber();
					Translation = FVector(X, Y, Z);
				}
				else
				{
					// Invalid Json Format
					UE_LOG(LogTemp, Error, TEXT("LiveLinkKinoWheels: Warning Invalid Dynamic JSON Reference"));
					return;
				}


				FrameData.Transform = FTransform(Rotation, Translation, FVector::OneVector);
				FrameData.FocalLength = Translation.Z;
				Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName }, MoveTemp(FrameDataStruct));
			}
		}
		
	}
}

#undef LOCTEXT_NAMESPACE