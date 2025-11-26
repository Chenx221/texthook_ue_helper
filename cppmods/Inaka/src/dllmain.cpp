#include <DynamicOutput/Output.hpp>
#include <Mod/CppUserModBase.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UClass.hpp>
#include <Unreal/FText.hpp>
#include <Unreal/Core/Containers/Array.hpp>
#include <windows.h>
#include <mutex>
#include <regex>

using namespace RC;
using namespace Unreal;

extern "C" __declspec(dllexport) __declspec(noinline) const wchar_t* __cdecl TextHookPoint(const wchar_t* text)
{
    return text;
}

bool CopyTextToClipboard(const std::wstring& text)
{
    if (text.empty()) return false;

    const wchar_t* processed_text = TextHookPoint(text.c_str());
    if (!processed_text)
    {
        Output::send<LogLevel::Warning>(STR("[Inaka] TextHookPoint returned null, using original text\n"));
        processed_text = text.c_str();
    }

    if (!OpenClipboard(nullptr))
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Failed to open clipboard\n"));
        return false;
    }

    if (!EmptyClipboard())
    {
        CloseClipboard();
        Output::send<LogLevel::Error>(STR("[Inaka] Failed to empty clipboard\n"));
        return false;
    }

    size_t size = (wcslen(processed_text) + 1) * sizeof(wchar_t);

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hGlobal)
    {
        CloseClipboard();
        Output::send<LogLevel::Error>(STR("[Inaka] Failed to allocate memory for clipboard\n"));
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
            Output::send<LogLevel::Error>(STR("[Inaka] Failed to set clipboard data\n"));
            return false;
        }
    }
    else
    {
        GlobalFree(hGlobal);
        CloseClipboard();
        Output::send<LogLevel::Error>(STR("[Inaka] Failed to lock global memory\n"));
        return false;
    }

    CloseClipboard();
    return true;
}

bool SafeGetFStringProperty(UObject* Context, UClass* ObjectClass, const wchar_t* PropertyName, StringType& OutValue)
{
    if (!Context || !ObjectClass || !PropertyName) return false;

    try
    {
        auto* Property = ObjectClass->GetPropertyByNameInChain(PropertyName);
        if (!Property) return false;

        void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(Context);
        if (!PropertyValuePtr) return false;

        const auto* StringValue = static_cast<FString*>(PropertyValuePtr);
        OutValue = StringValue->GetCharArray().GetData();
        return true;
    }
    catch (const std::exception& e)
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Exception while getting FString property '{}': {}\n"), PropertyName, ensure_str(e.what()));
        return false;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Unknown exception while getting FString property '{}'\n"), PropertyName);
        return false;
    }
}

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
        Output::send<LogLevel::Error>(STR("[Inaka] Exception while getting array property '{}': {}\n"), PropertyName, ensure_str(e.what()));
        return false;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Unknown exception while getting array property '{}'\n"), PropertyName);
        return false;
    }
}

bool SafeGetTextBlockText(UObject* Context, UClass* ObjectClass, const wchar_t* PropertyName, StringType& OutValue)
{
    if (!Context || !ObjectClass || !PropertyName) return false;

    try
    {
        auto* Property = ObjectClass->GetPropertyByNameInChain(PropertyName);
        if (!Property) return false;

        void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(Context);
        if (!PropertyValuePtr) return false;

        auto* TextBlock = *static_cast<UObject**>(PropertyValuePtr);
        if (!TextBlock) return false;

        auto* TextBlockClass = TextBlock->GetClassPrivate();
        if (!TextBlockClass) return false;

        auto* TextProperty = TextBlockClass->GetPropertyByNameInChain(STR("Text"));
        if (!TextProperty) return false;

        void* TextValuePtr = TextProperty->ContainerPtrToValuePtr<void>(TextBlock);
        if (!TextValuePtr) return false;

        const auto* TextValue = static_cast<FText*>(TextValuePtr);
        OutValue = TextValue->ToString();
        return true;
    }
    catch (const std::exception& e)
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Exception while getting TextBlock text '{}': {}\n"), PropertyName, ensure_str(e.what()));
        return false;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Unknown exception while getting TextBlock text '{}'\n"), PropertyName);
        return false;
    }
}

bool SafeGetFTextParameter(UnrealScriptFunctionCallableContext& context, const wchar_t* ParameterName, StringType& OutValue)
{
    if (!context.TheStack.Locals() || !ParameterName) return false;

    try
    {
        auto* Property = context.TheStack.Node()->GetPropertyByNameInChain(ParameterName);
        if (!Property) return false;

        void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(context.TheStack.Locals());
        if (!PropertyValuePtr) return false;

        const auto* TextValue = static_cast<FText*>(PropertyValuePtr);
        OutValue = TextValue->ToString();
        return true;
    }
    catch (const std::exception& e)
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Exception while getting FText parameter '{}': {}\n"), ParameterName, ensure_str(e.what()));
        return false;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("[Inaka] Unknown exception while getting FText parameter '{}'\n"), ParameterName);
        return false;
    }
}

class Inaka : public CppUserModBase
{
  private:
    struct HookData
    {
        std::pair<int, int> ids{};
        bool registered{false};
        StringType target_name{};
        int retry_count{0};
        bool should_retry{true};
        static constexpr int max_retries = 1000;

        uint64_t last_retry_tick{0};
        static constexpr uint64_t retry_interval_ms = 500;
    };

    // 初始化标志
    static inline bool m_initialized = false;

    // 钩子数据
    HookData m_dialogue_hook;  // 对话hook

    // 去重相关
    StringType m_previous_text{};
    std::mutex m_text_mutex;

  public:
    Inaka()
    {
        ModVersion = STR("0.5");
        ModName = STR("Inaka");
        ModAuthors = STR("Chenx221");
        ModDescription = STR("Text extractor for UE games");

        Output::send<LogLevel::Warning>(STR("[Inaka] Init v{}\n"), ModVersion);

        if (OpenClipboard(nullptr))
        {
            CloseClipboard();
        }
        else
        {
            Output::send<LogLevel::Warning>(STR("[Inaka] Warning: Clipboard access may be limited\n"));
        }
    }

    ~Inaka() override
    {
        UnregisterHook(m_dialogue_hook);
        Output::send<LogLevel::Warning>(STR("[Inaka] Unloaded\n"));
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
            Output::send<LogLevel::Warning>(STR("[Inaka] Already initialized, skipping\n"));
            return;
        }
        m_initialized = true;

        Output::send<LogLevel::Warning>(STR("[Inaka] Starting hook registration\n"));
        TryRegisterDialogueHook();
        Output::send<LogLevel::Warning>(STR("[Inaka] Initial hook registration complete (hook may register later if not found)\n"));
    }

    auto on_update() -> void override
    {
        if (!m_dialogue_hook.registered && m_dialogue_hook.should_retry)
        {
            TryRegisterDialogueHook();
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
                Output::send<LogLevel::Verbose>(STR("[Inaka] Unregistered hook: {}\n"), hook.target_name);
            }
            catch (const std::exception& e)
            {
                Output::send<LogLevel::Error>(STR("[Inaka] Error unregistering hook: {}\n"), ensure_str(e.what()));
            }
        }
    }

    static void ApplyTextFilters(StringType& combinedText)
    {
        try
        {
            // Regex: |織部《おりべ》 → 織部
            std::wregex ruby_re(
                L"\\|([^《]+)《.+?》",
                std::regex_constants::ECMAScript
            );

            combinedText = std::regex_replace(combinedText, ruby_re, L"$1");
        }
        catch (const std::regex_error& re_err)
        {
            Output::send<LogLevel::Error>(
                STR("[Inaka] Regex error: {}\n"),
                ensure_str(re_err.what())
            );
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(
                STR("[Inaka] Exception during filtering: {}\n"),
                ensure_str(e.what())
            );
        }
    }

    // 提取并合并所有文本
    static void ExtractAndCopyText(UnrealScriptFunctionCallableContext& context, Inaka* modInstance)
    {
        if (!context.TheStack.Locals())
        {
            Output::send<LogLevel::Verbose>(STR("[Inaka] ExtractAndCopyText: Locals is null\n"));
            return;
        }

        StringType speakerName;
        StringType dialogueText;

        // 使用辅助函数从函数参数中获取 SpeakerName 和 DialogueText
        if (SafeGetFTextParameter(context, STR("SpeakerName"), speakerName))
        {
            Output::send<LogLevel::Verbose>(STR("[Inaka] SpeakerName: {}\n"), speakerName);
        }

        if (SafeGetFTextParameter(context, STR("DialogueText"), dialogueText))
        {
            Output::send<LogLevel::Verbose>(STR("[Inaka] DialogueText: {}\n"), dialogueText);
        }

        // 组合文本
        StringType combinedText;
        if (!speakerName.empty() && !dialogueText.empty())
        {
            combinedText = STR("[") + speakerName + STR("] ") + dialogueText;
        }
        else if (!dialogueText.empty())
        {
            combinedText = dialogueText;
        }
        else if (!speakerName.empty())
        {
            combinedText = speakerName;
        }

        if (!combinedText.empty())
        {
            if (modInstance)
            {
                std::lock_guard lock(modInstance->m_text_mutex);
                if (modInstance->m_previous_text == combinedText)
                {
                    Output::send<LogLevel::Verbose>(STR("[Inaka] Text is same as previous, not copying\n"));
                    return;
                }

                ApplyTextFilters(combinedText);

                modInstance->m_previous_text = combinedText;
            }

            Output::send<LogLevel::Warning>(STR("[Inaka] Extracted text: {}\n"), combinedText);

            if (!CopyTextToClipboard(combinedText))
            {
                Output::send<LogLevel::Error>(STR("[Inaka] Failed to copy text to clipboard\n"));
            }
        }
        else
        {
            Output::send<LogLevel::Verbose>(STR("[Inaka] No text extracted\n"));
        }
    }


    void TryRegisterDialogueHook()
    {
        if (m_dialogue_hook.registered || !m_dialogue_hook.should_retry) return;

        uint64_t now = GetTickCount64();

        if (now - m_dialogue_hook.last_retry_tick < HookData::retry_interval_ms)
            return;

        m_dialogue_hook.last_retry_tick = now;
        m_dialogue_hook.retry_count++;

        // 超过最大重试次数
        if (m_dialogue_hook.retry_count > HookData::max_retries)
        {
            m_dialogue_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[Inaka] I_UpdateDialogueWidget hook registration failed after {} retries, skipping\n"), HookData::max_retries);
            return;
        }

        try
        {
            const auto target_name = STR("/Game/Inaka/UI/Dialogue/WBP_Dialogue_Inaka.WBP_Dialogue_Inaka_C:I_UpdateDialogueWidget");

            auto* Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, target_name);
            if (!Function)
            {
                if (m_dialogue_hook.retry_count == 1)
                {
                    Output::send<LogLevel::Verbose>(STR("[Inaka] I_UpdateDialogueWidget function not found yet, will retry\n"));
                }
                return;
            }

            auto pre_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {};

            auto post_callback = [](UnrealScriptFunctionCallableContext& context, void* custom_data) -> void {
                try
                {
                    auto* mod_instance = static_cast<Inaka*>(custom_data);
                    ExtractAndCopyText(context, mod_instance);
                }
                catch (const std::exception& e)
                {
                    Output::send<LogLevel::Error>(STR("[Inaka] Exception in I_UpdateDialogueWidget PostHook: {}\n"), ensure_str(e.what()));
                }
                catch (...)
                {
                    Output::send<LogLevel::Error>(STR("[Inaka] Unknown exception in I_UpdateDialogueWidget PostHook\n"));
                }
            };

            m_dialogue_hook.target_name = target_name;
            m_dialogue_hook.ids = UObjectGlobals::RegisterHook(m_dialogue_hook.target_name, pre_callback, post_callback, this);
            m_dialogue_hook.registered = true;
            m_dialogue_hook.should_retry = false;
            Output::send<LogLevel::Warning>(STR("[Inaka] Registered I_UpdateDialogueWidget hook (retry: {})\n"), m_dialogue_hook.retry_count);
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("[Inaka] Failed to register I_UpdateDialogueWidget hook: {}\n"), ensure_str(e.what()));
        }
        catch (...)
        {
            Output::send<LogLevel::Error>(STR("[Inaka] Failed to register I_UpdateDialogueWidget hook: Unknown error\n"));
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
    MOD_EXPORT CppUserModBase* start_mod()
    {
        return new Inaka();
    }
    MOD_EXPORT void uninstall_mod(CppUserModBase* mod)
    {
        delete mod;
    }
}