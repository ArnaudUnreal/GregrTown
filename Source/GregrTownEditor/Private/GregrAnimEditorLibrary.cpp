// GregrTown - editor tools

#include "GregrAnimEditorLibrary.h"

#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"

bool UGregrAnimEditorLibrary::SetBlendSpaceSamples(UBlendSpace* BlendSpace, const TArray<UAnimSequence*>& Animations, const TArray<FVector>& SampleValues)
{
	if (!BlendSpace || Animations.Num() != SampleValues.Num())
	{
		return false;
	}

	BlendSpace->Modify();
	for (int32 Index = BlendSpace->GetNumberOfBlendSamples() - 1; Index >= 0; --Index)
	{
		BlendSpace->DeleteSample(Index);
	}

	for (int32 Index = 0; Index < Animations.Num(); ++Index)
	{
		if (Animations[Index])
		{
			BlendSpace->AddSample(Animations[Index], SampleValues[Index]);
		}
	}

	BlendSpace->ValidateSampleData();
	BlendSpace->ResampleData();
	BlendSpace->MarkPackageDirty();
	return BlendSpace->GetNumberOfBlendSamples() == Animations.Num();
}
