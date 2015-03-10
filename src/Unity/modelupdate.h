/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
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

#include <QSet>
#include <QString>
#include <functional>

template <class ModelBase, class InputContainer, class OutputContainer>
class ModelUpdate: public ModelBase
{
public:
    void syncModel(InputContainer const& input, OutputContainer &model,
            const std::function<QString(typename InputContainer::value_type)>& inKeyFunc,
            const std::function<QString(typename OutputContainer::value_type)>& outKeyFunc,
            const std::function<typename OutputContainer::value_type(typename InputContainer::value_type const&)> createFunc)
    {
        // TODO update?

        int pos = 0;
        QMap<QString, int> newItems;
        for (auto item: input)
        {
            newItems[inKeyFunc(item)] = pos++;
        }

        int row = 0;
        QSet<QString> oldItems; // lookup for filters that were already displayed
        // iterate over old filters, remove filters that are not present anymore
        for (auto it = model.begin(); it != model.end();)
        {
            const QString id = outKeyFunc(*it);
            if (!newItems.contains(id))
            {
                this->beginRemoveRows(QModelIndex(), row, row);
                it = model.erase(it);
                this->endRemoveRows();
            }
            else
            {
                oldItems.insert(id);
                ++it;
                ++row;
            }
        }

        // iterate over new filters, insert new filters
        row = 0;
        for (auto const& item: input)
        {
            if (!oldItems.contains(inKeyFunc(item)))
            {
                auto filterObj = createFunc(item);
                if (filterObj)
                {
                    this->beginInsertRows(QModelIndex(), row, row);
                    //model.insert(row, filterObj);
                    this->endInsertRows();
                }
            }
            row++;
        }

        // move filters if position changed
        for (int i = 0; i<model.size(); )
        {
            auto const id = outKeyFunc(model[i]);
            int pos = newItems.value(id, -1);
            if (pos >= 0 && pos != i) {
                this->beginMoveRows(QModelIndex(), i, i, QModelIndex(), pos + (pos > i ? 1 : 0));
                model.move(i, pos);
                this->endMoveRows();
                continue;
            }
            i++;
        }
    }
};
