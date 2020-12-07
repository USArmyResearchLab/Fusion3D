/********************************************************************************
** Form generated from reading UI file 'dialogVecFly.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGVECFLY_H
#define UI_DIALOGVECFLY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_dialogVecFly
{
public:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QRadioButton *forwardButton;
    QRadioButton *reverseButton;
    QRadioButton *overButton;
    QRadioButton *mixedButton;
    QCheckBox *checkBox;
    QPushButton *cancelButton;
    QSlider *locSlider;
    QSlider *speedSlider;
    QSlider *avgSlider;

    void setupUi(QDialog *dialogVecFly)
    {
        if (dialogVecFly->objectName().isEmpty())
            dialogVecFly->setObjectName(QString::fromUtf8("dialogVecFly"));
        dialogVecFly->resize(400, 300);
        label = new QLabel(dialogVecFly);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 91, 16));
        label_2 = new QLabel(dialogVecFly);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(180, 10, 121, 16));
        label_3 = new QLabel(dialogVecFly);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 80, 91, 16));
        label_4 = new QLabel(dialogVecFly);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 110, 61, 16));
        label_5 = new QLabel(dialogVecFly);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(20, 140, 61, 16));
        forwardButton = new QRadioButton(dialogVecFly);
        forwardButton->setObjectName(QString::fromUtf8("forwardButton"));
        forwardButton->setGeometry(QRect(40, 30, 131, 17));
        reverseButton = new QRadioButton(dialogVecFly);
        reverseButton->setObjectName(QString::fromUtf8("reverseButton"));
        reverseButton->setGeometry(QRect(40, 50, 82, 17));
        overButton = new QRadioButton(dialogVecFly);
        overButton->setObjectName(QString::fromUtf8("overButton"));
        overButton->setGeometry(QRect(220, 30, 101, 17));
        mixedButton = new QRadioButton(dialogVecFly);
        mixedButton->setObjectName(QString::fromUtf8("mixedButton"));
        mixedButton->setGeometry(QRect(220, 50, 151, 17));
        checkBox = new QCheckBox(dialogVecFly);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(20, 180, 231, 17));
        cancelButton = new QPushButton(dialogVecFly);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(280, 210, 75, 23));
        locSlider = new QSlider(dialogVecFly);
        locSlider->setObjectName(QString::fromUtf8("locSlider"));
        locSlider->setGeometry(QRect(119, 80, 231, 20));
        locSlider->setOrientation(Qt::Horizontal);
        speedSlider = new QSlider(dialogVecFly);
        speedSlider->setObjectName(QString::fromUtf8("speedSlider"));
        speedSlider->setGeometry(QRect(120, 110, 231, 19));
        speedSlider->setOrientation(Qt::Horizontal);
        avgSlider = new QSlider(dialogVecFly);
        avgSlider->setObjectName(QString::fromUtf8("avgSlider"));
        avgSlider->setGeometry(QRect(120, 140, 231, 19));
        avgSlider->setOrientation(Qt::Horizontal);

        retranslateUi(dialogVecFly);
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogVecFly, SLOT(reject()));
        QObject::connect(checkBox, SIGNAL(toggled(bool)), dialogVecFly, SLOT(doLook(bool)));
        QObject::connect(reverseButton, SIGNAL(clicked()), dialogVecFly, SLOT(doReverse()));
        QObject::connect(overButton, SIGNAL(clicked()), dialogVecFly, SLOT(doTop()));
        QObject::connect(mixedButton, SIGNAL(clicked()), dialogVecFly, SLOT(doMixed()));
        QObject::connect(forwardButton, SIGNAL(clicked()), dialogVecFly, SLOT(doForward()));
        QObject::connect(locSlider, SIGNAL(sliderReleased()), dialogVecFly, SLOT(doLoc()));
        QObject::connect(speedSlider, SIGNAL(sliderReleased()), dialogVecFly, SLOT(doSpeed()));
        QObject::connect(avgSlider, SIGNAL(sliderReleased()), dialogVecFly, SLOT(doAvg()));

        QMetaObject::connectSlotsByName(dialogVecFly);
    } // setupUi

    void retranslateUi(QDialog *dialogVecFly)
    {
        dialogVecFly->setWindowTitle(QApplication::translate("dialogVecFly", "dialogVecFly", nullptr));
        label->setText(QApplication::translate("dialogVecFly", "Direction of travel", nullptr));
        label_2->setText(QApplication::translate("dialogVecFly", "How to draw the track", nullptr));
        label_3->setText(QApplication::translate("dialogVecFly", "Loc along track", nullptr));
        label_4->setText(QApplication::translate("dialogVecFly", "Speed", nullptr));
        label_5->setText(QApplication::translate("dialogVecFly", "Averaging", nullptr));
        forwardButton->setText(QApplication::translate("dialogVecFly", "Forward (as digitized)", nullptr));
        reverseButton->setText(QApplication::translate("dialogVecFly", "Reverse", nullptr));
        overButton->setText(QApplication::translate("dialogVecFly", "Track on top", nullptr));
        mixedButton->setText(QApplication::translate("dialogVecFly", "Track mixed with terrain", nullptr));
        checkBox->setText(QApplication::translate("dialogVecFly", "Flyover controls look direction (azimuth)", nullptr));
        cancelButton->setText(QApplication::translate("dialogVecFly", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogVecFly: public Ui_dialogVecFly {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGVECFLY_H
