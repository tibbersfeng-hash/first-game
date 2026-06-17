// Copyright 2026 格斗萌主 Team. All Rights Reserved.
// Audio Manager — background music and sound effects management

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioManagerSubsystem.generated.h"

class USoundBase;
class USoundCue;

UENUM(BlueprintType)
enum class EAudioCategory : uint8
{
	BGM UMETA(DisplayName = "Background Music"),
	Combat UMETA(DisplayName = "Combat SE"),
	UI UMETA(DisplayName = "UI Sounds"),
	Environment UMETA(DisplayName = "Environment")
};

UCLASS()
class FIRSTGAME_API UAudioManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ─── BGM ─────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayBGM(USoundBase* Sound, float FadeInTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopBGM(float FadeOutTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetBGMVolume(float Volume);

	// ─── Sound Effects ───────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayCombatSound(USoundBase* Sound, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayUISound(USoundBase* Sound, float VolumeMultiplier = 1.0f);

	// ─── Volume Control ──────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCategoryVolume(EAudioCategory Category, float Volume);

	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetMasterVolume() const { return MasterVolume; }

	// ─── Predefined Combat Sounds ────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
	void PlayHitSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
	void PlayDodgeSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
	void PlayComboSound(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
	void PlayVictorySound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Combat")
	void PlayDefeatSound();

private:
	UPROPERTY()
	class UAudioComponent* BGMComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Volumes")
	float MasterVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Volumes")
	float BGMVolume = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Volumes")
	float CombatVolume = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Volumes")
	float UIVolume = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Sounds")
	USoundBase* DefaultHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Sounds")
	USoundBase* DefaultDodgeSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Sounds")
	USoundBase* DefaultComboSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Sounds")
	USoundBase* DefaultVictorySound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Sounds")
	USoundBase* DefaultDefeatSound;
};
