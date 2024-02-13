// Copyright Dean Wilson


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(
	AActor* TargetActor,
	const FEffectType& Effect
)
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
		TargetActor
	);

	if (TargetAbilitySystemComponent == nullptr) return;

	check(Effect.GameplayEffect);
	FGameplayEffectContextHandle EffectContextHandle = TargetAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(
		Effect.GameplayEffect,
		ActorLevel,
		EffectContextHandle
	);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
		*EffectSpecHandle.Data.Get()
	);

	if (EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite)
	{
		if (Effect.RemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
		{
			ActiveEffectHandles.Add(ActiveEffectHandle, TargetAbilitySystemComponent);
		}
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	for (const FEffectType& Effect : Effects)
	{
		if (Effect.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, Effect);
		}
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	for (const FEffectType& Effect : Effects)
	{
		if (Effect.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, Effect);
		}

		if (Effect.GameplayEffect.GetDefaultObject()->DurationPolicy == EGameplayEffectDurationType::Infinite)
		{
			if (Effect.RemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
			{
				UAbilitySystemComponent* TargetAbilitySystemComponent =
					UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
						TargetActor
					);
				if (!IsValid(TargetAbilitySystemComponent)) return;
				TArray<FActiveGameplayEffectHandle> HandlesToRemove;
				for (TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*> HandlePair : ActiveEffectHandles)
				{
					if (TargetAbilitySystemComponent == HandlePair.Value)
					{
						TargetAbilitySystemComponent->RemoveActiveGameplayEffect(HandlePair.Key, 1);
						HandlesToRemove.Add(HandlePair.Key);
					}
				}

				for (FActiveGameplayEffectHandle& Handle : HandlesToRemove)
				{
					ActiveEffectHandles.FindAndRemoveChecked(Handle);
				}
			}
		}
	}
}
