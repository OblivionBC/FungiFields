#pragma once

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	NotStarted,
	InProgress,
	ReadyToCollect,
	Completed,
	Failed
};
