#ifndef BRAVE_BROWSER_EXTENSIONS_API_LEO_ASSISTANT_API_H_
#define BRAVE_BROWSER_EXTENSIONS_API_LEO_ASSISTANT_API_H_

#include "extensions/browser/extension_function.h"

namespace extensions {
namespace api {

class LeoSetShowLeoAssistantIconFunction : public ExtensionFunction {
public:
  DECLARE_EXTENSION_FUNCTION("leo.setShowLeoAssistantIcon", UNKNOWN)

protected:
    ~LeoSetShowLeoAssistantIconFunction() override {}
    ResponseAction Run() override;
};

class LeoGetShowLeoAssistantIconFunction : public ExtensionFunction {
public:
  DECLARE_EXTENSION_FUNCTION("leo.getShowLeoAssistantIcon", UNKNOWN)

protected:
    ~LeoGetShowLeoAssistantIconFunction() override {}
    ResponseAction Run() override;
};

class LeoResetFunction : public ExtensionFunction {
public:
  DECLARE_EXTENSION_FUNCTION("leo.reset", UNKNOWN)

protected:
    ~LeoResetFunction() override {}
    ResponseAction Run() override;
};

}  // namespace api
}  // namespace extensions

#endif // BRAVE_BROWSER_EXTENSIONS_API_LEO_ASSISTANT_API_H_