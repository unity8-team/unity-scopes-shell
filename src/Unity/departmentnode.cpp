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

#include "departmentnode.h"

namespace scopes_ng
{

using namespace unity;

DepartmentNode::DepartmentNode(DepartmentNode* parent) : m_parent(parent), m_isRoot(false)
{
}

DepartmentNode::~DepartmentNode()
{
    clearChildren();
}

void DepartmentNode::initializeForDepartment(scopes::Department::SCPtr const& dep)
{
    m_id = QString::fromStdString(dep->id());
    m_label = QString::fromStdString(dep->label());
    m_allLabel = QString::fromStdString(dep->alternate_label());
    m_hasSubdepartments = dep->has_subdepartments();

    clearChildren();

    auto subdeps = dep->subdepartments();
    for (auto it = subdeps.begin(); it != subdeps.end(); ++it) {
        DepartmentNode* subdep = new DepartmentNode(this);
        subdep->initializeForDepartment(*it);
        this->appendChild(subdep);
    }
}

void DepartmentNode::setIsRoot(bool isRoot)
{
    m_isRoot = isRoot;
}

bool DepartmentNode::isRoot() const
{
    return m_isRoot;
}

DepartmentNode* DepartmentNode::findNodeById(QString const& id)
{
    if (id == m_id) return this;

    Q_FOREACH(DepartmentNode* child, m_children) {
        DepartmentNode* node = child->findNodeById(id);
        if (node) return node;
    }

    return nullptr;
}

QString DepartmentNode::id() const
{
    return m_id;
}

QString DepartmentNode::label() const
{
    return m_label;
}

QString DepartmentNode::allLabel() const
{
    return m_allLabel;
}

bool DepartmentNode::hasSubdepartments() const
{
    return m_hasSubdepartments;
}

void DepartmentNode::appendChild(DepartmentNode* child)
{
    m_children.append(child);
}

int DepartmentNode::childCount() const
{
    return m_children.count();
}

QList<DepartmentNode*> DepartmentNode::childNodes() const
{
    return m_children;
}

DepartmentNode* DepartmentNode::parent() const
{
    return m_parent;
}

bool DepartmentNode::isLeaf() const
{
    return m_children.count() == 0 && !m_hasSubdepartments;
}

void DepartmentNode::clearChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

} // namespace scopes_ng
