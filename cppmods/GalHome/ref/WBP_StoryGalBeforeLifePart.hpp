#ifndef UE4SS_SDK_WBP_StoryGalBeforeLifePart_HPP
#define UE4SS_SDK_WBP_StoryGalBeforeLifePart_HPP

class UWBP_StoryGalBeforeLifePart_C : public UUserWidget
{
    FPointerToUberGraphFrame UberGraphFrame;                                          // 0x0280 (size: 0x8)
    class UWidgetAnimation* Anim_NameAppear;                                          // 0x0288 (size: 0x8)
    class UWidgetAnimation* Anim_Choice0;                                             // 0x0290 (size: 0x8)
    class UWidgetAnimation* Anim_Choice1;                                             // 0x0298 (size: 0x8)
    class UWidgetAnimation* Anim_Choice2;                                             // 0x02A0 (size: 0x8)
    class UButton* Button_Choice_0;                                                   // 0x02A8 (size: 0x8)
    class UButton* Button_Choice_1;                                                   // 0x02B0 (size: 0x8)
    class UButton* Button_Choice_2;                                                   // 0x02B8 (size: 0x8)
    class UButton* Button_GoNext;                                                     // 0x02C0 (size: 0x8)
    class UImage* ImageNameBackGround;                                                // 0x02C8 (size: 0x8)
    class UTextBlock* Text_Choice;                                                    // 0x02D0 (size: 0x8)
    class UTextBlock* Text_Choice_1;                                                  // 0x02D8 (size: 0x8)
    class UTextBlock* Text_Choice_2;                                                  // 0x02E0 (size: 0x8)
    class UTextBlock* Text_Talker;                                                    // 0x02E8 (size: 0x8)
    class UTextBlock* TextBlock_DialogueText;                                         // 0x02F0 (size: 0x8)
    class UDataTable* DT_StoryGal;                                                    // 0x02F8 (size: 0x8)
    FText PrevTalker;                                                                 // 0x0300 (size: 0x18)
    FText CurrentTalker;                                                              // 0x0318 (size: 0x18)
    FText Dialogue Text;                                                              // 0x0330 (size: 0x18)
    TArray<FString> Answer Choices;                                                   // 0x0348 (size: 0x10)
    bool Spawn Girl?;                                                                 // 0x0358 (size: 0x1)
    bool CompleteDisplayDialogueText?;                                                // 0x0359 (size: 0x1)
    FStr_StorysGalBeforeLifeRow Current Info Story Gal;                               // 0x0360 (size: 0x90)
    FString TalkerNameString;                                                         // 0x03F0 (size: 0x10)

    void CollapseChoiceButtons();
    void SetUp_ChoiceButton();
    void EnableButton_GoNext(bool Enable);
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_0_K2Node_ComponentBoundEvent_0_OnButtonHoverEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_0_K2Node_ComponentBoundEvent_1_OnButtonHoverEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_1_K2Node_ComponentBoundEvent_2_OnButtonHoverEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_1_K2Node_ComponentBoundEvent_3_OnButtonHoverEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_2_K2Node_ComponentBoundEvent_4_OnButtonHoverEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_2_K2Node_ComponentBoundEvent_5_OnButtonHoverEvent__DelegateSignature();
    void PreConstruct(bool IsDesignTime);
    void StartDialogue(FStr_StorysGalBeforeLifeRow CurrentInfo_StoryGal);
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_GoNext_K2Node_ComponentBoundEvent_0_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_0_K2Node_ComponentBoundEvent_1_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_1_K2Node_ComponentBoundEvent_2_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_StoryGalBeforeLifePart_Button_Choice_2_K2Node_ComponentBoundEvent_3_OnButtonClickedEvent__DelegateSignature();
    void ChangeDialogue(const FStr_StorysGalBeforeLifeRow& CurrentInfoStoryGirl);
    void RemoveDialogueWbp();
    void RequestPauseInput();
    void CompleteDisplayDialogue();
    void ExecuteUbergraph_WBP_StoryGalBeforeLifePart(int32 EntryPoint);
}; // Size: 0x400

#endif
