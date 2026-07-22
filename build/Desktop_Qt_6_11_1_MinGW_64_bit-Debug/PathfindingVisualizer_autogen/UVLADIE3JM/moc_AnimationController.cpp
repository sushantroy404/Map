/****************************************************************************
** Meta object code from reading C++ file 'AnimationController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/AnimationController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AnimationController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN19AnimationControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto AnimationController::qt_create_metaobjectdata<qt_meta_tag_ZN19AnimationControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AnimationController",
        "stepRendered",
        "",
        "PathfindingStep",
        "step",
        "comparisonStepsRendered",
        "dijkstraStep",
        "astarStep",
        "bfsStep",
        "processNextStep"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stepRendered'
        QtMocHelpers::SignalData<void(const PathfindingStep &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'comparisonStepsRendered'
        QtMocHelpers::SignalData<void(const PathfindingStep &, const PathfindingStep &, const PathfindingStep &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 6 }, { 0x80000000 | 3, 7 }, { 0x80000000 | 3, 8 },
        }}),
        // Slot 'processNextStep'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AnimationController, qt_meta_tag_ZN19AnimationControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AnimationController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AnimationControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AnimationControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19AnimationControllerE_t>.metaTypes,
    nullptr
} };

void AnimationController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AnimationController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stepRendered((*reinterpret_cast<std::add_pointer_t<PathfindingStep>>(_a[1]))); break;
        case 1: _t->comparisonStepsRendered((*reinterpret_cast<std::add_pointer_t<PathfindingStep>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<PathfindingStep>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<PathfindingStep>>(_a[3]))); break;
        case 2: _t->processNextStep(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(const PathfindingStep & )>(_a, &AnimationController::stepRendered, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(const PathfindingStep & , const PathfindingStep & , const PathfindingStep & )>(_a, &AnimationController::comparisonStepsRendered, 1))
            return;
    }
}

const QMetaObject *AnimationController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AnimationController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AnimationControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AnimationController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void AnimationController::stepRendered(const PathfindingStep & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AnimationController::comparisonStepsRendered(const PathfindingStep & _t1, const PathfindingStep & _t2, const PathfindingStep & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
