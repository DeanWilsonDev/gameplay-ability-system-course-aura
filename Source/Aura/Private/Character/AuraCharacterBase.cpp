// Copyright Dean Wilson


#include "Character/AuraCharacterBase.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

