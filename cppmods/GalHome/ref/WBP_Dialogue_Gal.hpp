#ifndef UE4SS_SDK_WBP_Dialogue_Gal_HPP
#define UE4SS_SDK_WBP_Dialogue_Gal_HPP

class UWBP_Dialogue_Gal_C : public UUserWidget
{
    FPointerToUberGraphFrame UberGraphFrame;                                          // 0x0280 (size: 0x8)
    class UButton* Button_Choice_0;                                                   // 0x0288 (size: 0x8)
    class UButton* Button_Choice_1;                                                   // 0x0290 (size: 0x8)
    class UButton* Button_Choice_2;                                                   // 0x0298 (size: 0x8)
    class UButton* Button_GoNext;                                                     // 0x02A0 (size: 0x8)
    class UImage* Image_TalkerBack;                                                   // 0x02A8 (size: 0x8)
    class UTextBlock* Text_Choice;                                                    // 0x02B0 (size: 0x8)
    class UTextBlock* Text_Choice_1;                                                  // 0x02B8 (size: 0x8)
    class UTextBlock* Text_Choice_2;                                                  // 0x02C0 (size: 0x8)
    class UTextBlock* Text_Talker;                                                    // 0x02C8 (size: 0x8)
    class UTextBlock* TextBlock_DialogueText;                                         // 0x02D0 (size: 0x8)
    class UDataTable* DT_StoryGal;                                                    // 0x02D8 (size: 0x8)
    FText Talker;                                                                     // 0x02E0 (size: 0x18)
    FText Dialogue Text;                                                              // 0x02F8 (size: 0x18)
    TArray<FString> Answer Choices;                                                   // 0x0310 (size: 0x10)
    FString Spawn Girl?;                                                              // 0x0320 (size: 0x10)
    FStr_DialoguePartRows Current Info Story Gal;                                     // 0x0330 (size: 0x88)

    void SetDialogueText(FText Text);
    void SetAnswerChoices(TArray<FString>& Choices);
    void SetTalker(FText TalkerName);
    void EnableGoNextButton(bool Enable);
    void PreConstruct(bool IsDesignTime);
    void StartDialogue(FStr_DialoguePartRows CurrentInfo_DialogueGal);
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_GoNext_K2Node_ComponentBoundEvent_0_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_0_K2Node_ComponentBoundEvent_1_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_1_K2Node_ComponentBoundEvent_2_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_2_K2Node_ComponentBoundEvent_3_OnButtonClickedEvent__DelegateSignature();
    void ChangeDialogue(const FStr_DialoguePartRows& CurrentInfoDialogueGal);
    void RemoveDialogueWbp();
    void RequestPauseInput();
    void StartFadeOutDuringDialogue();
    void ExecuteUbergraph_WBP_Dialogue_Gal(int32 EntryPoint);
}; // Size: 0x3B8

#endif
