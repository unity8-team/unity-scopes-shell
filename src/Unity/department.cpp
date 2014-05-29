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

#include "department.h"

namespace scopes_ng
{

using namespace unity;

Department::Department(QObject* parent) :
    unity::shell::scopes::DepartmentInterface(parent),
    m_loaded(false),
    m_isRoot(false)
{
}

void Department::loadFromDepartmentNode(DepartmentNode* treeNode)
{
    if (treeNode == nullptr) {
        qWarning("Tried to set null DepartmentNode!");
        return;
    }
    m_departmentId = treeNode->id();
    m_label = treeNode->label();
    m_allLabel = treeNode->allLabel();
    m_loaded = !treeNode->isLeaf() && treeNode->childCount() > 0;
    m_isRoot = treeNode->isRoot();

    DepartmentNode* parentNode = treeNode->parent();
    m_parentId = parentNode ? parentNode->id() : "";
    m_parentLabel = parentNode ? parentNode->label() : "";

    beginResetModel();

    m_subdepartments.clear();
    Q_FOREACH (DepartmentNode* node, treeNode->childNodes()) {
        QSharedPointer<SubdepartmentData> subdept(new SubdepartmentData);
        subdept->id = node->id();
        subdept->label = node->label();
        subdept->hasChildren = node->hasSubdepartments();
        subdept->isActive = false;
        m_subdepartments.append(subdept);
    }

    endResetModel();

    Q_EMIT departmentIdChanged();
    Q_EMIT labelChanged();
    Q_EMIT allLabelChanged();
    Q_EMIT parentIdChanged();
    Q_EMIT parentLabelChanged();
    Q_EMIT loadedChanged();
    Q_EMIT countChanged();
    Q_EMIT isRootChanged();
}

void Department::markSubdepartmentActive(QString const& subdepartmentId)
{
    int idx = -1;
    for (int i = 0; i < m_subdepartments.count(); i++) {
        if (m_subdepartments[i]->id == subdepartmentId) {
            m_subdepartments[i]->isActive = true;
            idx = i;
            break;
        }
    }

    if (idx < 0) return;

    QVector<int> roles;
    roles.append(Roles::RoleIsActive);

    QModelIndex changedIndex(index(idx));
    dataChanged(changedIndex, changedIndex, roles);
}

QVariant Department::data(const QModelIndex& index, int role) const
{
    SubdepartmentData* data = m_subdepartments[index.row()].data();
    switch (role) {
        case RoleDepartmentId: return data->id;
        case RoleLabel: return data->label;
        case RoleHasChildren: return data->hasChildren;
        case RoleIsActive: return data->isActive;
        default: return QVariant();
    }
}

int Department::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_subdepartments.size();
}

QHash<int, QByteArray> Department::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[RoleDepartmentId] = "departmentId";
    roles[RoleLabel] = "label";
    roles[RoleHasChildren] = "hasChildren";
    roles[RoleIsActive] = "isActive";
    return roles;
}

QString Department::departmentId() const
{
    return m_departmentId;
}

QString Department::label() const
{
    return m_label;
}

QString Department::allLabel() const
{
    return m_allLabel;
}

QString Department::parentId() const
{
    return m_parentId;
}

QString Department::parentLabel() const
{
    return m_parentLabel;
}

bool Department::loaded() const
{
    return m_loaded;
}

bool Department::isRoot() const
{
    return m_isRoot;
}

int Department::count() const
{
    return rowCount();
}

} // namespace scopes_ng
