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
 * Author: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <boost/python.hpp>
#include <unity/scopes/Variant.h>
#include <unity/UnityExceptions.h>

using namespace boost::python;
namespace us = unity::scopes;

struct variant_to_python_obj
{
    static PyObject* convert(const unity::scopes::Variant& v)
    {
        switch (v.which())
        {
            case us::Variant::Null:
                return incref(object().ptr());
            case us::Variant::Int:
                return incref(object(v.get_int()).ptr());
            case us::Variant::Bool:
                return incref(object(v.get_bool()).ptr());
            case us::Variant::String:
                return incref(object(v.get_string()).ptr());
            case us::Variant::Double:
                return incref(object(v.get_double()).ptr());
            case us::Variant::Dict:
                {
                    dict pydict;
                    for (auto el: v.get_dict())
                    {
                        pydict[el.first] = el.second;
                    }
                    return incref(pydict.ptr());
                }
                break;
            case us::Variant::Array:
                {
                    list pylist;
                    for (auto const el: v.get_array())
                    {
                        pylist.append(el);
                    }
                    return incref(pylist.ptr());
                }
                break;
            default:
                throw unity::InvalidArgumentException("Unsupported variant type");
        }
    }

    static void* convertible(PyObject *obj)
    {
        if (!PyBytes_Check(obj))
            return 0;
        return obj;
    }
};

us::Variant variant_demo()
{
    us::VariantMap vm2;
    vm2["a"] = us::Variant(true);
    us::VariantMap vm;
    vm["foo"] = "bar";
    vm["array"] = us::VariantArray({us::Variant(1), us::Variant(2), us::Variant("j")});
    vm["x"] = us::Variant::null();
    vm["y"] = us::Variant(10.3f);
    vm["inner"] = us::Variant(vm2);
    return us::Variant(vm);
}

void export_variant()
{
    boost::python::to_python_converter<unity::scopes::Variant, variant_to_python_obj>();

    def("variant_demo", variant_demo);
}
