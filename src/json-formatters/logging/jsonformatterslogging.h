#ifndef JSONFORMATTERSLOGGING_H
#define JSONFORMATTERSLOGGING_H

#ifndef RADAPTER_SHARED_SRC
#define RADAPTER_SHARED_SRC
#endif

#include <QLoggingCategory>
#include <QDebug>

RADAPTER_SHARED_SRC Q_DECLARE_LOGGING_CATEGORY(JsonFormatters);

#define jfWarn() qCWarning(JsonFormatters)

#ifndef CUSTOM_MESSAGE_PATTERN
#define CUSTOM_MESSAGE_PATTERN  "[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] "\
"%{if-category}%{category}: %{endif}%{message}"
#endif

#endif // JSONFORMATTERSLOGGING_H
