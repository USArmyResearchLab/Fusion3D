/********************************************************************************
** Form generated from reading UI file 'dialogDrawAlt.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGDRAWALT_H
#define UI_DIALOGDRAWALT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_dialogDrawAlt
{
public:
    QPushButton *okButton;
    QPushButton *cancelButton;
    QRadioButton *absButton;
    QRadioButton *relButton;
    QLineEdit *altEdit;
    QLabel *label;
    QLabel *label_2;

    void setupUi(QDialog *dialogDrawAlt)
    {
        if (dialogDrawAlt->objectName().isEmpty())
            dialogDrawAlt->setObjectName(QString::fromUtf8("dialogDrawAlt"));
        dialogDrawAlt->resize(308, 187);
        okButton = new QPushButton(dialogDrawAlt);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setGeometry(QRect(110, 140, 75, 23));
        cancelButton = new QPushButton(dialogDrawAlt);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(200, 140, 75, 23));
        absButton = new QRadioButton(dialogDrawAlt);
        absButton->setObjectName(QString::fromUtf8("absButton"));
        absButton->setGeometry(QRect(40, 80, 121, 17));
        relButton = new QRadioButton(dialogDrawAlt);
        relButton->setObjectName(QString::fromUtf8("relButton"));
        relButton->setGeometry(QRect(40, 100, 161, 17));
        altEdit = new QLineEdit(dialogDrawAlt);
        altEdit->setObjectName(QString::fromUtf8("altEdit"));
        altEdit->setGeometry(QRect(100, 40, 113, 20));
        label = new QLabel(dialogDrawAlt);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 40, 71, 21));
        label_2 = new QLabel(dialogDrawAlt);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(30, 20, 221, 16));

        retranslateUi(dialogDrawAlt);
        QObject::connect(absButton, SIGNAL(clicked()), dialogDrawAlt, SLOT(doAbsolute()));
        QObject::connect(relButton, SIGNAL(clicked()), dialogDrawAlt, SLOT(doRelative()));
        QObject::connect(okButton, SIGNAL(clicked()), dialogDrawAlt, SLOT(doAccept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogDrawAlt, SLOT(doReject()));

        QMetaObject::connectSlotsByName(dialogDrawAlt);
    } // setupUi

    void retranslateUi(QDialog *dialogDrawAlt)
    {
        dialogDrawAlt->setWindowTitle(QApplication::translate("dialogDrawAlt", "dialogDrawAlt", nullptr));
        okButton->setText(QApplication::translate("dialogDrawAlt", "Ok", nullptr));
        cancelButton->setText(QApplication::translate("dialogDrawAlt", "Cancel", nullptr));
        absButton->setText(QApplication::translate("dialogDrawAlt", "Absolute altitude", nullptr));
        relButton->setText(QApplication::translate("dialogDrawAlt", "Altitude relative to ground", nullptr));
        altEdit->setText(QString());
        label->setText(QApplication::translate("dialogDrawAlt", "Altitude (m)", nullptr));
        label_2->setText(QApplication::translate("dialogDrawAlt", "Entries apply only to the current line or point", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogDrawAlt: public Ui_dialogDrawAlt {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGDRAWALT_H
