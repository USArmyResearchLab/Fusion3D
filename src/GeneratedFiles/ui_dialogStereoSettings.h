/********************************************************************************
** Form generated from reading UI file 'dialogStereoSettings.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGSTEREOSETTINGS_H
#define UI_DIALOGSTEREOSETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_dialogStereoSettings
{
public:
    QLabel *label;
    QLabel *label_2;
    QRadioButton *offButton;
    QRadioButton *rawButton;
    QRadioButton *anaglyphButton;
    QCheckBox *swapBox;
    QSlider *offsetSlider;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QPushButton *cancelButton;
    QLineEdit *offEdit;

    void setupUi(QDialog *dialogStereoSettings)
    {
        if (dialogStereoSettings->objectName().isEmpty())
            dialogStereoSettings->setObjectName(QString::fromUtf8("dialogStereoSettings"));
        dialogStereoSettings->resize(373, 210);
        label = new QLabel(dialogStereoSettings);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 361, 21));
        label_2 = new QLabel(dialogStereoSettings);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(130, 30, 191, 16));
        offButton = new QRadioButton(dialogStereoSettings);
        offButton->setObjectName(QString::fromUtf8("offButton"));
        offButton->setGeometry(QRect(30, 50, 161, 17));
        rawButton = new QRadioButton(dialogStereoSettings);
        rawButton->setObjectName(QString::fromUtf8("rawButton"));
        rawButton->setGeometry(QRect(30, 70, 321, 17));
        anaglyphButton = new QRadioButton(dialogStereoSettings);
        anaglyphButton->setObjectName(QString::fromUtf8("anaglyphButton"));
        anaglyphButton->setGeometry(QRect(30, 90, 231, 17));
        swapBox = new QCheckBox(dialogStereoSettings);
        swapBox->setObjectName(QString::fromUtf8("swapBox"));
        swapBox->setGeometry(QRect(30, 160, 211, 17));
        offsetSlider = new QSlider(dialogStereoSettings);
        offsetSlider->setObjectName(QString::fromUtf8("offsetSlider"));
        offsetSlider->setGeometry(QRect(120, 130, 181, 19));
        offsetSlider->setOrientation(Qt::Horizontal);
        label_3 = new QLabel(dialogStereoSettings);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 130, 91, 16));
        label_4 = new QLabel(dialogStereoSettings);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(120, 110, 21, 16));
        label_5 = new QLabel(dialogStereoSettings);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(300, 110, 21, 16));
        cancelButton = new QPushButton(dialogStereoSettings);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(290, 180, 75, 23));
        offEdit = new QLineEdit(dialogStereoSettings);
        offEdit->setObjectName(QString::fromUtf8("offEdit"));
        offEdit->setGeometry(QRect(310, 130, 41, 20));

        retranslateUi(dialogStereoSettings);
        QObject::connect(offButton, SIGNAL(clicked()), dialogStereoSettings, SLOT(doOff()));
        QObject::connect(rawButton, SIGNAL(clicked()), dialogStereoSettings, SLOT(doRaw()));
        QObject::connect(anaglyphButton, SIGNAL(clicked()), dialogStereoSettings, SLOT(doAnaglyph()));
        QObject::connect(offsetSlider, SIGNAL(sliderReleased()), dialogStereoSettings, SLOT(doOffset()));
        QObject::connect(swapBox, SIGNAL(toggled(bool)), dialogStereoSettings, SLOT(doSwap(bool)));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogStereoSettings, SLOT(doCancel()));
        QObject::connect(offEdit, SIGNAL(editingFinished()), dialogStereoSettings, SLOT(doOffsetEdit()));

        QMetaObject::connectSlotsByName(dialogStereoSettings);
    } // setupUi

    void retranslateUi(QDialog *dialogStereoSettings)
    {
        dialogStereoSettings->setWindowTitle(QApplication::translate("dialogStereoSettings", "dialogStereoSettings", nullptr));
        label->setText(QApplication::translate("dialogStereoSettings", "Coin3D library cant toggle between raw stereo and other stereo settings", nullptr));
        label_2->setText(QApplication::translate("dialogStereoSettings", "Use Defaults  menu to do this", nullptr));
        offButton->setText(QApplication::translate("dialogStereoSettings", "Stereo off", nullptr));
        rawButton->setText(QApplication::translate("dialogStereoSettings", "Raw stereo (OpenGL Quadbuffered)", nullptr));
        anaglyphButton->setText(QApplication::translate("dialogStereoSettings", "Anaglyph (red-blue)", nullptr));
        swapBox->setText(QApplication::translate("dialogStereoSettings", "Swap Eyes -- if stereo effect 'inside out'", nullptr));
        label_3->setText(QApplication::translate("dialogStereoSettings", "Camera offset", nullptr));
        label_4->setText(QApplication::translate("dialogStereoSettings", "0", nullptr));
        label_5->setText(QApplication::translate("dialogStereoSettings", "4", nullptr));
        cancelButton->setText(QApplication::translate("dialogStereoSettings", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogStereoSettings: public Ui_dialogStereoSettings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGSTEREOSETTINGS_H
