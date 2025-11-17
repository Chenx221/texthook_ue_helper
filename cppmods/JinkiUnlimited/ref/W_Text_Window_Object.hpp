#ifndef UE4SS_SDK_W_Text_Window_Object_HPP
#define UE4SS_SDK_W_Text_Window_Object_HPP

class UW_Text_Window_Object_C : public UUserWidget
{
    FPointerToUberGraphFrame UberGraphFrame;                                          // 0x0260 (size: 0x8)
    class UWidgetAnimation* Visible;                                                  // 0x0268 (size: 0x8)
    TArray<FString> TextArray;                                                        // 0x0270 (size: 0x10)
    FSlateFontInfo Font Style Main;                                                   // 0x0280 (size: 0x58)
    FSlateFontInfo Font Style Ruby;                                                   // 0x02D8 (size: 0x58)
    float Text_Fade_Timer;                                                            // 0x0330 (size: 0x4)
    int32 Text_Visible_Index;                                                         // 0x0334 (size: 0x4)
    int32 Text Row Count;                                                             // 0x0338 (size: 0x4)
    int32 Text_Count;                                                                 // 0x033C (size: 0x4)
    FW_Text_Window_Object_CALL_TextFade_End ALL_TextFade_End;                         // 0x0340 (size: 0x10)
    void ALL_TextFade_End();
    bool Debag;                                                                       // 0x0350 (size: 0x1)
    float Text_Window_Padding_X;                                                      // 0x0354 (size: 0x4)
    float Text_Window_Padding_Y;                                                      // 0x0358 (size: 0x4)
    bool Set_First_Size;                                                              // 0x035C (size: 0x1)
    class UCanvasPanel* TargetBackGound;                                              // 0x0360 (size: 0x8)
    class UVerticalBox* TargetText_VerticalBox;                                       // 0x0368 (size: 0x8)
    class UTextBlock* TargetName_Box;                                                 // 0x0370 (size: 0x8)
    class UCanvasPanel* TargetCanvasPanel;                                            // 0x0378 (size: 0x8)

    void ToggleUI();
    float Play_Visibility(FString Selection);
    void Parse_String(FString Temp1, TArray<FString>& Array);
    void Text_Create(FString String);
    void Test_Text(FString NewParam, TArray<FString>& NewLocalVar_01);
    void Set_Appearance(int32 Text_RowCount, int32 Text_Count);
    void Text_Continue(bool Continue_Flag);
    void Setup_Text(FString In String, bool IsFade);
    void Start_Text_Visible();
    void End_Fade();
    void Set_Size();
    void PreConstruct(bool IsDesignTime);
    void Click_End_Fade();
    void Stop_Fade();
    void Unstop_Fade();
    void WindowConstruct(class UCanvasPanel* BackGound_Canvas, class UVerticalBox* Text_VerticalBox, class UTextBlock* Name_BOx, class UCanvasPanel* CanvasPanel_0);
    void ExecuteUbergraph_W_Text_Window_Object(int32 EntryPoint);
    void ALL_TextFade_End__DelegateSignature();
}; // Size: 0x380

#endif
