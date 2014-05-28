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


#ifndef NG_DEPARTMENT_H
#define NG_DEPARTMENT_H

//#include <unity/shell/scopes/DepartmentInterface.h>

#include <QString>
#include <QSharedPointer>
#include <QPointer>
#include <QAbstractListModel>

#include <unity/scopes/Result.h>
#include <unity/scopes/Department.h>

#include "departmentnode.h"

namespace scopes_ng
{

struct SubdepartmentData
{
    QString id;
    QString label;
    bool hasChildren;
    bool isActive;
};

class Q_DECL_EXPORT Department : public QAbstractListModel //public unity::shell::scopes::DepartmentInterface
{
    Q_OBJECT

    Q_PROPERTY(QString departmentId READ departmentId NOTIFY departmentIdChanged)
    Q_PROPERTY(QString label READ label NOTIFY labelChanged)
    Q_PROPERTY(QString allLabel READ allLabel NOTIFY allLabelChanged)
    Q_PROPERTY(QString parentId READ parentId NOTIFY parentIdChanged)
    Q_PROPERTY(QString parentLabel READ parentLabel NOTIFY parentLabelChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
    Q_PROPERTY(bool isRoot READ isRoot NOTIFY isRootChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        RoleDepartmentId,
        RoleLabel,
        RoleHasChildren,
        RoleIsActive
    };

    explicit Department(QObject* parent = 0);
    void loadFromDepartmentNode(DepartmentNode* treeNode);
    void markSubdepartmentActive(QString const& subdepartmentId);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString departmentId() const;
    QString label() const;
    QString allLabel() const;
    QString parentId() const;
    QString parentLabel() const;
    bool loaded() const;
    bool isRoot() const;
    int count() const;

Q_SIGNALS:
    void departmentIdChanged();
    void labelChanged();
    void allLabelChanged();
    void parentIdChanged();
    void parentLabelChanged();
    void loadedChanged();
    void isRootChanged();
    void countChanged();

private:
    QString m_departmentId;
    QString m_label;
    QString m_allLabel;
    QString m_parentId;
    QString m_parentLabel;
    bool m_loaded;
    bool m_isRoot;

    QList<QSharedPointer<SubdepartmentData>> m_subdepartments;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::Department*)

#endif // NG_DEPARTMENT_H
