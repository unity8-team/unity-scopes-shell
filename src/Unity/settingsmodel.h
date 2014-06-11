/*
 * setttingsmodel.h
 *
 *  Created on: 10 Jun 2014
 *      Author: pete
 */

#ifndef NG_PREVIEW_SETTTINGSMODEL_H_
#define NG_PREVIEW_SETTTINGSMODEL_H_

#include <libu1db-qt5/database.h>
#include <libu1db-qt5/document.h>
#include <unity/SymbolExport.h>
#include <unity/shell/scopes/SettingsModelInterface.h>

#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>

namespace scopes_ng
{

class SettingsModel: public unity::shell::scopes::SettingsModelInterface
{
Q_OBJECT

    struct Data
    {
        QString id;
        QString displayName;
        QString type;
        QVariantMap data;

        Data(QString const& id_, QString const& displayName_,
                QString const& type_, QVariantMap const& data_)
                : id(id_), displayName(displayName_), type(type_), data(data_)
        {
        }
    };

public:
    explicit SettingsModel(const QString& scopeId, const QByteArray& json,
            QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
            override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE
    void setValue(const QString& settingName, const QVariant& value);

protected:
    QList<QSharedPointer<Data>> m_data;

    U1db::Database m_database;

    QMap<QString, QSharedPointer<U1db::Document>> m_documents;
};

}

Q_DECLARE_METATYPE(scopes_ng::SettingsModel*)

#endif /* NG_PREVIEW_SETTTINGSMODEL_H_ */
