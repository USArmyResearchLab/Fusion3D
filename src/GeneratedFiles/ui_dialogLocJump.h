/********************************************************************************
** Form generated from reading UI file 'dialogLocJump.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGLOCJUMP_H
#define UI_DIALOGLOCJUMP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_dialogLocJump
{
public:
    QRadioButton *mgrsButton;
    QRadioButton *utmButton;
    QRadioButton *latButton;
    QLabel *label;
    QPushButton *cancelButton;
    QLineEdit *longEdit;
    QLineEdit *latEdit;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *northEdit;
    QLineEdit *eastEdit;
    QLineEdit *mgrsnEdit;
    QLineEdit *mgrseEdit;
    QLineEdit *mgrsxEdit;
    QLineEdit *mgrsZoneLatEdit;
    QLineEdit *utmZoneLatEdit;
    QPushButton *JumpButton;
    QLineEdit *mgrsZonLonEdit;
    QLineEdit *utmZoneLonEdit;
    QLabel *label_4;
    QLabel *label_5;

    void setupUi(QDialog *dialogLocJump)
    {
        if (dialogLocJump->objectName().isEmpty())
            dialogLocJump->setObjectName(QString::fromUtf8("dialogLocJump"));
        dialogLocJump->resize(537, 161);
        mgrsButton = new QRadioButton(dialogLocJump);
        mgrsButton->setObjectName(QString::fromUtf8("mgrsButton"));
        mgrsButton->setGeometry(QRect(20, 40, 81, 17));
        utmButton = new QRadioButton(dialogLocJump);
        utmButton->setObjectName(QString::fromUtf8("utmButton"));
        utmButton->setGeometry(QRect(20, 70, 81, 17));
        latButton = new QRadioButton(dialogLocJump);
        latButton->setObjectName(QString::fromUtf8("latButton"));
        latButton->setGeometry(QRect(20, 100, 82, 17));
        label = new QLabel(dialogLocJump);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(60, 10, 301, 16));
        cancelButton = new QPushButton(dialogLocJump);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(450, 130, 75, 23));
        longEdit = new QLineEdit(dialogLocJump);
        longEdit->setObjectName(QString::fromUtf8("longEdit"));
        longEdit->setGeometry(QRect(432, 100, 91, 20));
        latEdit = new QLineEdit(dialogLocJump);
        latEdit->setObjectName(QString::fromUtf8("latEdit"));
        latEdit->setGeometry(QRect(282, 100, 91, 20));
        label_2 = new QLabel(dialogLocJump);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(230, 100, 41, 20));
        label_3 = new QLabel(dialogLocJump);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(380, 100, 47, 13));
        northEdit = new QLineEdit(dialogLocJump);
        northEdit->setObjectName(QString::fromUtf8("northEdit"));
        northEdit->setGeometry(QRect(280, 70, 91, 20));
        eastEdit = new QLineEdit(dialogLocJump);
        eastEdit->setObjectName(QString::fromUtf8("eastEdit"));
        eastEdit->setGeometry(QRect(430, 70, 91, 20));
        mgrsnEdit = new QLineEdit(dialogLocJump);
        mgrsnEdit->setObjectName(QString::fromUtf8("mgrsnEdit"));
        mgrsnEdit->setGeometry(QRect(280, 40, 91, 20));
        mgrseEdit = new QLineEdit(dialogLocJump);
        mgrseEdit->setObjectName(QString::fromUtf8("mgrseEdit"));
        mgrseEdit->setGeometry(QRect(430, 40, 91, 20));
        mgrsxEdit = new QLineEdit(dialogLocJump);
        mgrsxEdit->setObjectName(QString::fromUtf8("mgrsxEdit"));
        mgrsxEdit->setGeometry(QRect(212, 40, 41, 20));
        mgrsZoneLatEdit = new QLineEdit(dialogLocJump);
        mgrsZoneLatEdit->setObjectName(QString::fromUtf8("mgrsZoneLatEdit"));
        mgrsZoneLatEdit->setGeometry(QRect(172, 40, 21, 20));
        utmZoneLatEdit = new QLineEdit(dialogLocJump);
        utmZoneLatEdit->setObjectName(QString::fromUtf8("utmZoneLatEdit"));
        utmZoneLatEdit->setGeometry(QRect(170, 70, 21, 20));
        JumpButton = new QPushButton(dialogLocJump);
        JumpButton->setObjectName(QString::fromUtf8("JumpButton"));
        JumpButton->setGeometry(QRect(350, 130, 75, 23));
        mgrsZonLonEdit = new QLineEdit(dialogLocJump);
        mgrsZonLonEdit->setObjectName(QString::fromUtf8("mgrsZonLonEdit"));
        mgrsZonLonEdit->setGeometry(QRect(132, 40, 21, 20));
        utmZoneLonEdit = new QLineEdit(dialogLocJump);
        utmZoneLonEdit->setObjectName(QString::fromUtf8("utmZoneLonEdit"));
        utmZoneLonEdit->setGeometry(QRect(133, 70, 20, 20));
        label_4 = new QLabel(dialogLocJump);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(230, 70, 41, 16));
        label_5 = new QLabel(dialogLocJump);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(380, 70, 47, 13));

        retranslateUi(dialogLocJump);
        QObject::connect(mgrsButton, SIGNAL(clicked()), dialogLocJump, SLOT(doMgrs()));
        QObject::connect(utmButton, SIGNAL(clicked()), dialogLocJump, SLOT(doUtm()));
        QObject::connect(latButton, SIGNAL(clicked()), dialogLocJump, SLOT(doLatLon()));
        QObject::connect(JumpButton, SIGNAL(clicked()), dialogLocJump, SLOT(doJump()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogLocJump, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogLocJump);
    } // setupUi

    void retranslateUi(QDialog *dialogLocJump)
    {
        dialogLocJump->setWindowTitle(QApplication::translate("dialogLocJump", "dialogLocJump", nullptr));
        mgrsButton->setText(QApplication::translate("dialogLocJump", "Jump MGRS", nullptr));
        utmButton->setText(QApplication::translate("dialogLocJump", "Jump UTM", nullptr));
        latButton->setText(QApplication::translate("dialogLocJump", "Jump LatLon", nullptr));
        label->setText(QApplication::translate("dialogLocJump", "Jump to location / Read location of screen center", nullptr));
        cancelButton->setText(QApplication::translate("dialogLocJump", "Cancel", nullptr));
        label_2->setText(QApplication::translate("dialogLocJump", "Latitude", nullptr));
        label_3->setText(QApplication::translate("dialogLocJump", "Longitude", nullptr));
        JumpButton->setText(QApplication::translate("dialogLocJump", "Jump", nullptr));
        label_4->setText(QApplication::translate("dialogLocJump", "Northing", nullptr));
        label_5->setText(QApplication::translate("dialogLocJump", "Easting", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogLocJump: public Ui_dialogLocJump {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGLOCJUMP_H
