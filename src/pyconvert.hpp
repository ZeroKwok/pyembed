// This file is part of the pymebed distribution.
// Copyright (c) 2018-2023 Zero Kwok.
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
// 
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this software; 
// If not, see <http://www.gnu.org/licenses/>.
//
// Author:  Zero Kwok
// Contact: zero.kwok@foxmail.com 
// 

#ifndef pyconvert_h__
#define pyconvert_h__

#include <boost/python.hpp>

//
// PyTypeObject*
//
struct string_from_python_type
{
    string_from_python_type()
    {
        boost::python::converter::registry::push_back(
            &convertible,
            &construct,
            boost::python::type_id<std::string>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (PyObject_IsInstance(obj_ptr, (PyObject*)&PyType_Type))
            return obj_ptr;
        return 0;
    }

    static void construct(
        PyObject* obj_ptr,
        boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        auto obj_str = PyObject_Str(obj_ptr);
        if (obj_str == 0) boost::python::throw_error_already_set();
        void* storage = (
            (boost::python::converter::rvalue_from_python_storage<std::string>*)
            data)->storage.bytes;
        new (storage) std::string(boost::python::extract<std::string>(obj_str) BOOST_EXTRACT_WORKAROUND);
        data->convertible = storage;
        Py_DECREF(obj_str);
    }
};

//
// PyBaseExceptionObject*
//
struct string_from_python_base_exception
{
    string_from_python_base_exception()
    {
        boost::python::converter::registry::push_back(
            &convertible,
            &construct,
            boost::python::type_id<std::string>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (PyObject_IsInstance(obj_ptr, PyExc_BaseException))
            return obj_ptr;
        return 0;
    }

    static void construct(
        PyObject* obj_ptr,
        boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        auto obj_str = PyObject_Str(obj_ptr);
        if (obj_str == 0) boost::python::throw_error_already_set();
        void* storage = (
            (boost::python::converter::rvalue_from_python_storage<std::string>*)
            data)->storage.bytes;
        new (storage) std::string(boost::python::extract<std::string>(obj_str) BOOST_EXTRACT_WORKAROUND);
        data->convertible = storage;
        Py_DECREF(obj_str);
    }
};

//
// PyTracebackObject*
//
#if 0
struct string_from_python_traceback
{
    string_from_python_traceback()
    {
        boost::python::converter::registry::push_back(
            &convertible,
            &construct,
            boost::python::type_id<std::string>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (PyTraceBack_Check(obj_ptr))
            return obj_ptr;
        return 0;
    }

    static void construct(
        PyObject* obj_ptr,
        boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        namespace py = boost::python;
        py::handle<> obj(py::borrowed(obj_ptr));
        py::object traceback = py::import("traceback");
        py::object formatted = traceback.attr("format_exception")(obj);
        py::object tbcontent = py::str("\n").join(formatted);

        void* storage = (
            (boost::python::converter::rvalue_from_python_storage<std::string>*)
            data)->storage.bytes;
        new (storage) std::string(boost::python::extract<std::string>(tbcontent) BOOST_EXTRACT_WORKAROUND);
        data->convertible = storage;
    }
};
#endif

#endif // pyconvert_h__
