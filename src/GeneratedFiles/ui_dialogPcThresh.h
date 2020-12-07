/********************************************************************************
** Form generated from reading UI file 'dialogPcThresh.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGPCTHRESH_H
#define UI_DIALOGPCTHRESH_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_dialogPcThresh
{
public:
    QLabel *label;
    QSlider *threshSlider;
    QLineEdit *threshEdit;
    QPushButton *downButton;
    QPushButton *upButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *dialogPcThresh)
    {
        if (dialogPcThresh->objectName().isEmpty())
            dialogPcThresh->setObjectName(QString::fromUtf8("dialogPcThresh"));
        dialogPcThresh->resize(344, 139);
        label = new QLabel(dialogPcThresh);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 281, 16));
        threshSlider = new QSlider(dialogPcThresh);
        threshSlider->setObjectName(QString::fromUtf8("threshSlider"));
        threshSlider->setGeometry(QRect(30, 40, 231, 19));
        threshSlider->setOrientation(Qt::Horizontal);
        threshEdit = new QLineEdit(dialogPcThresh);
        threshEdit->setObjectName(QString::fromUtf8("threshEdit"));
        threshEdit->setGeometry(QRect(280, 40, 41, 20));
        downButton = new QPushButton(dialogPcThresh);
        downButton->setObjectName(QString::fromUtf8("downButton"));
        downButton->setGeometry(QRect(30, 70, 75, 23));
        upButton = new QPushButton(dialogPcThresh);
        upButton->setObjectName(QString::fromUtf8("upButton"));
        upButton->setGeometry(QRect(190, 70, 75, 23));
        cancelButton = new QPushButton(dialogPcThresh);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(250, 110, 75, 23));

        retranslateUi(dialogPcThresh);
        QObject::connect(threshSlider, SIGNAL(sliderReleased()), dialogPcThresh, SLOT(doThresh()));
        QObject::connect(downButton, SIGNAL(clicked()), dialogPcThresh, SLOT(doDown()));
        QObject::connect(upButton, SIGNAL(clicked()), dialogPcThresh, SLOT(doUp()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogPcThresh, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogPcThresh);
    } // setupUi

    void retranslateUi(QDialog *dialogPcThresh)
    {
        dialogPcThresh->setWindowTitle(QApplication::translate("dialogPcThresh", "dialogPcThresh", nullptr));
        label->setText(QApplication::translate("dialogPcThresh", "Display only points with TAU values above this threshold", nullptr));
        downButton->setText(QApplication::translate("dialogPcThresh", "Down", nullptr));
        upButton->setText(QApplication::translate("dialogPcThresh", "Up", nullptr));
        cancelButton->setText(QApplication::translate("dialogPcThresh", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogPcThresh: public Ui_dialogPcThresh {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGPCTHRESH_H
