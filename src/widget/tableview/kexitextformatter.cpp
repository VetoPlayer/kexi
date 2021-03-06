/* This file is part of the KDE project
   Copyright (C) 2007-2016 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kexitextformatter.h"
#include <widget/utils/kexidatetimeformatter.h>

#include <KDb>

#include <QLocale>

//! @internal
class Q_DECL_HIDDEN KexiTextFormatter::Private
{
public:
    Private() : field(nullptr), dateFormatter(nullptr), timeFormatter(nullptr) {
    }

    ~Private() {
        delete dateFormatter;
        delete timeFormatter;
    }

    const KDbField* field;
    KexiDateFormatter *dateFormatter;
    KexiTimeFormatter *timeFormatter;
    KexiTextFormatter::OverrideDecimalPlaces overrideDecimalPlaces;
    QLocale locale;
};

KexiTextFormatter::KexiTextFormatter()
        : d(new Private)
{
}

KexiTextFormatter::~KexiTextFormatter()
{
    delete d;
}

void KexiTextFormatter::setField(const KDbField* field)
{
    d->field = field;
    if (!d->field)
        return;
    const KDbField::Type t = d->field->type();
    switch (t) {
    case KDbField::Date:
    case KDbField::DateTime:
        d->dateFormatter = new KexiDateFormatter();
        break;
    default:
        delete d->dateFormatter;
        d->dateFormatter = 0;
        break;
    }
    switch (t) {
    case KDbField::Time:
    case KDbField::DateTime:
        d->timeFormatter = new KexiTimeFormatter();
        break;
    default:
        delete d->timeFormatter;
        d->timeFormatter = 0;
        break;
    }
}

void KexiTextFormatter::setOverrideDecimalPlaces(const OverrideDecimalPlaces& overrideDecimalPlaces)
{
    d->overrideDecimalPlaces = overrideDecimalPlaces;
}

void KexiTextFormatter::setGroupSeparatorsEnabled(bool set)
{
    d->locale.setNumberOptions(set ? QLocale::NumberOptions() : QLocale::OmitGroupSeparator);
}

bool KexiTextFormatter::isGroupSeparatorsEnabled() const
{
    return !(d->locale.numberOptions() & QLocale::OmitGroupSeparator);
}

KexiTextFormatter::OverrideDecimalPlaces KexiTextFormatter::overridesDecimalPlaces() const
{
    return d->overrideDecimalPlaces;
}

//! toString() implementation for Text type
static QString toStringForTextType(const QVariant& value, const QString& add,
                                   const KDbField *field,
                                   bool *lengthExceeded)
{
    const QString str(value.toString());
    if (lengthExceeded) {
        if (field && field->maxLength() > 0) {
            *lengthExceeded = (str.length() + add.length()) > field->maxLength();
        }
        else {
            *lengthExceeded = false;
        }
    }
    return str + add;
}

QString KexiTextFormatter::toString(const QVariant& value, const QString& add,
                                    bool *lengthExceeded) const
{
    //cases, in order of expected frequency
    if (!d->field || d->field->type() == KDbField::Text) {
        return toStringForTextType(value, add, d->field, lengthExceeded);
    }

    if (lengthExceeded) {
        *lengthExceeded = false;
    }
    if (d->field->isIntegerType()) {
        if (!add.isEmpty() && value.toInt() == 0)
            return add; //eat 0
    }
    else if (d->field->isFPNumericType()) {
//! @todo precision!
//! @todo support 'g' format
        if (value.toDouble() == 0.0) {
            return add.isEmpty() ? QString::fromLatin1("0") : add; //eat 0
        }
        return KDb::numberToLocaleString(
                   value.toDouble(), // use Double for Float too for better accuracy
                   d->overrideDecimalPlaces.enabled ? d->overrideDecimalPlaces.value
                                                    : d->field->visibleDecimalPlaces(),
                   d->locale)
            + add;
    }

    switch (d->field->type()) {
    case KDbField::Boolean: {
    //! @todo temporary solution for booleans!
        const bool boolValue = value.isNull() ? QVariant(add).toBool() : value.toBool();
        return QString::fromLatin1(boolValue ? "1" : "0");
    }
    case KDbField::Date:
        return d->dateFormatter->toString(
                   value.toString().isEmpty() ? QDate() : value.toDate());
    case KDbField::Time:
        return d->timeFormatter->toString(
                   value.toString().isEmpty()
                   ? QTime(99, 0, 0) //hack to avoid converting null variant to valid QTime(0,0,0)
                   : value.toTime());
    case KDbField::DateTime:
        if (value.toString().isEmpty())
            return add;
        return KexiDateTimeFormatter::toString(
                   *d->dateFormatter, *d->timeFormatter, value.toDateTime());
    default:
        break;
    }
    //default: text
    return toStringForTextType(value, add, d->field, lengthExceeded);
}

QVariant KexiTextFormatter::fromString(const QString& text, bool *ok) const
{
    QVariant result;
    bool thisOk;
    if (!ok) {
        ok = &thisOk;
    }
    if (d->field) {
        switch (d->field->type()) {
        case KDbField::Text:
        case KDbField::LongText:
            *ok = true;
            result = text;
            break;
        case KDbField::Byte:
        case KDbField::ShortInteger:
            result = d->field->isUnsigned() ? QVariant(text.toUShort(ok)) : QVariant(text.toShort(ok));
            break;
    //! @todo uint, etc?
        case KDbField::Integer:
            result = d->field->isUnsigned() ? QVariant(text.toUInt(ok)) : QVariant(text.toInt(ok));
            break;
        case KDbField::BigInteger:
            result = d->field->isUnsigned() ? QVariant(text.toULongLong(ok)) : QVariant(text.toLongLong(ok));
            break;
        case KDbField::Boolean:
    //! @todo temporary solution for booleans!
            *ok = true;
            result = text == QString::fromLatin1("1");
            break;
        case KDbField::Date:
            result = d->dateFormatter->stringToVariant(text);
            *ok = !result.isNull();
            break;
        case KDbField::Time:
            result = d->timeFormatter->stringToVariant(text);
            *ok = !result.isNull();
            break;
        case KDbField::DateTime: {
            const QDateTime dt(KexiDateTimeFormatter::fromString(
                       *d->dateFormatter, *d->timeFormatter, text));
            *ok = dt.isValid();
            result = dt;
            break;
        }
        // locale parses decimal point and thousands group separators for us
        case KDbField::Float:
            result = d->locale.toFloat(text, ok);
            break;
        case KDbField::Double:
            result = d->locale.toDouble(text, ok);
            break;
        default:
            break;
        }
        if (!*ok) {
            result = QVariant();
        }
    } else {
        *ok = true;
    }
    return result;
//! @todo more data types!
}

bool KexiTextFormatter::valueIsEmpty(const QString& text) const
{
    if (text.isEmpty())
        return true;

    if (d->field) {
        switch (d->field->type()) {
        case KDbField::Date:
            return d->dateFormatter->isEmpty(text);
        case KDbField::Time:
            return d->timeFormatter->isEmpty(text);
        case KDbField::DateTime:
            return KexiDateTimeFormatter::isEmpty(*d->dateFormatter, *d->timeFormatter, text);
        default:
            break;
        }
    }

//! @todo
    return text.isEmpty();
}

bool KexiTextFormatter::valueIsValid(const QString& text) const
{
    if (!d->field)
        return true;
//! @todo fix for fields with "required" property = true
    if (valueIsEmpty(text)/*ok?*/)
        return true;

    switch (d->field->type()) {
    case KDbField::Date:
        return d->dateFormatter->stringToVariant(text).isValid();
    case KDbField::Time:
        return d->timeFormatter->stringToVariant(text).isValid();
    case KDbField::DateTime:
        return KexiDateTimeFormatter::isValid(*d->dateFormatter, *d->timeFormatter, text);
    default:
        break;
    }

//! @todo
    return true;
}

QString KexiTextFormatter::inputMask() const
{
    switch (d->field->type()) {
    case KDbField::Date:
//! @todo use KDateWidget?
        return d->dateFormatter->inputMask();
    case KDbField::Time:
//! @todo use KTimeWidget?
        return d->timeFormatter->inputMask();
    case KDbField::DateTime:
        return KexiDateTimeFormatter::inputMask(*d->dateFormatter, *d->timeFormatter);
    default:
        break;
    }

    return QString();
}

bool KexiTextFormatter::lengthExceeded(const QString& text) const
{
    return d->field && d->field->type() == KDbField::Text && d->field->maxLength() > 0
            && text.length() > d->field->maxLength();
}
