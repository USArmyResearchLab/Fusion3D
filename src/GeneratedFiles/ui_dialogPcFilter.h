/********************************************************************************
** Form generated from reading UI file 'dialogPcFilter.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGPCFILTER_H
#define UI_DIALOGPCFILTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_dialogPcFilter
{
public:
    QLabel *label;
    QRadioButton *noneButton;
    QRadioButton *fineButton;
    QRadioButton *elevAbsButton;
    QRadioButton *elevRelButton;
    QLineEdit *minEdit;
    QLineEdit *maxEdit;
    QPushButton *applyButton;
    QPushButton *cancelButton;
    QLabel *label_2;
    QLabel *label_3;

    void setupUi(QDialog *dialogPcFilter)
    {
        if (dialogPcFilter->objectName().isEmpty())
            dialogPcFilter->setObjectName(QString::fromUtf8("dialogPcFilter"));
        dialogPcFilter->resize(274, 229);
        label = new QLabel(dialogPcFilter);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 121, 16));
        noneButton = new QRadioButton(dialogPcFilter);
        noneButton->setObjectName(QString::fromUtf8("noneButton"));
        noneButton->setGeometry(QRect(40, 30, 82, 17));
        fineButton = new QRadioButton(dialogPcFilter);
        fineButton->setObjectName(QString::fromUtf8("fineButton"));
        fineButton->setGeometry(QRect(40, 50, 231, 17));
        elevAbsButton = new QRadioButton(dialogPcFilter);
        elevAbsButton->setObjectName(QString::fromUtf8("elevAbsButton"));
        elevAbsButton->setGeometry(QRect(40, 70, 211, 17));
        elevRelButton = new QRadioButton(dialogPcFilter);
        elevRelButton->setObjectName(QString::fromUtf8("elevRelButton"));
        elevRelButton->setGeometry(QRect(40, 90, 191, 17));
        minEdit = new QLineEdit(dialogPcFilter);
        minEdit->setObjectName(QString::fromUtf8("minEdit"));
        minEdit->setGeometry(QRect(170, 120, 71, 20));
        maxEdit = new QLineEdit(dialogPcFilter);
        maxEdit->setObjectName(QString::fromUtf8("maxEdit"));
        maxEdit->setGeometry(QRect(170, 150, 71, 20));
        applyButton = new QPushButton(dialogPcFilter);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setGeometry(QRect(80, 190, 75, 23));
        cancelButton = new QPushButton(dialogPcFilter);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(180, 190, 75, 23));
        label_2 = new QLabel(dialogPcFilter);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(30, 120, 131, 16));
        label_3 = new QLabel(dialogPcFilter);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 150, 131, 16));

        retranslateUi(dialogPcFilter);
        QObject::connect(noneButton, SIGNAL(clicked()), dialogPcFilter, SLOT(doNone()));
        QObject::connect(fineButton, SIGNAL(clicked()), dialogPcFilter, SLOT(doFine()));
        QObject::connect(elevAbsButton, SIGNAL(clicked()), dialogPcFilter, SLOT(doElevAbs()));
        QObject::connect(elevRelButton, SIGNAL(clicked()), dialogPcFilter, SLOT(doElevRel()));
        QObject::connect(applyButton, SIGNAL(clicked()), dialogPcFilter, SLOT(doApply()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogPcFilter, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogPcFilter);
    } // setupUi

    void retranslateUi(QDialog *dialogPcFilter)
    {
        dialogPcFilter->setWindowTitle(QApplication::translate("dialogPcFilter", "dialogPcFilter", nullptr));
        label->setText(QApplication::translate("dialogPcFilter", "Filter type", nullptr));
        noneButton->setText(QApplication::translate("dialogPcFilter", "None", nullptr));
        fineButton->setText(QApplication::translate("dialogPcFilter", "Filter on FINE algorithm quality metirc TAU", nullptr));
        elevAbsButton->setText(QApplication::translate("dialogPcFilter", "Filter on elevation", nullptr));
        elevRelButton->setText(QApplication::translate("dialogPcFilter", "Filter on elevation above DEM", nullptr));
        applyButton->setText(QApplication::translate("dialogPcFilter", "Apply", nullptr));
        cancelButton->setText(QApplication::translate("dialogPcFilter", "Cancel", nullptr));
        label_2->setText(QApplication::translate("dialogPcFilter", "Min filter value to process", nullptr));
        label_3->setText(QApplication::translate("dialogPcFilter", "Max filter value to process", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogPcFilter: public Ui_dialogPcFilter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGPCFILTER_H
