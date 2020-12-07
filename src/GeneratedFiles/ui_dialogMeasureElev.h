/********************************************************************************
** Form generated from reading UI file 'dialogMeasureElev.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGMEASUREELEV_H
#define UI_DIALOGMEASUREELEV_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_dialogMeasureElev
{
public:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QSlider *lowerSlider;
    QSlider *upperSlider;
    QLineEdit *lowerEdit;
    QLineEdit *upperEdit;
    QPushButton *cancelButton;
    QPushButton *applyButton;

    void setupUi(QDialog *dialogMeasureElev)
    {
        if (dialogMeasureElev->objectName().isEmpty())
            dialogMeasureElev->setObjectName(QString::fromUtf8("dialogMeasureElev"));
        dialogMeasureElev->resize(375, 257);
        label = new QLabel(dialogMeasureElev);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 361, 16));
        label_2 = new QLabel(dialogMeasureElev);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 40, 341, 16));
        label_3 = new QLabel(dialogMeasureElev);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 70, 321, 16));
        label_4 = new QLabel(dialogMeasureElev);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 90, 321, 16));
        label_5 = new QLabel(dialogMeasureElev);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(20, 120, 311, 16));
        label_6 = new QLabel(dialogMeasureElev);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(20, 160, 101, 16));
        label_7 = new QLabel(dialogMeasureElev);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(20, 190, 101, 16));
        lowerSlider = new QSlider(dialogMeasureElev);
        lowerSlider->setObjectName(QString::fromUtf8("lowerSlider"));
        lowerSlider->setGeometry(QRect(120, 160, 160, 19));
        lowerSlider->setOrientation(Qt::Horizontal);
        upperSlider = new QSlider(dialogMeasureElev);
        upperSlider->setObjectName(QString::fromUtf8("upperSlider"));
        upperSlider->setGeometry(QRect(120, 190, 160, 19));
        upperSlider->setOrientation(Qt::Horizontal);
        lowerEdit = new QLineEdit(dialogMeasureElev);
        lowerEdit->setObjectName(QString::fromUtf8("lowerEdit"));
        lowerEdit->setGeometry(QRect(290, 160, 71, 20));
        upperEdit = new QLineEdit(dialogMeasureElev);
        upperEdit->setObjectName(QString::fromUtf8("upperEdit"));
        upperEdit->setGeometry(QRect(290, 190, 71, 20));
        cancelButton = new QPushButton(dialogMeasureElev);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(290, 220, 75, 23));
        applyButton = new QPushButton(dialogMeasureElev);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setGeometry(QRect(200, 220, 75, 23));

        retranslateUi(dialogMeasureElev);
        QObject::connect(lowerSlider, SIGNAL(sliderReleased()), dialogMeasureElev, SLOT(doLower()));
        QObject::connect(upperSlider, SIGNAL(sliderReleased()), dialogMeasureElev, SLOT(doUpper()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogMeasureElev, SLOT(reject()));
        QObject::connect(applyButton, SIGNAL(clicked()), dialogMeasureElev, SLOT(doApply()));

        QMetaObject::connectSlotsByName(dialogMeasureElev);
    } // setupUi

    void retranslateUi(QDialog *dialogMeasureElev)
    {
        dialogMeasureElev->setWindowTitle(QApplication::translate("dialogMeasureElev", "dialogMeasureElev", nullptr));
        label->setText(QApplication::translate("dialogMeasureElev", "Highlight locations where difference between first-return (a1) DEM", nullptr));
        label_2->setText(QApplication::translate("dialogMeasureElev", "and last-return (a2) DEM is greater that given threshold", nullptr));
        label_3->setText(QApplication::translate("dialogMeasureElev", "Differences greater than the higher threshold are shown in red", nullptr));
        label_4->setText(QApplication::translate("dialogMeasureElev", "Differences greater than the lower threshold are shown in yellow", nullptr));
        label_5->setText(QApplication::translate("dialogMeasureElev", "Toggle highlighting with button on left side of main window", nullptr));
        label_6->setText(QApplication::translate("dialogMeasureElev", "Lower threshold (m)", nullptr));
        label_7->setText(QApplication::translate("dialogMeasureElev", "Upper threshold (m)", nullptr));
        cancelButton->setText(QApplication::translate("dialogMeasureElev", "Cancel", nullptr));
        applyButton->setText(QApplication::translate("dialogMeasureElev", "Apply", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogMeasureElev: public Ui_dialogMeasureElev {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGMEASUREELEV_H
