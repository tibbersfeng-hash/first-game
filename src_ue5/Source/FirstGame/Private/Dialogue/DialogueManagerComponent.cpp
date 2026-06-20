// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "Dialogue/DialogueManagerComponent.h"
#include "FirstGame.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void UDialogueManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initialized"));
}

void UDialogueManagerComponent::StartDialogue(TArray<UDialogueNodeData*> Nodes)
{
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager: No dialogue nodes provided"));
		return;
	}

	DialogueNodes = Nodes;
	CurrentIndex = 0;
	bIsInDialogue = true;

	UDialogueNodeData* FirstNode = GetCurrentNode();
	if (FirstNode)
	{
		OnDialogueStarted.Broadcast(FirstNode, DialogueNodes.Num());
		UE_LOG(LogTemp, Log, TEXT("DialogueManager: Started dialogue with %d nodes"), DialogueNodes.Num());
	}
}

UDialogueNodeData* UDialogueManagerComponent::GetCurrentDialogue() const
{
	return GetCurrentNode();
}

void UDialogueManagerComponent::SelectChoice(int32 ChoiceIndex)
{
	UDialogueNodeData* CurrentNode = GetCurrentNode();
	if (!CurrentNode)
	{
		return;
	}

	OnChoiceSelected.Broadcast(ChoiceIndex);

	if (ChoiceIndex >= 0 && ChoiceIndex < CurrentNode->Choices.Num())
	{
		FName NextID = CurrentNode->Choices[ChoiceIndex].NextDialogueID;

		// 查找下一个对话节点
		for (int32 i = 0; i < DialogueNodes.Num(); i++)
		{
			if (DialogueNodes[i]->DialogueID == NextID)
			{
				CurrentIndex = i;
				UDialogueNodeData* NextNode = GetCurrentNode();
				if (NextNode)
				{
					OnDialogueStarted.Broadcast(NextNode, DialogueNodes.Num());
				}
				return;
			}
		}
	}

	// 如果没有找到，继续下一个
	NextDialogue();
}

void UDialogueManagerComponent::NextDialogue()
{
	UDialogueNodeData* CurrentNode = GetCurrentNode();
	if (!CurrentNode)
	{
		return;
	}

	// 检查是否有选项
	if (CurrentNode->Choices.Num() > 0)
	{
		// 等待玩家选择
		return;
	}

	// 检查是否是结束节点
	if (CurrentNode->bIsEnd)
	{
		EndDialogue();
		return;
	}

	// 继续下一个对话
	CurrentIndex++;

	if (CurrentIndex >= DialogueNodes.Num())
	{
		EndDialogue();
		return;
	}

	UDialogueNodeData* NextNode = GetCurrentNode();
	if (NextNode)
	{
		OnDialogueStarted.Broadcast(NextNode, DialogueNodes.Num());
	}
}

void UDialogueManagerComponent::EndDialogue()
{
	bIsInDialogue = false;
	CurrentIndex = 0;
	DialogueNodes.Empty();

	OnDialogueEnded.Broadcast(true);

	UE_LOG(LogTemp, Log, TEXT("DialogueManager: Ended dialogue"));
}

bool UDialogueManagerComponent::IsInDialogue() const
{
	return bIsInDialogue;
}

UDialogueNodeData* UDialogueManagerComponent::GetCurrentNode() const
{
	if (CurrentIndex >= 0 && CurrentIndex < DialogueNodes.Num())
	{
		return DialogueNodes[CurrentIndex];
	}
	return nullptr;
}
