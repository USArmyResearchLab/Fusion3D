/********************************************************************************
** Form generated from reading UI file 'dialogFileDemOther.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGFILEDEMOTHER_H
#define UI_DIALOGFILEDEMOTHER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_dialogFileDemOther
{
public:
    QPushButton *loadButton;
    QPushButton *cancelButton;
    QLineEdit *PrimEdit;
    QLineEdit *secEdit;
    QLineEdit *texEdit;
    QLineEdit *kmlEdit;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QPushButton *primButton;
    QPushButton *secButton;
    QPushButton *texButton;
    QPushButton *kmlButton;

    void setupUi(QDialog *dialogFileDemOther)
    {
        if (dialogFileDemOther->objectName().isEmpty())
            dialogFileDemOther->setObjectName(QString::fromUtf8("dialogFileDemOther"));
        dialogFileDemOther->resize(521, 325);
        loadButton = new QPushButton(dialogFileDemOther);
        loadButton->setObjectName(QString::fromUtf8("loadButton"));
        loadButton->setGeometry(QRect(330, 290, 75, 23));
        cancelButton = new QPushButton(dialogFileDemOther);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(430, 290, 75, 23));
        PrimEdit = new QLineEdit(dialogFileDemOther);
        PrimEdit->setObjectName(QString::fromUtf8("PrimEdit"));
        PrimEdit->setGeometry(QRect(40, 50, 351, 20));
        secEdit = new QLineEdit(dialogFileDemOther);
        secEdit->setObjectName(QString::fromUtf8("secEdit"));
        secEdit->setGeometry(QRect(40, 100, 361, 20));
        texEdit = new QLineEdit(dialogFileDemOther);
        texEdit->setObjectName(QString::fromUtf8("texEdit"));
        texEdit->setGeometry(QRect(40, 170, 361, 20));
        kmlEdit = new QLineEdit(dialogFileDemOther);
        kmlEdit->setObjectName(QString::fromUtf8("kmlEdit"));
        kmlEdit->setGeometry(QRect(40, 240, 361, 20));
        label = new QLabel(dialogFileDemOther);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(40, 30, 271, 16));
        label_2 = new QLabel(dialogFileDemOther);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(40, 80, 281, 16));
        label_3 = new QLabel(dialogFileDemOther);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 150, 141, 16));
        label_4 = new QLabel(dialogFileDemOther);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(40, 220, 121, 16));
        primButton = new QPushButton(dialogFileDemOther);
        primButton->setObjectName(QString::fromUtf8("primButton"));
        primButton->setGeometry(QRect(430, 50, 75, 23));
        secButton = new QPushButton(dialogFileDemOther);
        secButton->setObjectName(QString::fromUtf8("secButton"));
        secButton->setGeometry(QRect(430, 100, 75, 23));
        texButton = new QPushButton(dialogFileDemOther);
        texButton->setObjectName(QString::fromUtf8("texButton"));
        texButton->setGeometry(QRect(430, 170, 75, 23));
        kmlButton = new QPushButton(dialogFileDemOther);
        kmlButton->setObjectName(QString::fromUtf8("kmlButton"));
        kmlButton->setGeometry(QRect(430, 240, 75, 23));

        retranslateUi(dialogFileDemOther);
        QObject::connect(loadButton, SIGNAL(clicked()), dialogFileDemOther, SLOT(doLoad()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogFileDemOther, SLOT(reject()));
        QObject::connect(primButton, SIGNAL(clicked()), dialogFileDemOther, SLOT(doPrimBrowse()));
        QObject::connect(dialogFileDemOther, SIGNAL(windowIconTextChanged(QString)), PrimEdit, SLOT(setText(QString)));
        QObject::connect(texButton, SIGNAL(clicked()), dialogFileDemOther, SLOT(doTexBrowse()));
        QObject::connect(secButton, SIGNAL(clicked()), dialogFileDemOther, SLOT(doSecBrowse()));
        QObject::connect(kmlButton, SIGNAL(clicked()), dialogFileDemOther, SLOT(doKmlBrowse()));

        QMetaObject::connectSlotsByName(dialogFileDemOther);
    } // setupUi

    void retranslateUi(QDialog *dialogFileDemOther)
    {
        dialogFileDemOther->setWindowTitle(QApplication::translate("dialogFileDemOther", "dialogFileDemOther", nullptr));
        loadButton->setText(QApplication::translate("dialogFileDemOther", "Load", nullptr));
        cancelButton->setText(QApplication::translate("dialogFileDemOther", "Cancel", nullptr));
        PrimEdit->setText(QApplication::translate("dialogFileDemOther", "<Required>", nullptr));
        secEdit->setText(QApplication::translate("dialogFileDemOther", "<optional>", nullptr));
        texEdit->setText(QApplication::translate("dialogFileDemOther", "<Required>", nullptr));
        kmlEdit->setText(QApplication::translate("dialogFileDemOther", "<Optional>", nullptr));
        label->setText(QApplication::translate("dialogFileDemOther", "Primary DEM (last-hit 'a2' or bare-earth DEM", nullptr));
        label_2->setText(QApplication::translate("dialogFileDemOther", "Secondary DEM (first-hit 'a1' DEM or DSM over DTM", nullptr));
        label_3->setText(QApplication::translate("dialogFileDemOther", "MrSID or GeoTiff texture", nullptr));
        label_4->setText(QApplication::translate("dialogFileDemOther", "KML extent file", nullptr));
        primButton->setText(QApplication::translate("dialogFileDemOther", "Browse", nullptr));
        secButton->setText(QApplication::translate("dialogFileDemOther", "Browse", nullptr));
        texButton->setText(QApplication::translate("dialogFileDemOther", "Browse", nullptr));
        kmlButton->setText(QApplication::translate("dialogFileDemOther", "Browse", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogFileDemOther: public Ui_dialogFileDemOther {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGFILEDEMOTHER_H
