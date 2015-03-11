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

#ifndef NG_MODEL_UPDATE_H
#define NG_MODEL_UPDATE_H

#include <QSet>
#include <QString>
#include <functional>

template <class ModelBase, class InputContainer, class OutputContainer>
class ModelUpdate: public ModelBase
{
public:
    using InputKeyFunc = std::function<QString(typename InputContainer::value_type)>;
    using OutputKeyFunc = std::function<QString(typename OutputContainer::value_type)>;
    using CreateFunc = std::function<typename OutputContainer::value_type(typename InputContainer::value_type const&)>;
    using UpdateFunc = std::function<bool(typename InputContainer::value_type const&, typename OutputContainer::value_type const&)>;

    ModelUpdate(QObject *parent = nullptr): ModelBase(parent) {}

    void syncModel(InputContainer const& input,
            OutputContainer &model,
            const InputKeyFunc& inKeyFunc,
            const OutputKeyFunc& outKeyFunc,
            const CreateFunc& createFunc,
            const UpdateFunc& updateFunc)
    {
        QMap<QString, int> newItems; // lookup for recevied objects and their desired rows in the model
        QSet<QString> oldItems; // lookup for objects that were already displayed

        {
            int pos = 0;
            for (auto item: input)
            {
                newItems[inKeyFunc(item)] = pos++;
            }
        }

        // iterate over old objects, remove objects that are not present anymore
        {
            int row = 0;
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
        }

        // iterate over new objects and insert them in the model
        {
            int row = 0;
            for (auto const& item: input)
            {
                if (!oldItems.contains(inKeyFunc(item)))
                {
                    auto obj = createFunc(item);
                    if (obj)
                    {
                        this->beginInsertRows(QModelIndex(), row, row);
                        model.insert(row, obj);
                        this->endInsertRows();
                    }
                }
                row++;
            }
        }

        // move objects if position changed
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

        // check if any of the existing objects which didn't change position needs updating
        {
            int row = 0;
            for (auto const& in: input)
            {
                if (oldItems.contains(inKeyFunc(in)))
                {
                    if (!updateFunc(in, model[row]))
                    {
                        model[row] = createFunc(in);
                        Q_EMIT this->dataChanged(this->index(row, 0), this->index(row, 0)); // or beginRemoveRows & beginInsertRows ?
                    }
                }
                ++row;
            }
        }
    }
};

#endif
