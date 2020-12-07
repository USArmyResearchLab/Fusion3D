/********************************************************************************
** Form generated from reading UI file 'dialogMeasureRoute.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGMEASUREROUTE_H
#define UI_DIALOGMEASUREROUTE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_dialogMeasureRoute
{
public:
    QLabel *label;
    QRadioButton *firstButton;
    QRadioButton *secondButton;
    QRadioButton *calcButton;
    QRadioButton *offButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *dialogMeasureRoute)
    {
        if (dialogMeasureRoute->objectName().isEmpty())
            dialogMeasureRoute->setObjectName(QString::fromUtf8("dialogMeasureRoute"));
        dialogMeasureRoute->resize(326, 168);
        label = new QLabel(dialogMeasureRoute);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 291, 16));
        firstButton = new QRadioButton(dialogMeasureRoute);
        firstButton->setObjectName(QString::fromUtf8("firstButton"));
        firstButton->setGeometry(QRect(40, 40, 241, 17));
        secondButton = new QRadioButton(dialogMeasureRoute);
        secondButton->setObjectName(QString::fromUtf8("secondButton"));
        secondButton->setGeometry(QRect(40, 60, 221, 17));
        calcButton = new QRadioButton(dialogMeasureRoute);
        calcButton->setObjectName(QString::fromUtf8("calcButton"));
        calcButton->setGeometry(QRect(40, 80, 241, 17));
        offButton = new QRadioButton(dialogMeasureRoute);
        offButton->setObjectName(QString::fromUtf8("offButton"));
        offButton->setGeometry(QRect(40, 100, 181, 17));
        cancelButton = new QPushButton(dialogMeasureRoute);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(230, 130, 75, 23));

        retranslateUi(dialogMeasureRoute);
        QObject::connect(firstButton, SIGNAL(clicked()), dialogMeasureRoute, SLOT(doFirst()));
        QObject::connect(secondButton, SIGNAL(clicked()), dialogMeasureRoute, SLOT(doSecond()));
        QObject::connect(calcButton, SIGNAL(clicked()), dialogMeasureRoute, SLOT(doCalc()));
        QObject::connect(offButton, SIGNAL(clicked()), dialogMeasureRoute, SLOT(doClear()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogMeasureRoute, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogMeasureRoute);
    } // setupUi

    void retranslateUi(QDialog *dialogMeasureRoute)
    {
        dialogMeasureRoute->setWindowTitle(QApplication::translate("dialogMeasureRoute", "dialogMeasureRoute", nullptr));
        label->setText(QApplication::translate("dialogMeasureRoute", "Calculate distance along a route between chosen endpoints", nullptr));
        firstButton->setText(QApplication::translate("dialogMeasureRoute", "Pick first route endpoint (middle mouse)", nullptr));
        secondButton->setText(QApplication::translate("dialogMeasureRoute", "Pick second route endpoint", nullptr));
        calcButton->setText(QApplication::translate("dialogMeasureRoute", "Calculate distance between two chosen points", nullptr));
        offButton->setText(QApplication::translate("dialogMeasureRoute", "Turn off anc clear", nullptr));
        cancelButton->setText(QApplication::translate("dialogMeasureRoute", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogMeasureRoute: public Ui_dialogMeasureRoute {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGMEASUREROUTE_H
