#ifndef UE4SS_SDK_WBP_Kari_TaskChoices_HPP
#define UE4SS_SDK_WBP_Kari_TaskChoices_HPP

class UWBP_Kari_TaskChoices_C : public UUserWidget
{
    FPointerToUberGraphFrame UberGraphFrame;                                          // 0x0340 (size: 0x8)
    class UTextBlock* Text_TaskText;                                                  // 0x0348 (size: 0x8)
    class UTextBlock* Text_TaskChoice3;                                               // 0x0350 (size: 0x8)
    class UTextBlock* Text_TaskChoice2;                                               // 0x0358 (size: 0x8)
    class UTextBlock* Text_TaskChoice1;                                               // 0x0360 (size: 0x8)
    class UImage* Line_Start_1;                                                       // 0x0368 (size: 0x8)
    class UImage* Line_Right_1;                                                       // 0x0370 (size: 0x8)
    class UImage* Line_End_1;                                                         // 0x0378 (size: 0x8)
    class UImage* Image_BackTaskName;                                                 // 0x0380 (size: 0x8)
    class UImage* Image_BackChoices;                                                  // 0x0388 (size: 0x8)
    class UButton* Button_TaskChoice3;                                                // 0x0390 (size: 0x8)
    class UButton* Button_TaskChoice2;                                                // 0x0398 (size: 0x8)
    class UButton* Button_TaskChoice1;                                                // 0x03A0 (size: 0x8)
    class UWidgetAnimation* Animation_LineAppear;                                     // 0x03A8 (size: 0x8)
    FText Text;                                                                       // 0x03B0 (size: 0x10)
    FWBP_Kari_TaskChoices_CEventDispatcher_TaskChoiceSelected EventDispatcher_TaskChoiceSelected; // 0x03C0 (size: 0x10)
    void EventDispatcher_TaskChoiceSelected(FGameplayTag ChoiceId, bool AvailableChoice?);
    bool Display?;                                                                    // 0x03D0 (size: 0x1)
    FStr_TaskDisplayedInfo Task Displayed Info;                                       // 0x03D8 (size: 0x28)

    class UWidget* Get_Button_Interact_2_ToolTipWidget();
    class UWidget* Get_Button_Interact_1_ToolTipWidget();
    class UWidget* Get_Button_Interact_ToolTipWidget();
    void DisplayWidget(bool Display?);
    void ClickTaskChoiceButton(int32 ChoiceIndex);
    void BindedEventRestrictWidget(FName InteractedObjectName, bool NewRestrict);
    void Construct();
    void PreConstruct(bool IsDesignTime);
    void Deactivate();
    void Activate(FStr_TaskDisplayedInfo TaskDisplayedInfo);
    void BndEvt__WBP_Kari_TaskChoices_Button_TaskChoice1_K2Node_ComponentBoundEvent_3_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_Kari_TaskChoices_Button_TaskChoice2_K2Node_ComponentBoundEvent_4_OnButtonClickedEvent__DelegateSignature();
    void BndEvt__WBP_Kari_TaskChoices_Button_TaskChoice3_K2Node_ComponentBoundEvent_5_OnButtonClickedEvent__DelegateSignature();
    void ExecuteUbergraph_WBP_Kari_TaskChoices(int32 EntryPoint);
    void EventDispatcher_TaskChoiceSelected__DelegateSignature(FGameplayTag ChoiceId, bool AvailableChoice?);
}; // Size: 0x400

#endif
