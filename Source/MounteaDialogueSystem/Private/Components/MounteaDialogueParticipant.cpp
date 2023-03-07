// All rights reserved Dominik Pavlicek 2023


#include "Components/MounteaDialogueParticipant.h"

#include "Components/AudioComponent.h"
#include "Graph/MounteaDialogueGraph.h"
#include "Helpers/MounteaDialogueSystemBFC.h"
#include "Nodes/MounteaDialogueGraphNode.h"

UMounteaDialogueParticipant::UMounteaDialogueParticipant()
{
	DefaultParticipantState = EDialogueParticipantState::EDPS_Enabled;
}

void UMounteaDialogueParticipant::BeginPlay()
{
	Super::BeginPlay();

	OnDialogueGraphChanged.AddUniqueDynamic(this, &UMounteaDialogueParticipant::OnDialogueGraphChangedEvent);

	SetParticipantState(GetDefaultParticipantState());

	SetAudioComponent(FindAudioComponent());

	InitializeParticipant_Implementation();
}


void UMounteaDialogueParticipant::InitializeParticipant_Implementation()
{
	if (DialogueGraph == nullptr) return;

	TArray<FMounteaDialogueDecorator> Decorators = UMounteaDialogueSystemBFC::GetAllDialogueDecorators(DialogueGraph);
	for (const auto Itr : Decorators)
	{
		if (Itr.DecoratorType)
		{
			Itr.DecoratorType->InitializeDecorator(GetWorld());
		}
	}
}

UAudioComponent* UMounteaDialogueParticipant::FindAudioComponent() const
{
	if (AudioComponent != nullptr) return nullptr;

	if (AudioComponentIdentification.IsNone()) return nullptr;

	if (AudioComponentIdentification.IsValid() == false) return nullptr;
	
	if (const auto Return = FindAudioComponentByName(AudioComponentIdentification))
	{
		return Return;
	}

	if (const auto Return = FindAudioComponentByTag(AudioComponentIdentification))
	{
		return Return;
	}

	LOG_WARNING(TEXT("[FindAudioComponent] No Audio Component found with by %s"), *AudioComponentIdentification.ToString())
	
	return nullptr;
}

UAudioComponent* UMounteaDialogueParticipant::FindAudioComponentByName(const FName& Arg) const
{
	if (GetOwner() == nullptr) return nullptr;

	return UMounteaDialogueSystemBFC::FindAudioComponentByName(GetOwner(), Arg);
}

UAudioComponent* UMounteaDialogueParticipant::FindAudioComponentByTag(const FName& Arg) const
{
	if (GetOwner() == nullptr) return nullptr;

	return UMounteaDialogueSystemBFC::FindAudioComponentByTag(GetOwner(), Arg);
}

void UMounteaDialogueParticipant::PlayParticipantVoice(USoundBase* ParticipantVoice)
{
	if (AudioComponent)
	{
		AudioComponent->SetSound(ParticipantVoice);
		AudioComponent->Play();
	}
}

void UMounteaDialogueParticipant::SkipParticipantVoice(USoundBase* ParticipantVoice)
{
	if (AudioComponent)
	{
		AudioComponent->SetSound(nullptr);
		AudioComponent->StopDelayed(UMounteaDialogueSystemBFC::GetDialogueSystemSettings_Internal()->GetSkipFadeDuration());
	}
}

bool UMounteaDialogueParticipant::CanStartDialogue() const
{
	return
	Execute_CanStartDialogueEvent(this) &&
	DialogueGraph != nullptr &&
	DialogueGraph->CanStartDialogueGraph() &&
	DialogueGraph->GetStartNode() != nullptr &&
	DialogueGraph->GetStartNode()->GetChildrenNodes().Num() > 0;
}

void UMounteaDialogueParticipant::SetDialogueGraph(UMounteaDialogueGraph* NewDialogueGraph)
{
	if (NewDialogueGraph != DialogueGraph)
	{
		DialogueGraph = NewDialogueGraph;

		OnDialogueGraphChanged.Broadcast(NewDialogueGraph);
	}
}

void UMounteaDialogueParticipant::SetParticipantState(const EDialogueParticipantState NewState)
{
	ParticipantState = NewState;

	OnDialogueParticipantStateChanged.Broadcast(NewState);
}

void UMounteaDialogueParticipant::SetDefaultParticipantState(const EDialogueParticipantState NewState)
{
	DefaultParticipantState = NewState;
}

void UMounteaDialogueParticipant::SetAudioComponent(UAudioComponent* NewAudioComponent)
{
	AudioComponent = NewAudioComponent;

	OnAudioComponentChanged.Broadcast(AudioComponent);
}
