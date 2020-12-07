/********************************************************************************
** Form generated from reading UI file 'dialogFileDemBuck.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGFILEDEMBUCK_H
#define UI_DIALOGFILEDEMBUCK_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_dialogFileDemBuck
{
public:
    QLineEdit *dirEdit;
    QLabel *label;
    QPushButton *dirButton;
    QLabel *label_2;
    QLabel *label_3;
    QRadioButton *buckeyeButton;
    QRadioButton *alirtButton;
    QRadioButton *haloeButton;
    QLabel *label_4;
    QRadioButton *egm96Button;
    QRadioButton *wgs84Button;
    QLineEdit *out1Edit;
    QLineEdit *out2Edit;
    QLineEdit *out3Edit;
    QLineEdit *out4Edit;
    QLineEdit *out5Edit;
    QPushButton *loadButton;
    QPushButton *cancelButton;

    void setupUi(QWidget *dialogFileDemBuck)
    {
        if (dialogFileDemBuck->objectName().isEmpty())
            dialogFileDemBuck->setObjectName(QString::fromUtf8("dialogFileDemBuck"));
        dialogFileDemBuck->resize(529, 451);
        dirEdit = new QLineEdit(dialogFileDemBuck);
        dirEdit->setObjectName(QString::fromUtf8("dirEdit"));
        dirEdit->setGeometry(QRect(120, 70, 311, 20));
        label = new QLabel(dialogFileDemBuck);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 70, 91, 16));
        dirButton = new QPushButton(dialogFileDemBuck);
        dirButton->setObjectName(QString::fromUtf8("dirButton"));
        dirButton->setGeometry(QRect(440, 70, 75, 23));
        label_2 = new QLabel(dialogFileDemBuck);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 40, 351, 16));
        label_3 = new QLabel(dialogFileDemBuck);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 100, 201, 16));
        buckeyeButton = new QRadioButton(dialogFileDemBuck);
        buckeyeButton->setObjectName(QString::fromUtf8("buckeyeButton"));
        buckeyeButton->setGeometry(QRect(70, 130, 341, 17));
        alirtButton = new QRadioButton(dialogFileDemBuck);
        alirtButton->setObjectName(QString::fromUtf8("alirtButton"));
        alirtButton->setGeometry(QRect(70, 150, 321, 17));
        haloeButton = new QRadioButton(dialogFileDemBuck);
        haloeButton->setObjectName(QString::fromUtf8("haloeButton"));
        haloeButton->setGeometry(QRect(70, 170, 311, 17));
        label_4 = new QLabel(dialogFileDemBuck);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(120, 190, 401, 16));
        egm96Button = new QRadioButton(dialogFileDemBuck);
        egm96Button->setObjectName(QString::fromUtf8("egm96Button"));
        egm96Button->setGeometry(QRect(120, 210, 82, 17));
        wgs84Button = new QRadioButton(dialogFileDemBuck);
        wgs84Button->setObjectName(QString::fromUtf8("wgs84Button"));
        wgs84Button->setGeometry(QRect(120, 230, 82, 17));
        out1Edit = new QLineEdit(dialogFileDemBuck);
        out1Edit->setObjectName(QString::fromUtf8("out1Edit"));
        out1Edit->setGeometry(QRect(30, 260, 481, 20));
        out2Edit = new QLineEdit(dialogFileDemBuck);
        out2Edit->setObjectName(QString::fromUtf8("out2Edit"));
        out2Edit->setGeometry(QRect(30, 290, 481, 20));
        out3Edit = new QLineEdit(dialogFileDemBuck);
        out3Edit->setObjectName(QString::fromUtf8("out3Edit"));
        out3Edit->setGeometry(QRect(30, 320, 481, 20));
        out4Edit = new QLineEdit(dialogFileDemBuck);
        out4Edit->setObjectName(QString::fromUtf8("out4Edit"));
        out4Edit->setGeometry(QRect(30, 350, 481, 20));
        out5Edit = new QLineEdit(dialogFileDemBuck);
        out5Edit->setObjectName(QString::fromUtf8("out5Edit"));
        out5Edit->setGeometry(QRect(30, 380, 481, 20));
        loadButton = new QPushButton(dialogFileDemBuck);
        loadButton->setObjectName(QString::fromUtf8("loadButton"));
        loadButton->setGeometry(QRect(330, 420, 75, 23));
        cancelButton = new QPushButton(dialogFileDemBuck);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(440, 420, 75, 23));

        retranslateUi(dialogFileDemBuck);
        QObject::connect(dirButton, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doDirBrowse()));
        QObject::connect(buckeyeButton, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doBuckeye()));
        QObject::connect(alirtButton, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doAlirt()));
        QObject::connect(haloeButton, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doHaloe()));
        QObject::connect(egm96Button, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doEgm96()));
        QObject::connect(wgs84Button, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doWgs84()));
        QObject::connect(loadButton, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doLoad()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogFileDemBuck, SLOT(doCancel()));

        QMetaObject::connectSlotsByName(dialogFileDemBuck);
    } // setupUi

    void retranslateUi(QWidget *dialogFileDemBuck)
    {
        dialogFileDemBuck->setWindowTitle(QApplication::translate("dialogFileDemBuck", "dialogFileDemBuck", nullptr));
        label->setText(QApplication::translate("dialogFileDemBuck", "Choose directory", nullptr));
        dirButton->setText(QApplication::translate("dialogFileDemBuck", "Browse", nullptr));
        label_2->setText(QApplication::translate("dialogFileDemBuck", "Load all primary DEMs and associated files from a directory", nullptr));
        label_3->setText(QApplication::translate("dialogFileDemBuck", "Choose only 1 of the following sensors", nullptr));
        buckeyeButton->setText(QApplication::translate("dialogFileDemBuck", "BuckEye Primary, secondary DEMs, MrSID or lidar texture files", nullptr));
        alirtButton->setText(QApplication::translate("dialogFileDemBuck", "ALIRT -- Primary DEMs, MrSID or lidar intensity texture files", nullptr));
        haloeButton->setText(QApplication::translate("dialogFileDemBuck", "Haloe -- Primary DEMs, MrSID of lidar intensity texture files", nullptr));
        label_4->setText(QApplication::translate("dialogFileDemBuck", "Haloe only -- Choose vertical datum (typically duplicate files with different datums)", nullptr));
        egm96Button->setText(QApplication::translate("dialogFileDemBuck", "EGM96", nullptr));
        wgs84Button->setText(QApplication::translate("dialogFileDemBuck", "WGS84", nullptr));
        loadButton->setText(QApplication::translate("dialogFileDemBuck", "Load", nullptr));
        cancelButton->setText(QApplication::translate("dialogFileDemBuck", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogFileDemBuck: public Ui_dialogFileDemBuck {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGFILEDEMBUCK_H
