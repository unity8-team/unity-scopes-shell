/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <unity/scopes/Category.h>
#include <unity/scopes/Result.h>

#include <deque>
#include <memory>
#include <string>

namespace unity
{
namespace scopeharness
{
typedef std::deque<unity::scopes::Result::SCPtr> ResultList;

typedef std::pair<unity::scopes::Category::SCPtr, ResultList> CategoryResultListPair;

typedef std::deque<CategoryResultListPair> CategoryList;

}
}
