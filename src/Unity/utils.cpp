/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Michal Hruby <michal.hruby@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// self
#include "utils.h"

#include <QStringList>

namespace scopes_ng
{

using namespace unity;

QVariant scopeVariantToQVariant(scopes::Variant const& variant)
{
    switch (variant.which()) {
        case scopes::Variant::Type::Null:
            return QVariant();
        case scopes::Variant::Type::Int:
            return QVariant(variant.get_int());
        case scopes::Variant::Type::Bool:
            return QVariant(variant.get_bool());
        case scopes::Variant::Type::String:
            return QVariant(QString::fromStdString(variant.get_string()));
        case scopes::Variant::Type::Double:
            return QVariant(variant.get_double());
        case scopes::Variant::Type::Dict: {
            scopes::VariantMap dict(variant.get_dict());
            QVariantMap result_dict;
            for (auto it = dict.begin(); it != dict.end(); ++it) {
                result_dict.insert(QString::fromStdString(it->first), scopeVariantToQVariant(it->second));
            }
            return result_dict;
        }
        case scopes::Variant::Type::Array: {
            scopes::VariantArray arr(variant.get_array());
            QVariantList result_list;
            result_list.reserve(arr.size());
            for (size_t i = 0; i < arr.size(); i++) {
                result_list.append(scopeVariantToQVariant(arr[i]));
            }
            return result_list;
        }
        default:
            qWarning("Unhandled Variant type");
            return QVariant();
    }
}

scopes::Variant qVariantToScopeVariant(QVariant const& variant)
{
    if (variant.isNull()) {
        return scopes::Variant();
    }

    switch (variant.type()) {
        case QMetaType::Bool:
            return scopes::Variant(variant.toBool());
        case QMetaType::Int:
            return scopes::Variant(variant.toInt());
        case QMetaType::UInt:
            return scopes::Variant(static_cast<int64_t>(variant.toUInt()));
        case QMetaType::Double:
            return scopes::Variant(variant.toDouble());
        case QMetaType::QString:
            return scopes::Variant(variant.toString().toStdString());
        case QMetaType::QVariantMap: {
            scopes::VariantMap vm;
            QVariantMap m(variant.toMap());
            for (auto it = m.begin(); it != m.end(); ++it) {
                vm[it.key().toStdString()] = qVariantToScopeVariant(it.value());
            }
            return scopes::Variant(vm);
        }
        case QMetaType::QVariantList: {
            QVariantList l(variant.toList());
            scopes::VariantArray arr;
            arr.reserve(l.size());
            for (int i = 0; i < l.size(); i++) {
                arr.push_back(qVariantToScopeVariant(l[i]));
            }
            return scopes::Variant(arr);
        }
        default:
            qWarning("Unhandled QVariant type: %s", variant.typeName());
            return scopes::Variant();
    }
}

QVariant backgroundUriToVariant(QString const& uri)
{
    if (uri.startsWith(QLatin1String("color:///"))) {
        QVariantList elements;
        elements.append(uri.mid(9));
        QVariantMap m;
        m[QStringLiteral("type")] = QStringLiteral("color");
        m[QStringLiteral("elements")] = elements;
        return m;
    } else if (uri.startsWith(QLatin1String("gradient:///"))) {
        QStringList parts = uri.mid(12).split(QStringLiteral("/"), QString::SkipEmptyParts);
        QVariantList elements;
        elements.reserve(parts.size());
        for (int i = 0; i < parts.size(); i++) {
            elements.append(parts[i]);
        }
        QVariantMap m;
        m[QStringLiteral("type")] = QStringLiteral("gradient");
        m[QStringLiteral("elements")] = elements;
        return m;
    } else {
        return QVariant(uri);
    }
}

Q_DECL_EXPORT QString uuidToString(QUuid const& uuid)
{
    // workaround: use mid to get rid of curly braces; see https://bugreports.qt-project.org/browse/QTBUG-885
    auto const uuid_str = uuid.toString();
    if (uuid_str.startsWith(QStringLiteral("{"))) {
        return uuid_str.mid(1, 36);
    }
    return uuid_str;
}

} // namespace scopes_ng
