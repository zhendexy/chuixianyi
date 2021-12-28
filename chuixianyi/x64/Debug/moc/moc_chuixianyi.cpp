/****************************************************************************
** Meta object code from reading C++ file 'chuixianyi.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../chuixianyi.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'chuixianyi.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_chuixianyi_t {
    QByteArrayData data[13];
    char stringdata0[270];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_chuixianyi_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_chuixianyi_t qt_meta_stringdata_chuixianyi = {
    {
QT_MOC_LITERAL(0, 0, 10), // "chuixianyi"
QT_MOC_LITERAL(1, 11, 25), // "on_startCameraBtn_clicked"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 23), // "on_endCameraBtn_clicked"
QT_MOC_LITERAL(4, 62, 23), // "on_saveFrameBtn_clicked"
QT_MOC_LITERAL(5, 86, 23), // "on_calibrateBtn_clicked"
QT_MOC_LITERAL(6, 110, 21), // "on_rectifyBtn_clicked"
QT_MOC_LITERAL(7, 132, 21), // "on_measureBtn_clicked"
QT_MOC_LITERAL(8, 154, 23), // "on_autoMatchBtn_clicked"
QT_MOC_LITERAL(9, 178, 25), // "on_detectCrossBtn_clicked"
QT_MOC_LITERAL(10, 204, 25), // "on_planeCalibrate_clicked"
QT_MOC_LITERAL(11, 230, 29), // "on_liveShowCoordinate_clicked"
QT_MOC_LITERAL(12, 260, 9) // "readFrame"

    },
    "chuixianyi\0on_startCameraBtn_clicked\0"
    "\0on_endCameraBtn_clicked\0"
    "on_saveFrameBtn_clicked\0on_calibrateBtn_clicked\0"
    "on_rectifyBtn_clicked\0on_measureBtn_clicked\0"
    "on_autoMatchBtn_clicked\0"
    "on_detectCrossBtn_clicked\0"
    "on_planeCalibrate_clicked\0"
    "on_liveShowCoordinate_clicked\0readFrame"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_chuixianyi[] = {

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
       5,    0,   72,    2, 0x08 /* Private */,
       6,    0,   73,    2, 0x08 /* Private */,
       7,    0,   74,    2, 0x08 /* Private */,
       8,    0,   75,    2, 0x08 /* Private */,
       9,    0,   76,    2, 0x08 /* Private */,
      10,    0,   77,    2, 0x08 /* Private */,
      11,    0,   78,    2, 0x08 /* Private */,
      12,    0,   79,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void chuixianyi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        chuixianyi *_t = static_cast<chuixianyi *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_startCameraBtn_clicked(); break;
        case 1: _t->on_endCameraBtn_clicked(); break;
        case 2: _t->on_saveFrameBtn_clicked(); break;
        case 3: _t->on_calibrateBtn_clicked(); break;
        case 4: _t->on_rectifyBtn_clicked(); break;
        case 5: _t->on_measureBtn_clicked(); break;
        case 6: _t->on_autoMatchBtn_clicked(); break;
        case 7: _t->on_detectCrossBtn_clicked(); break;
        case 8: _t->on_planeCalibrate_clicked(); break;
        case 9: _t->on_liveShowCoordinate_clicked(); break;
        case 10: _t->readFrame(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject chuixianyi::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_chuixianyi.data,
      qt_meta_data_chuixianyi,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *chuixianyi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *chuixianyi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_chuixianyi.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int chuixianyi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
