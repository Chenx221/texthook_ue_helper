#include <DynamicOutput/Output.hpp>
#include <Mod/CppUserModBase.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FFrame.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UClass.hpp>
#include <Unreal/FText.hpp>
#include <Unreal/FString.hpp>
#include <Unreal/Core/Containers/Array.hpp>
#include <windows.h>
#include <mutex>

using namespace RC;
using namespace Unreal;

/**
 * 导出函数：文本钩子点
 * 此函数被导出供外部工具 hook，用于截获即将复制到剪贴板的文本
 * @param text 要处理的文本
 * @return 处理后的文本（当前实现直接返回原文本）
 */
extern "C" __declspec(dllexport) const wchar_t* __cdecl TextHookPoint(const wchar_t* text)
{
    // 这个函数故意什么都不做，只是作为一个钩子点
    // 外部工具可以 hook 这个函数来截获文本
    return text;
}

bool CopyTextToClipboard(const std::wstring& text)
{
    if (text.empty()) return false;

    // 在复制到剪贴板之前，先通过钩子点
    // 这样外部工具可以截获文本
    const wchar_t* processed_text = TextHookPoint(text.c_str());
    if (!processed_text)
    {
        Output::send<LogLevel::Warning>(STR("[GalHome] TextHookPoint returned null, using original text\n"));
        processed_text = text.c_str();
    }

    if (!OpenClipboard(nullptr))
    {
        Output::send<LogLevel::Error>(STR("[GalHome] Failed to open clipboard\n"));
        return false;
    }

    if (!EmptyClipboard())
    {
        CloseClipboard();
        Output::send<LogLevel::Error>(STR("[GalHome] Failed to empty clipboard\n"));
        return false;
    }

    size_t size = (wcslen(processed_text) + 1) * sizeof(wchar_t);

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hGlobal)
    {
        CloseClipboard();
        Output::send<LogLevel::Error>(STR("[GalHome] Failed to allocate memory for clipboard\n"));
        return false;
    }

    if (auto* pGlobal = static_cast<wchar_t*>(GlobalLock(hGlobal)))
    {
        wcscpy_s(pGlobal, wcslen(processed_text) + 1, processed_text);
        GlobalUnlock(hGlobal);

        if (!SetClipboardData(CF_UNICODETEXT, hGlobal))
        {
            GlobalFree(hGlobal);
            CloseClipboard();
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to set clipboard data\n"));
            return false;
        }
    }
    else
    {
        GlobalFree(hGlobal);
        CloseClipboard();
        Output::send<LogLevel::Error>(STR("[GalHome] Failed to lock global memory\n"));
        return false;
    }

    CloseClipboard();
    return true;
}

// 安全获取 FText 属性的辅助函数
bool SafeGetFTextProperty(UObject* Context, UClass* ObjectClass, const wchar_t* PropertyName, StringType& OutValue)
{
    if (!Context || !ObjectClass || !PropertyName) return false;

    try
    {
        auto* Property = ObjectClass->GetPropertyByNameInChain(PropertyName);
        if (!Property) return false;

        void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(Context);
        if (!PropertyValuePtr) return false;

        FName property_type = Property->GetClass().GetFName();
        static auto s_text_property_name = FName(STR("TextProperty"), FNAME_Add);

        if (!property_type.Equals(s_text_property_name))
        {
            Output::send<LogLevel::Warning>(STR("[GalHome] Property '{}' is not FText type, got type: {}\n"), PropertyName, property_type.ToString());
            return false;
        }

        const auto* TextValue = static_cast<FText*>(PropertyValuePtr);
        OutValue = TextValue->ToString();
        return true;
    }
    catch (const std::exception& e)
    {
        Output::send<LogLevel::Error>(STR("[GalHome] Exception while getting property '{}': {}\n"), PropertyName, ensure_str(e.what()));
        return false;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("[GalHome] Unknown exception while getting property '{}'\n"), PropertyName);
        return false;
    }
}

// 安全获取 TArray<FString> 属性的辅助函数
bool SafeGetArrayProperty(UObject* Context, UClass* ObjectClass, const wchar_t* PropertyName, TArray<FString>** OutArray)
{
    if (!Context || !ObjectClass || !PropertyName || !OutArray) return false;

    try
    {
        auto* Property = ObjectClass->GetPropertyByNameInChain(PropertyName);
        if (!Property) return false;

        void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(Context);
        if (!PropertyValuePtr) return false;

        *OutArray = static_cast<TArray<FString>*>(PropertyValuePtr);
        return *OutArray != nullptr;
    }
    catch (const std::exception& e)
    {
        Output::send<LogLevel::Error>(STR("[GalHome] Exception while getting array property '{}': {}\n"), PropertyName, ensure_str(e.what()));
        return false;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("[GalHome] Unknown exception while getting array property '{}'\n"), PropertyName);
        return false;
    }
}

class GalHome : public CppUserModBase
{
  private:
    // 钩子数据结构
    struct HookData
    {
        std::pair<int, int> ids{};
        bool registered{false};
        StringType target_name{};
        int retry_count{0};
        bool should_retry{true};
        static constexpr int max_retries = 5;  // 最大重试次数
    };

    // 初始化标志
    static inline bool m_initialized = false;

    // 钩子数据
    HookData m_dialogue_hook;
    HookData m_dialogue2_hook;
    HookData m_choice_hook;
    HookData m_choice2_hook;

    // Choice2 去重相关
    StringType m_pre_choice2_text{};
    std::mutex m_choice2_mutex;

  public:
    GalHome()
    {
        ModVersion = STR("0.2");
        ModName = STR("GalHome");
        ModAuthors = STR("Chenx221");
        ModDescription = STR("For GalHome-Win64-Shipping");

        Output::send<LogLevel::Warning>(STR("[GalHome] Init v{}\n"), ModVersion);

        // 验证剪贴板访问
        if (OpenClipboard(nullptr))
        {
            CloseClipboard();
        }
        else
        {
            Output::send<LogLevel::Warning>(STR("[GalHome] Warning: Clipboard access may be limited\n"));
        }
    }

    ~GalHome() override
    {
        UnregisterHook(m_dialogue_hook);
        UnregisterHook(m_dialogue2_hook);
        UnregisterHook(m_choice_hook);
        UnregisterHook(m_choice2_hook);

        Output::send<LogLevel::Warning>(STR("[GalHome] Unloaded\n"));
    }

    auto on_program_start() -> void override
    {
    }

    auto on_dll_load(std::wstring_view dll_name) -> void override
    {
    }

    auto on_unreal_init() -> void override
    {
        if (m_initialized)
        {
            Output::send<LogLevel::Warning>(STR("[GalHome] Already initialized, skipping\n"));
            return;
        }
        m_initialized = true;

        Output::send<LogLevel::Warning>(STR("[GalHome] Starting hook registration\n"));

        // 尝试立即注册所有钩子
        TryRegisterDialogueHook();
        TryRegisterDialogue2Hook();
        TryRegisterChoiceHook();
        TryRegisterChoice2Hook();

        Output::send<LogLevel::Warning>(STR("[GalHome] Initial hook registration complete (hooks may register later if not found)\n"));
    }

    auto on_update() -> void override
    {
        // 检查并重试未注册的钩子
        if (!m_dialogue_hook.registered && m_dialogue_hook.should_retry)
        {
            TryRegisterDialogueHook();
        }
        if (!m_dialogue2_hook.registered && m_dialogue2_hook.should_retry)
        {
            TryRegisterDialogue2Hook();
        }
        if (!m_choice_hook.registered && m_choice_hook.should_retry)
        {
            TryRegisterChoiceHook();
        }
        if (!m_choice2_hook.registered && m_choice2_hook.should_retry)
        {
            TryRegisterChoice2Hook();
        }
    }

  private:
    // 统一的钩子注销函数
    void UnregisterHook(HookData& hook)
    {
        if (hook.registered)
        {
            try
            {
                UObjectGlobals::UnregisterHook(hook.target_name, hook.ids);
                hook.registered = false;
                Output::send<LogLevel::Verbose>(STR("[GalHome] Unregistered hook: {}\n"), hook.target_name);
            }
            catch (const std::exception& e)
            {
                Output::send<LogLevel::Error>(STR("[GalHome] Error unregistering hook: {}\n"), ensure_str(e.what()));
            }
        }
    }

    // 提取通用的对话处理逻辑
    static void ProcessDialogueText(UObject* Context, const wchar_t* talkerPropertyName, const wchar_t* dialoguePropertyName, const wchar_t* logPrefix)
    {
        if (!Context)
        {
            Output::send<LogLevel::Verbose>(STR("[GalHome] ProcessDialogueText: Context is null\n"));
            return;
        }

        auto* ObjectClass = Context->GetClassPrivate();
        if (!ObjectClass)
        {
            Output::send<LogLevel::Verbose>(STR("[GalHome] ProcessDialogueText: Failed to get class\n"));
            return;
        }

        StringType talkerName;
        StringType dialogueText;

        SafeGetFTextProperty(Context, ObjectClass, talkerPropertyName, talkerName);
        SafeGetFTextProperty(Context, ObjectClass, dialoguePropertyName, dialogueText);

        if (!dialogueText.empty())
        {
            StringType fullText = talkerName.empty() ? dialogueText : STR("[") + talkerName + STR("] ") + dialogueText;

            Output::send<LogLevel::Warning>(STR("[{}] {}\n"), logPrefix, fullText);

            if (!CopyTextToClipboard(fullText))
            {
                Output::send<LogLevel::Error>(STR("[GalHome] Failed to copy dialogue to clipboard\n"));
            }
        }
    }

    // 提取通用的选择处理逻辑
    static void ProcessChoiceText(UObject* Context, const wchar_t* choicePropertyName, const wchar_t* logPrefix, GalHome* modInstance = nullptr)
    {
        if (!Context)
        {
            Output::send<LogLevel::Verbose>(STR("[GalHome] ProcessChoiceText: Context is null\n"));
            return;
        }

        auto* ObjectClass = Context->GetClassPrivate();
        if (!ObjectClass)
        {
            Output::send<LogLevel::Verbose>(STR("[GalHome] ProcessChoiceText: Failed to get class\n"));
            return;
        }

        TArray<FString>* ChoicesArray = nullptr;
        if (!SafeGetArrayProperty(Context, ObjectClass, choicePropertyName, &ChoicesArray)) return;

        if (ChoicesArray && ChoicesArray->Num() > 0)
        {
            StringType fullText;

            for (int32 i = ChoicesArray->Num() - 1; i >= 0; --i)
            {
                FString& Choice = (*ChoicesArray)[i];
                StringType ChoiceText(Choice.GetCharArray().GetData());


                if (!ChoiceText.empty())
                {
                    fullText += ChoiceText + STR("\n");
                }
            }

            if (!fullText.empty())
            {
                Output::send<LogLevel::Warning>(STR("[{}] {}\n"), logPrefix, fullText);

                // 如果提供了 modInstance，则使用去重逻辑（用于 Choice2）
                if (modInstance)
                {
                    std::lock_guard<std::mutex> lock(modInstance->m_choice2_mutex);
                    if (modInstance->m_pre_choice2_text == fullText)
                    {
                        Output::send<LogLevel::Verbose>(STR("[GalHome] Choice text is same as previous, not copying\n"));
                        return;
                    }
                    modInstance->m_pre_choice2_text = fullText;
                }

                if (!CopyTextToClipboard(fullText))
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Failed to copy choices to clipboard\n"));
                }
            }
        }
    }

    void TryRegisterDialogueHook()
    {
        if (m_dialogue_hook.registered || !m_dialogue_hook.should_retry)
            return;

        m_dialogue_hook.retry_count++;

        // 超过最大重试次数
        if (m_dialogue_hook.retry_count > HookData::max_retries)
        {
            m_dialogue_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Dialogue hook registration failed after {} retries, skipping\n"), HookData::max_retries);
            return;
        }

        try
        {
            const auto target_name = STR("/Game/0_Gal/UI/WBP/Story_Gal/WBP_StoryGalBeforeLifePart.WBP_StoryGalBeforeLifePart_C:StartDialogue");

            // 检查函数是否存在
            auto* Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, target_name);
            if (!Function)
            {
                if (m_dialogue_hook.retry_count == 1)
                {
                    Output::send<LogLevel::Verbose>(STR("[GalHome] Dialogue function not found yet, will retry\n"));
                }
                return;
            }

            auto pre_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {};

            auto post_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {
                try
                {
                    ProcessDialogueText(context.Context, STR("CurrentTalker"), STR("Dialogue Text"), STR("Dialogue"));
                }
                catch (const std::exception& e)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Exception in Dialogue PostHook: {}\n"), ensure_str(e.what()));
                }
                catch (...)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Unknown exception in Dialogue PostHook\n"));
                }
            };

            m_dialogue_hook.target_name = target_name;
            m_dialogue_hook.ids = UObjectGlobals::RegisterHook(m_dialogue_hook.target_name, pre_callback, post_callback, this);
            m_dialogue_hook.registered = true;
            m_dialogue_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Registered dialogue hook (retry: {})\n"), m_dialogue_hook.retry_count);
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register dialogue hook: {}\n"), ensure_str(e.what()));
        }
        catch (...)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register dialogue hook: Unknown error\n"));
        }
    }

    void TryRegisterDialogue2Hook()
    {
        if (m_dialogue2_hook.registered || !m_dialogue2_hook.should_retry)
            return;

        m_dialogue2_hook.retry_count++;

        // 超过最大重试次数
        if (m_dialogue2_hook.retry_count > HookData::max_retries)
        {
            m_dialogue2_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Dialogue2 hook registration failed after {} retries, skipping\n"), HookData::max_retries);
            return;
        }

        try
        {
            const auto target_name = STR("/Game/0_Gal/UI/WBP/Dialogue_Gal/WBP_Dialogue_Gal.WBP_Dialogue_Gal_C:SetDialogueText");

            // 检查函数是否存在
            auto* Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, target_name);
            if (!Function)
            {
                if (m_dialogue2_hook.retry_count == 1)
                {
                    Output::send<LogLevel::Verbose>(STR("[GalHome] Dialogue2 function not found yet, will retry\n"));
                }
                return;
            }

            auto pre_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {};

            auto post_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {
                try
                {
                    ProcessDialogueText(context.Context, STR("Talker"), STR("Dialogue Text"), STR("Dialogue2"));
                }
                catch (const std::exception& e)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Exception in Dialogue2 PostHook: {}\n"), ensure_str(e.what()));
                }
                catch (...)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Unknown exception in Dialogue2 PostHook\n"));
                }
            };

            m_dialogue2_hook.target_name = target_name;
            m_dialogue2_hook.ids = UObjectGlobals::RegisterHook(m_dialogue2_hook.target_name, pre_callback, post_callback, this);
            m_dialogue2_hook.registered = true;
            m_dialogue2_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Registered dialogue2 hook (retry: {})\n"), m_dialogue2_hook.retry_count);
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register dialogue2 hook: {}\n"), ensure_str(e.what()));
        }
        catch (...)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register dialogue2 hook: Unknown error\n"));
        }
    }

    void TryRegisterChoiceHook()
    {
        if (m_choice_hook.registered || !m_choice_hook.should_retry)
            return;

        m_choice_hook.retry_count++;

        // 超过最大重试次数
        if (m_choice_hook.retry_count > HookData::max_retries)
        {
            m_choice_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Choice hook registration failed after {} retries, skipping\n"), HookData::max_retries);
            return;
        }

        try
        {
            const auto target_name = STR("/Game/0_Gal/UI/WBP/Story_Gal/WBP_StoryGalBeforeLifePart.WBP_StoryGalBeforeLifePart_C:SetUp_ChoiceButton");

            // 检查函数是否存在
            auto* Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, target_name);
            if (!Function)
            {
                if (m_choice_hook.retry_count == 1)
                {
                    Output::send<LogLevel::Verbose>(STR("[GalHome] Choice function not found yet, will retry\n"));
                }
                return;
            }

            auto pre_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {};

            auto post_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {
                try
                {
                    ProcessChoiceText(context.Context, STR("Answer Choices"), STR("Choice"));
                }
                catch (const std::exception& e)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Exception in Choice PostHook: {}\n"), ensure_str(e.what()));
                }
                catch (...)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Unknown exception in Choice PostHook\n"));
                }
            };

            m_choice_hook.target_name = target_name;
            m_choice_hook.ids = UObjectGlobals::RegisterHook(m_choice_hook.target_name, pre_callback, post_callback, this);
            m_choice_hook.registered = true;
            m_choice_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Registered choice hook (retry: {})\n"), m_choice_hook.retry_count);
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register choice hook: {}\n"), ensure_str(e.what()));
        }
        catch (...)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register choice hook: Unknown error\n"));
        }
    }

    void TryRegisterChoice2Hook()
    {
        if (m_choice2_hook.registered || !m_choice2_hook.should_retry)
            return;

        m_choice2_hook.retry_count++;

        // 超过最大重试次数
        if (m_choice2_hook.retry_count > HookData::max_retries)
        {
            m_choice2_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Choice2 hook registration failed after {} retries, skipping\n"), HookData::max_retries);
            return;
        }

        try
        {
            const auto target_name = STR("/Game/0_Gal/UI/WBP/Dialogue_Gal/WBP_Dialogue_Gal.WBP_Dialogue_Gal_C:SetAnswerChoices");

            // 检查函数是否存在
            auto* Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, target_name);
            if (!Function)
            {
                if (m_choice2_hook.retry_count == 1)
                {
                    Output::send<LogLevel::Verbose>(STR("[GalHome] Choice2 function not found yet, will retry\n"));
                }
                return;
            }

            auto pre_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {};

            auto post_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {
                try
                {
                    auto* mod_instance = static_cast<GalHome*>(custom_data);
                    if (mod_instance)
                    {
                        ProcessChoiceText(context.Context, STR("Answer Choices"), STR("Choice2"), mod_instance);
                    }
                }
                catch (const std::exception& e)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Exception in Choice2 PostHook: {}\n"), ensure_str(e.what()));
                }
                catch (...)
                {
                    Output::send<LogLevel::Error>(STR("[GalHome] Unknown exception in Choice2 PostHook\n"));
                }
            };

            m_choice2_hook.target_name = target_name;
            m_choice2_hook.ids = UObjectGlobals::RegisterHook(m_choice2_hook.target_name, pre_callback, post_callback, this);
            m_choice2_hook.registered = true;
            m_choice2_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[GalHome] Registered choice2 hook (retry: {})\n"), m_choice2_hook.retry_count);
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register choice2 hook: {}\n"), ensure_str(e.what()));
        }
        catch (...)
        {
            Output::send<LogLevel::Error>(STR("[GalHome] Failed to register choice2 hook: Unknown error\n"));
        }
    }
};

/**
 * export the start_mod() and uninstall_mod() functions to
 * be used by the core ue4ss system to load in our dll mod
 */
#define MOD_EXPORT __declspec(dllexport)
extern "C"
{
    MOD_EXPORT RC::CppUserModBase* start_mod()
    {
        return new GalHome();
    }
    MOD_EXPORT void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}