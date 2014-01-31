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
            for (unsigned i = 0; i < arr.size(); i++) {
                result_list.append(scopeVariantToQVariant(arr[i]));
            }
            return result_list;
        }
        default:
            qWarning("Unhandled Variant type");
            return QVariant();
    }
}

} // namespace scopes_ng
