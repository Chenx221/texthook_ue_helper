#ifndef UE4SS_SDK_WBP_Dialogue_Inaka_HPP
#define UE4SS_SDK_WBP_Dialogue_Inaka_HPP

class UWBP_Dialogue_Inaka_C : public USubtitleWidget
{
    FPointerToUberGraphFrame UberGraphFrame;                                          // 0x0350 (size: 0x8)
    class UTextBlock* TextBlock_Subtitle;                                             // 0x0358 (size: 0x8)
    class UTextBlock* Text_Name;                                                      // 0x0360 (size: 0x8)
    class UImage* Image_SubtitleBack;                                                 // 0x0368 (size: 0x8)
    class UButton* Button_Next;                                                       // 0x0370 (size: 0x8)
    class UWidgetAnimation* Anim_StartDialogue;                                       // 0x0378 (size: 0x8)
    int32 TextCharacterCount;                                                         // 0x0380 (size: 0x4)
    FTimerHandle TimerHandle_SetSubtitle;                                             // 0x0388 (size: 0x8)

    void PreConstruct(bool IsDesignTime);
    void Construct();
    void BndEvt__WBP_Dialogue_Inaka_Button_Next_K2Node_ComponentBoundEvent_0_OnButtonClickedEvent__DelegateSignature();
    void Destruct();
    void I_UpdateDialogueWidget(FText SpeakerName, FText DialogueText);
    void SetSubtitleText_Gradually(FText Text);
    void SetSubtitleText();
    void ExecuteUbergraph_WBP_Dialogue_Inaka(int32 EntryPoint);
}; // Size: 0x390

#endif
