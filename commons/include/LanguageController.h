#ifndef LANGUAGECONTROLLER_H
#define LANGUAGECONTROLLER_H

#include <QObject>
#include <QApplication>
#include <QApplication>

class LanguageController : QObject
{
Q_OBJECT
public:
  enum LOCALE_TYPE {LOCALE_EN = 0, LOCALE_PT_BR, LOCALE_RU, LOCALE_LAST = LOCALE_RU};
  static LanguageController* Instance();
  static const QString& LocaleTypeToStr(LOCALE_TYPE lt);
  static const QString& LocaleTypeToVal(LOCALE_TYPE lt);
  static QString CurrentLocale();

private:
  LanguageController();
  virtual ~LanguageController();
};

#endif // LANGUAGECONTROLLER_H

