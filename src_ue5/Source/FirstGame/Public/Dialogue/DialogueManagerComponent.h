// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/DialogueData.h"
#include "DialogueManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, UDialogueNodeData*, CurrentNode, int32, TotalNodes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, bool, bCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChoiceSelected, int32, ChoiceIndex);

/**
 * 剧情对话管理器组件
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FIRSTGAME_API UDialogueManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueManagerComponent();

	virtual void BeginPlay() override;

	/** 开始对话 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(TArray<UDialogueNodeData*> DialogueNodes);

	/** 显示当前对话 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	UDialogueNodeData* GetCurrentDialogue() const;

	/** 选择选项 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(int32 ChoiceIndex);

	/** 继续下一个对话 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void NextDialogue();

	/** 结束对话 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	/** 是否正在对话 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool IsInDialogue() const;

	/** 对话开始事件 */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueStarted OnDialogueStarted;

	/** 对话结束事件 */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

	/** 选项选择事件 */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnChoiceSelected OnChoiceSelected;

protected:
	/** 对话节点列表 */
	UPROPERTY()
	TArray<UDialogueNodeData*> DialogueNodes;

	/** 当前对话索引 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	int32 CurrentIndex = 0;

	/** 是否正在对话 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	bool bIsInDialogue = false;

private:
	/** 获取当前对话节点 */
	UDialogueNodeData* GetCurrentNode() const;
};
