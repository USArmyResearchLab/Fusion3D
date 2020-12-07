/********************************************************************************
** Form generated from reading UI file 'dialogPcParms.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGPCPARMS_H
#define UI_DIALOGPCPARMS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_dialogPcParms
{
public:
    QLabel *label;
    QRadioButton *balanceOffButton;
    QRadioButton *balanceOnButton;
    QFrame *line;
    QLabel *label_2;
    QLabel *label_3;
    QRadioButton *falseNaturalButton;
    QRadioButton *falseBlueButton;
    QRadioButton *elevAbsButton;
    QRadioButton *elevRelButton;
    QSlider *minHueSlider;
    QSlider *maxHueSlider;
    QLineEdit *minHueEdit;
    QLineEdit *maxHueEdit;
    QLabel *label_4;
    QLabel *label_5;
    QPushButton *applyButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *dialogPcParms)
    {
        if (dialogPcParms->objectName().isEmpty())
            dialogPcParms->setObjectName(QString::fromUtf8("dialogPcParms"));
        dialogPcParms->resize(373, 308);
        label = new QLabel(dialogPcParms);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 291, 20));
        balanceOffButton = new QRadioButton(dialogPcParms);
        balanceOffButton->setObjectName(QString::fromUtf8("balanceOffButton"));
        balanceOffButton->setGeometry(QRect(20, 40, 241, 17));
        balanceOnButton = new QRadioButton(dialogPcParms);
        balanceOnButton->setObjectName(QString::fromUtf8("balanceOnButton"));
        balanceOnButton->setGeometry(QRect(20, 60, 301, 17));
        line = new QFrame(dialogPcParms);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(10, 90, 341, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label_2 = new QLabel(dialogPcParms);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 110, 121, 16));
        label_3 = new QLabel(dialogPcParms);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(80, 250, 151, 20));
        falseNaturalButton = new QRadioButton(dialogPcParms);
        falseNaturalButton->setObjectName(QString::fromUtf8("falseNaturalButton"));
        falseNaturalButton->setGeometry(QRect(20, 140, 131, 17));
        falseBlueButton = new QRadioButton(dialogPcParms);
        falseBlueButton->setObjectName(QString::fromUtf8("falseBlueButton"));
        falseBlueButton->setGeometry(QRect(20, 160, 131, 17));
        elevAbsButton = new QRadioButton(dialogPcParms);
        elevAbsButton->setObjectName(QString::fromUtf8("elevAbsButton"));
        elevAbsButton->setGeometry(QRect(190, 140, 131, 17));
        elevRelButton = new QRadioButton(dialogPcParms);
        elevRelButton->setObjectName(QString::fromUtf8("elevRelButton"));
        elevRelButton->setGeometry(QRect(190, 160, 171, 20));
        minHueSlider = new QSlider(dialogPcParms);
        minHueSlider->setObjectName(QString::fromUtf8("minHueSlider"));
        minHueSlider->setGeometry(QRect(110, 190, 160, 19));
        minHueSlider->setOrientation(Qt::Horizontal);
        maxHueSlider = new QSlider(dialogPcParms);
        maxHueSlider->setObjectName(QString::fromUtf8("maxHueSlider"));
        maxHueSlider->setGeometry(QRect(110, 220, 160, 19));
        maxHueSlider->setOrientation(Qt::Horizontal);
        minHueEdit = new QLineEdit(dialogPcParms);
        minHueEdit->setObjectName(QString::fromUtf8("minHueEdit"));
        minHueEdit->setGeometry(QRect(290, 190, 71, 20));
        maxHueEdit = new QLineEdit(dialogPcParms);
        maxHueEdit->setObjectName(QString::fromUtf8("maxHueEdit"));
        maxHueEdit->setGeometry(QRect(290, 220, 71, 20));
        label_4 = new QLabel(dialogPcParms);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 190, 71, 16));
        label_5 = new QLabel(dialogPcParms);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(20, 220, 81, 16));
        applyButton = new QPushButton(dialogPcParms);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setGeometry(QRect(244, 250, 121, 23));
        cancelButton = new QPushButton(dialogPcParms);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(290, 280, 75, 23));

        retranslateUi(dialogPcParms);
        QObject::connect(balanceOffButton, SIGNAL(clicked()), dialogPcParms, SLOT(doBalanceOff()));
        QObject::connect(balanceOnButton, SIGNAL(clicked()), dialogPcParms, SLOT(doBalanceOn()));
        QObject::connect(falseNaturalButton, SIGNAL(clicked()), dialogPcParms, SLOT(doNatural()));
        QObject::connect(falseBlueButton, SIGNAL(clicked()), dialogPcParms, SLOT(doBlue()));
        QObject::connect(minHueSlider, SIGNAL(sliderReleased()), dialogPcParms, SLOT(doMinHue()));
        QObject::connect(maxHueSlider, SIGNAL(sliderReleased()), dialogPcParms, SLOT(doMaxHue()));
        QObject::connect(elevRelButton, SIGNAL(clicked()), dialogPcParms, SLOT(doElevRel()));
        QObject::connect(elevAbsButton, SIGNAL(clicked()), dialogPcParms, SLOT(doElevAbs()));
        QObject::connect(applyButton, SIGNAL(clicked()), dialogPcParms, SLOT(doNewFalse()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogPcParms, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogPcParms);
    } // setupUi

    void retranslateUi(QDialog *dialogPcParms)
    {
        dialogPcParms->setWindowTitle(QApplication::translate("dialogPcParms", "dialogPcParms", nullptr));
        label->setText(QApplication::translate("dialogPcParms", "Color balance options  (changes slow for large datasets)", nullptr));
        balanceOffButton->setText(QApplication::translate("dialogPcParms", "No color balance", nullptr));
        balanceOnButton->setText(QApplication::translate("dialogPcParms", "Histogram equalization independently on each channel", nullptr));
        label_2->setText(QApplication::translate("dialogPcParms", "False color options", nullptr));
        label_3->setText(QApplication::translate("dialogPcParms", "May be slow for large datasets", nullptr));
        falseNaturalButton->setText(QApplication::translate("dialogPcParms", "Natural (green-white)", nullptr));
        falseBlueButton->setText(QApplication::translate("dialogPcParms", "Blue-to-red", nullptr));
        elevAbsButton->setText(QApplication::translate("dialogPcParms", "Absolute elevation", nullptr));
        elevRelButton->setText(QApplication::translate("dialogPcParms", "Elevation rel to smoothed DEM", nullptr));
        label_4->setText(QApplication::translate("dialogPcParms", "Elev of min hue", nullptr));
        label_5->setText(QApplication::translate("dialogPcParms", "Elev o max hue", nullptr));
        applyButton->setText(QApplication::translate("dialogPcParms", "Apply new color scale", nullptr));
        cancelButton->setText(QApplication::translate("dialogPcParms", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogPcParms: public Ui_dialogPcParms {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGPCPARMS_H
