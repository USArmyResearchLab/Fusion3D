/********************************************************************************
** Form generated from reading UI file 'dialogClockTime.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGCLOCKTIME_H
#define UI_DIALOGCLOCKTIME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_dialogClockTime
{
public:
    QLabel *label;
    QSlider *timeSlider;
    QLineEdit *timeEdit;
    QPushButton *cancelButton;

    void setupUi(QDialog *dialogClockTime)
    {
        if (dialogClockTime->objectName().isEmpty())
            dialogClockTime->setObjectName(QString::fromUtf8("dialogClockTime"));
        dialogClockTime->resize(342, 103);
        label = new QLabel(dialogClockTime);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 271, 16));
        timeSlider = new QSlider(dialogClockTime);
        timeSlider->setObjectName(QString::fromUtf8("timeSlider"));
        timeSlider->setGeometry(QRect(20, 40, 221, 19));
        timeSlider->setOrientation(Qt::Horizontal);
        timeEdit = new QLineEdit(dialogClockTime);
        timeEdit->setObjectName(QString::fromUtf8("timeEdit"));
        timeEdit->setGeometry(QRect(250, 40, 71, 20));
        cancelButton = new QPushButton(dialogClockTime);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(250, 70, 75, 23));

        retranslateUi(dialogClockTime);
        QObject::connect(timeSlider, SIGNAL(sliderReleased()), dialogClockTime, SLOT(doTime()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogClockTime, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogClockTime);
    } // setupUi

    void retranslateUi(QDialog *dialogClockTime)
    {
        dialogClockTime->setWindowTitle(QApplication::translate("dialogClockTime", "dialogClockTime", nullptr));
        label->setText(QApplication::translate("dialogClockTime", "Normalized clock interval [0,1] covers all data intervals", nullptr));
        cancelButton->setText(QApplication::translate("dialogClockTime", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogClockTime: public Ui_dialogClockTime {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGCLOCKTIME_H
