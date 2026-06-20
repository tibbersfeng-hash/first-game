// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueData.generated.h"

/**
 * 对话选项
 */
USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()

	/** 选项文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText ChoiceText;

	/** 选择后跳转的对话 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName NextDialogueID;

	/** 需要的条件（为空则无条件） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString RequiredCondition;
};

/**
 * 对话节点数据
 */
UCLASS(BlueprintType, Blueprintable)
class FIRSTGAME_API UDialogueNodeData : public UDataAsset
{
	GENERATED_BODY()

public:
	UDialogueNodeData();

	/** 对话 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName DialogueID;

	/** 说话者名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText SpeakerName;

	/** 对话内容 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText DialogueText;

	/** 说话者头像 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* SpeakerPortrait;

	/** 对话选项（分支对话） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choices")
	TArray<FDialogueChoice> Choices;

	/** 下一个对话 ID（线性对话） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName NextDialogueID;

	/** 是否为结束节点 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	bool bIsEnd = false;
};
