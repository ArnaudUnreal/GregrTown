// GregrTown - editor tools

#include "DestructibleReplacementLibrary.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "GregrTownEditor"

class FGregrTownEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGregrTownEditorModule::RegisterMenus));
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

private:
	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);

		// Tools > GregrTown
		UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& Section = ToolsMenu->AddSection("GregrTown", LOCTEXT("GregrTownSection", "GregrTown"));

		Section.AddMenuEntry("GregrReplaceLevel",
			LOCTEXT("ReplaceLevel", "Make Level Destructible"),
			LOCTEXT("ReplaceLevelTooltip", "Replaces every Static Mesh actor of the level that has a matching Geometry Collection by a destructible actor."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FGregrTownEditorModule::RunReplacement, false)));

		Section.AddMenuEntry("GregrReplaceSelection",
			LOCTEXT("ReplaceSelection", "Make Selection Destructible"),
			LOCTEXT("ReplaceSelectionTooltip", "Replaces the selected Static Mesh actors that have a matching Geometry Collection by destructible actors."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FGregrTownEditorModule::RunReplacement, true)));

		// Right click on actors in the viewport / outliner
		UToolMenu* ActorMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
		FToolMenuSection& ActorSection = ActorMenu->AddSection("GregrTown", LOCTEXT("GregrTownSection", "GregrTown"));
		ActorSection.AddMenuEntry("GregrReplaceSelectionContext",
			LOCTEXT("ReplaceSelection", "Make Selection Destructible"),
			LOCTEXT("ReplaceSelectionTooltip", "Replaces the selected Static Mesh actors that have a matching Geometry Collection by destructible actors."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FGregrTownEditorModule::RunReplacement, true)));
	}

	static void RunReplacement(bool bSelectedOnly)
	{
		const FDestructibleReplacementReport Preview = UDestructibleReplacementLibrary::ReplaceStaticMeshActors(bSelectedOnly, true);
		if (Preview.Replaced == 0)
		{
			Notify(FText::Format(LOCTEXT("NothingToReplace", "Nothing to replace ({0})."), FText::FromString(Preview.ToString())), false);
			return;
		}

		const FText Question = FText::Format(
			LOCTEXT("Confirm", "{0} Static Mesh actors will be replaced by destructibles.\n{1} actors have no Geometry Collection, {2} are excluded.\n\nThis can be undone with Ctrl+Z. Continue?"),
			Preview.Replaced, Preview.SkippedNoGeometryCollection, Preview.SkippedExcluded);
		if (FMessageDialog::Open(EAppMsgType::YesNo, Question) != EAppReturnType::Yes)
		{
			return;
		}

		const FDestructibleReplacementReport Report = UDestructibleReplacementLibrary::ReplaceStaticMeshActors(bSelectedOnly, false);
		Notify(FText::Format(LOCTEXT("Done", "Destructibles: {0}. Details in the Output Log."), FText::FromString(Report.ToString())), Report.Replaced > 0);
	}

	static void Notify(const FText& Message, bool bSuccess)
	{
		FNotificationInfo Info(Message);
		Info.ExpireDuration = 5.f;
		TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info);
		if (Item.IsValid())
		{
			Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_None);
		}
	}
};

IMPLEMENT_MODULE(FGregrTownEditorModule, GregrTownEditor)

#undef LOCTEXT_NAMESPACE
