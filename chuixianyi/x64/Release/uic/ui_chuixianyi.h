/********************************************************************************
** Form generated from reading UI file 'chuixianyi.ui'
**
** Created by: Qt User Interface Compiler version 5.9.9
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHUIXIANYI_H
#define UI_CHUIXIANYI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_chuixianyiClass
{
public:
    QWidget *centralWidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *startCameraBtn;
    QPushButton *saveFrameBtn;
    QPushButton *endCameraBtn;
    QPushButton *calibrateBtn;
    QPushButton *rectifyBtn;
    QPushButton *measureBtn;
    QPushButton *autoMatchBtn;
    QPushButton *detectCrossBtn;
    QPushButton *planeCalibrateBtn;
    QPushButton *liveShowCoordinateBtn;
    QSpacerItem *verticalSpacer;
    QPushButton *exitBtn;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *labelLeft;
    QLabel *labelRight;
    QTextBrowser *textBrowser;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *chuixianyiClass)
    {
        if (chuixianyiClass->objectName().isEmpty())
            chuixianyiClass->setObjectName(QStringLiteral("chuixianyiClass"));
        chuixianyiClass->resize(1177, 602);
        centralWidget = new QWidget(chuixianyiClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayoutWidget = new QWidget(centralWidget);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(1010, -1, 191, 541));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        startCameraBtn = new QPushButton(verticalLayoutWidget);
        startCameraBtn->setObjectName(QStringLiteral("startCameraBtn"));

        verticalLayout->addWidget(startCameraBtn);

        saveFrameBtn = new QPushButton(verticalLayoutWidget);
        saveFrameBtn->setObjectName(QStringLiteral("saveFrameBtn"));

        verticalLayout->addWidget(saveFrameBtn);

        endCameraBtn = new QPushButton(verticalLayoutWidget);
        endCameraBtn->setObjectName(QStringLiteral("endCameraBtn"));

        verticalLayout->addWidget(endCameraBtn);

        calibrateBtn = new QPushButton(verticalLayoutWidget);
        calibrateBtn->setObjectName(QStringLiteral("calibrateBtn"));

        verticalLayout->addWidget(calibrateBtn);

        rectifyBtn = new QPushButton(verticalLayoutWidget);
        rectifyBtn->setObjectName(QStringLiteral("rectifyBtn"));

        verticalLayout->addWidget(rectifyBtn);

        measureBtn = new QPushButton(verticalLayoutWidget);
        measureBtn->setObjectName(QStringLiteral("measureBtn"));

        verticalLayout->addWidget(measureBtn);

        autoMatchBtn = new QPushButton(verticalLayoutWidget);
        autoMatchBtn->setObjectName(QStringLiteral("autoMatchBtn"));

        verticalLayout->addWidget(autoMatchBtn);

        detectCrossBtn = new QPushButton(verticalLayoutWidget);
        detectCrossBtn->setObjectName(QStringLiteral("detectCrossBtn"));

        verticalLayout->addWidget(detectCrossBtn);

        planeCalibrateBtn = new QPushButton(verticalLayoutWidget);
        planeCalibrateBtn->setObjectName(QStringLiteral("planeCalibrateBtn"));

        verticalLayout->addWidget(planeCalibrateBtn);

        liveShowCoordinateBtn = new QPushButton(verticalLayoutWidget);
        liveShowCoordinateBtn->setObjectName(QStringLiteral("liveShowCoordinateBtn"));

        verticalLayout->addWidget(liveShowCoordinateBtn);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        exitBtn = new QPushButton(verticalLayoutWidget);
        exitBtn->setObjectName(QStringLiteral("exitBtn"));

        verticalLayout->addWidget(exitBtn);

        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(-8, 0, 1021, 401));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        labelLeft = new QLabel(layoutWidget);
        labelLeft->setObjectName(QStringLiteral("labelLeft"));
        labelLeft->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(labelLeft);

        labelRight = new QLabel(layoutWidget);
        labelRight->setObjectName(QStringLiteral("labelRight"));
        labelRight->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(labelRight);

        textBrowser = new QTextBrowser(centralWidget);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        textBrowser->setGeometry(QRect(-5, 401, 1011, 141));
        chuixianyiClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(chuixianyiClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1177, 23));
        chuixianyiClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(chuixianyiClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        chuixianyiClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(chuixianyiClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        chuixianyiClass->setStatusBar(statusBar);

        retranslateUi(chuixianyiClass);

        QMetaObject::connectSlotsByName(chuixianyiClass);
    } // setupUi

    void retranslateUi(QMainWindow *chuixianyiClass)
    {
        chuixianyiClass->setWindowTitle(QApplication::translate("chuixianyiClass", "chuixianyi", Q_NULLPTR));
        startCameraBtn->setText(QApplication::translate("chuixianyiClass", "\345\274\200\345\247\213\346\213\215\346\221\204", Q_NULLPTR));
        saveFrameBtn->setText(QApplication::translate("chuixianyiClass", "\344\277\235\345\255\230\345\275\223\345\211\215\345\270\247", Q_NULLPTR));
        endCameraBtn->setText(QApplication::translate("chuixianyiClass", "\347\273\223\346\235\237\346\213\215\346\221\204", Q_NULLPTR));
        calibrateBtn->setText(QApplication::translate("chuixianyiClass", "\346\240\207\345\256\232\347\233\270\346\234\272", Q_NULLPTR));
        rectifyBtn->setText(QApplication::translate("chuixianyiClass", "\346\240\241\346\255\243\345\233\276\345\203\217", Q_NULLPTR));
        measureBtn->setText(QApplication::translate("chuixianyiClass", "\351\200\211\347\202\271\346\265\213\351\207\217", Q_NULLPTR));
        autoMatchBtn->setText(QApplication::translate("chuixianyiClass", "\350\207\252\345\212\250\345\214\271\351\205\215", Q_NULLPTR));
        detectCrossBtn->setText(QApplication::translate("chuixianyiClass", "\346\243\200\346\265\213\344\272\244\347\202\271", Q_NULLPTR));
        planeCalibrateBtn->setText(QApplication::translate("chuixianyiClass", "\345\205\211\345\271\263\351\235\242\346\240\207\345\256\232", Q_NULLPTR));
        liveShowCoordinateBtn->setText(QApplication::translate("chuixianyiClass", "\345\256\236\346\227\266\346\230\276\347\244\272\345\236\202\347\272\277\345\235\220\346\240\207", Q_NULLPTR));
        exitBtn->setText(QApplication::translate("chuixianyiClass", "\351\200\200\345\207\272", Q_NULLPTR));
        labelLeft->setText(QApplication::translate("chuixianyiClass", "\345\267\246\346\221\204\345\203\217\345\244\264", Q_NULLPTR));
        labelRight->setText(QApplication::translate("chuixianyiClass", "\345\217\263\346\221\204\345\203\217\345\244\264", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class chuixianyiClass: public Ui_chuixianyiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHUIXIANYI_H
