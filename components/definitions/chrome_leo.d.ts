declare namespace chrome.leo {
    const reset: (callback: (success: boolean) => void) => void
    const getShowLeoAssistantIcon: (callback: (success: boolean) => void) => boolean
    const setShowLeoAssistantIcon: (is_visible: boolean, callback: (success: boolean) => void) => void
}