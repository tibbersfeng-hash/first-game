// Copyright 2026 格斗萌主 Team. All Rights Reserved.

#include "FirstGame.h"
#include "Subsystems/AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("AudioManagerSubsystem initialized"));
}

void UAudioManagerSubsystem::PlayBGM(USoundBase* Sound, float FadeInTime)
{
	if (!Sound) return;

	if (!BGMComponent)
	{
		BGMComponent = UGameplayStatics::SpawnSound2D(this, Sound, BGMVolume * MasterVolume, 1.f, 0.f, nullptr, true);
	}
	else
	{
		BGMComponent->SetSound(Sound);
		BGMComponent->SetVolumeMultiplier(BGMVolume * MasterVolume);
		BGMComponent->Play();
	}

	UE_LOG(LogTemp, Log, TEXT("BGM started: %s (fade=%.1f)"), *Sound->GetName(), FadeInTime);
}

void UAudioManagerSubsystem::StopBGM(float FadeOutTime)
{
	if (BGMComponent)
	{
		BGMComponent->FadeOut(FadeOutTime, 0.f);
	}
}

void UAudioManagerSubsystem::SetBGMVolume(float Volume)
{
	BGMVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (BGMComponent)
	{
		BGMComponent->SetVolumeMultiplier(BGMVolume * MasterVolume);
	}
}

void UAudioManagerSubsystem::PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, float VolumeMultiplier)
{
	if (!Sound) return;
	UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, VolumeMultiplier * MasterVolume);
}

void UAudioManagerSubsystem::PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachToComponent, float VolumeMultiplier)
{
	if (!Sound || !AttachToComponent) return;
	UGameplayStatics::PlaySoundAtLocation(this, Sound, AttachToComponent->GetComponentLocation(),
		VolumeMultiplier * MasterVolume);
}

void UAudioManagerSubsystem::PlayCombatSound(USoundBase* Sound, float VolumeMultiplier)
{
	PlaySoundAtLocation(Sound, FVector::ZeroVector, VolumeMultiplier * CombatVolume);
}

void UAudioManagerSubsystem::PlayUISound(USoundBase* Sound, float VolumeMultiplier)
{
	if (!Sound) return;
	UGameplayStatics::PlaySound2D(this, Sound, VolumeMultiplier * UIVolume * MasterVolume);
}

void UAudioManagerSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);
	UE_LOG(LogTemp, Log, TEXT("Master volume: %.2f"), MasterVolume);
}

void UAudioManagerSubsystem::SetCategoryVolume(EAudioCategory Category, float Volume)
{
	switch (Category)
	{
	case EAudioCategory::BGM:         SetBGMVolume(Volume); break;
	case EAudioCategory::Combat:      CombatVolume = FMath::Clamp(Volume, 0.f, 1.f); break;
	case EAudioCategory::UI:          UIVolume = FMath::Clamp(Volume, 0.f, 1.f); break;
	case EAudioCategory::Environment: break;
	}
}

// ─── Predefined Combat Sounds ────────────────────────────────────────

void UAudioManagerSubsystem::PlayHitSound()
{
	if (DefaultHitSound) PlayCombatSound(DefaultHitSound);
}

void UAudioManagerSubsystem::PlayDodgeSound()
{
	if (DefaultDodgeSound) PlayCombatSound(DefaultDodgeSound);
}

void UAudioManagerSubsystem::PlayComboSound(int32 Count)
{
	if (DefaultComboSound && Count >= 3)
	{
		PlayCombatSound(DefaultComboSound, 1.f + FMath::Min(Count, 20) * 0.05f);
	}
}

void UAudioManagerSubsystem::PlayVictorySound()
{
	if (DefaultVictorySound) PlaySoundAtLocation(DefaultVictorySound, FVector::ZeroVector);
}

void UAudioManagerSubsystem::PlayDefeatSound()
{
	if (DefaultDefeatSound) PlaySoundAtLocation(DefaultDefeatSound, FVector::ZeroVector);
}
