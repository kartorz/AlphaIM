/****************************************************************************
** Meta object code from reading C++ file 'aimwin.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "aimwin.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'aimwin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_AimWin_t {
    QByteArrayData data[23];
    char stringdata0[315];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AimWin_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AimWin_t qt_meta_stringdata_AimWin = {
    {
QT_MOC_LITERAL(0, 0, 6), // "AimWin"
QT_MOC_LITERAL(1, 7, 24), // "on_lanToolButton_clicked"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 24), // "on_punToolButton_clicked"
QT_MOC_LITERAL(4, 58, 25), // "on_helpToolButton_clicked"
QT_MOC_LITERAL(5, 84, 18), // "onSystrayActivated"
QT_MOC_LITERAL(6, 103, 33), // "QSystemTrayIcon::ActivationRe..."
QT_MOC_LITERAL(7, 137, 6), // "reason"
QT_MOC_LITERAL(8, 144, 15), // "onMesssagerIMOn"
QT_MOC_LITERAL(9, 160, 16), // "onMesssagerIMOff"
QT_MOC_LITERAL(10, 177, 18), // "onMesssagerIMClose"
QT_MOC_LITERAL(11, 196, 19), // "onMesssagerIMCommit"
QT_MOC_LITERAL(12, 216, 16), // "onMesssagerIMLan"
QT_MOC_LITERAL(13, 233, 4), // "isCN"
QT_MOC_LITERAL(14, 238, 16), // "onMesssagerIMPun"
QT_MOC_LITERAL(15, 255, 20), // "onMesssagerIMPreedit"
QT_MOC_LITERAL(16, 276, 1), // "x"
QT_MOC_LITERAL(17, 278, 1), // "y"
QT_MOC_LITERAL(18, 280, 1), // "w"
QT_MOC_LITERAL(19, 282, 1), // "h"
QT_MOC_LITERAL(20, 284, 11), // "std::string"
QT_MOC_LITERAL(21, 296, 8), // "strInput"
QT_MOC_LITERAL(22, 305, 9) // "user_data"

    },
    "AimWin\0on_lanToolButton_clicked\0\0"
    "on_punToolButton_clicked\0"
    "on_helpToolButton_clicked\0onSystrayActivated\0"
    "QSystemTrayIcon::ActivationReason\0"
    "reason\0onMesssagerIMOn\0onMesssagerIMOff\0"
    "onMesssagerIMClose\0onMesssagerIMCommit\0"
    "onMesssagerIMLan\0isCN\0onMesssagerIMPun\0"
    "onMesssagerIMPreedit\0x\0y\0w\0h\0std::string\0"
    "strInput\0user_data"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AimWin[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    0,   71,    2, 0x08 /* Private */,
       5,    1,   72,    2, 0x08 /* Private */,
       8,    0,   75,    2, 0x08 /* Private */,
       9,    0,   76,    2, 0x08 /* Private */,
      10,    0,   77,    2, 0x08 /* Private */,
      11,    0,   78,    2, 0x08 /* Private */,
      12,    1,   79,    2, 0x08 /* Private */,
      14,    1,   82,    2, 0x08 /* Private */,
      15,    6,   85,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, 0x80000000 | 20, QMetaType::VoidStar,   16,   17,   18,   19,   21,   22,

       0        // eod
};

void AimWin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AimWin *_t = static_cast<AimWin *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_lanToolButton_clicked(); break;
        case 1: _t->on_punToolButton_clicked(); break;
        case 2: _t->on_helpToolButton_clicked(); break;
        case 3: _t->onSystrayActivated((*reinterpret_cast< QSystemTrayIcon::ActivationReason(*)>(_a[1]))); break;
        case 4: _t->onMesssagerIMOn(); break;
        case 5: _t->onMesssagerIMOff(); break;
        case 6: _t->onMesssagerIMClose(); break;
        case 7: _t->onMesssagerIMCommit(); break;
        case 8: _t->onMesssagerIMLan((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->onMesssagerIMPun((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->onMesssagerIMPreedit((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< std::string(*)>(_a[5])),(*reinterpret_cast< void*(*)>(_a[6]))); break;
        default: ;
        }
    }
}

const QMetaObject AimWin::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_AimWin.data,
      qt_meta_data_AimWin,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *AimWin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AimWin::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_AimWin.stringdata0))
        return static_cast<void*>(const_cast< AimWin*>(this));
    return QDialog::qt_metacast(_clname);
}

int AimWin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
