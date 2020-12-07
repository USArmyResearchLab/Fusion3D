/********************************************************************************
** Form generated from reading UI file 'dialogDrawNew.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGDRAWNEW_H
#define UI_DIALOGDRAWNEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_dialogDrawNew
{
public:
    QLabel *label;
    QRadioButton *lineButton;
    QRadioButton *polButton;
    QRadioButton *circleButton;
    QRadioButton *pointButton;
    QLabel *label_2;
    QRadioButton *solidButton;
    QRadioButton *dotButton;
    QLabel *label_3;
    QCheckBox *pointStyleBox;
    QFrame *line;
    QLabel *label_4;
    QLineEdit *nameEdit;
    QPushButton *applyButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *dialogDrawNew)
    {
        if (dialogDrawNew->objectName().isEmpty())
            dialogDrawNew->setObjectName(QString::fromUtf8("dialogDrawNew"));
        dialogDrawNew->resize(465, 515);
        label = new QLabel(dialogDrawNew);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 111, 16));
        lineButton = new QRadioButton(dialogDrawNew);
        lineButton->setObjectName(QString::fromUtf8("lineButton"));
        lineButton->setGeometry(QRect(30, 30, 91, 17));
        polButton = new QRadioButton(dialogDrawNew);
        polButton->setObjectName(QString::fromUtf8("polButton"));
        polButton->setGeometry(QRect(30, 50, 221, 17));
        circleButton = new QRadioButton(dialogDrawNew);
        circleButton->setObjectName(QString::fromUtf8("circleButton"));
        circleButton->setGeometry(QRect(30, 70, 221, 17));
        pointButton = new QRadioButton(dialogDrawNew);
        pointButton->setObjectName(QString::fromUtf8("pointButton"));
        pointButton->setGeometry(QRect(30, 90, 111, 17));
        label_2 = new QLabel(dialogDrawNew);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(310, 10, 191, 16));
        solidButton = new QRadioButton(dialogDrawNew);
        solidButton->setObjectName(QString::fromUtf8("solidButton"));
        solidButton->setGeometry(QRect(320, 30, 141, 17));
        dotButton = new QRadioButton(dialogDrawNew);
        dotButton->setObjectName(QString::fromUtf8("dotButton"));
        dotButton->setGeometry(QRect(320, 50, 141, 17));
        label_3 = new QLabel(dialogDrawNew);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(310, 70, 131, 16));
        pointStyleBox = new QCheckBox(dialogDrawNew);
        pointStyleBox->setObjectName(QString::fromUtf8("pointStyleBox"));
        pointStyleBox->setGeometry(QRect(320, 90, 171, 17));
        line = new QFrame(dialogDrawNew);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(10, 120, 441, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label_4 = new QLabel(dialogDrawNew);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(180, 130, 211, 16));
        nameEdit = new QLineEdit(dialogDrawNew);
        nameEdit->setObjectName(QString::fromUtf8("nameEdit"));
        nameEdit->setGeometry(QRect(200, 160, 113, 20));
        applyButton = new QPushButton(dialogDrawNew);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setGeometry(QRect(280, 480, 75, 23));
        cancelButton = new QPushButton(dialogDrawNew);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(370, 480, 75, 23));

        retranslateUi(dialogDrawNew);
        QObject::connect(lineButton, SIGNAL(clicked()), dialogDrawNew, SLOT(doLine()));
        QObject::connect(polButton, SIGNAL(clicked()), dialogDrawNew, SLOT(doPol()));
        QObject::connect(circleButton, SIGNAL(clicked()), dialogDrawNew, SLOT(doCircle()));
        QObject::connect(pointButton, SIGNAL(clicked()), dialogDrawNew, SLOT(doPoint()));
        QObject::connect(solidButton, SIGNAL(clicked()), dialogDrawNew, SLOT(doSolid()));
        QObject::connect(dotButton, SIGNAL(clicked()), dialogDrawNew, SLOT(doDot()));
        QObject::connect(applyButton, SIGNAL(clicked()), dialogDrawNew, SLOT(doApply()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogDrawNew, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogDrawNew);
    } // setupUi

    void retranslateUi(QDialog *dialogDrawNew)
    {
        dialogDrawNew->setWindowTitle(QApplication::translate("dialogDrawNew", "dialogDrawNew", nullptr));
        label->setText(QApplication::translate("dialogDrawNew", "Feature type", nullptr));
        lineButton->setText(QApplication::translate("dialogDrawNew", "Line", nullptr));
        polButton->setText(QApplication::translate("dialogDrawNew", "Polygon (last pt adjussted to match first)", nullptr));
        circleButton->setText(QApplication::translate("dialogDrawNew", "Circle (click center and drag to edge)", nullptr));
        pointButton->setText(QApplication::translate("dialogDrawNew", "Point", nullptr));
        label_2->setText(QApplication::translate("dialogDrawNew", "Line/polygon/circle style", nullptr));
        solidButton->setText(QApplication::translate("dialogDrawNew", "Solid line", nullptr));
        dotButton->setText(QApplication::translate("dialogDrawNew", "Dotted line", nullptr));
        label_3->setText(QApplication::translate("dialogDrawNew", "Point style", nullptr));
        pointStyleBox->setText(QApplication::translate("dialogDrawNew", "Draw a point at location", nullptr));
        label_4->setText(QApplication::translate("dialogDrawNew", "Specify name or icon for the point", nullptr));
        applyButton->setText(QApplication::translate("dialogDrawNew", "Apply", nullptr));
        cancelButton->setText(QApplication::translate("dialogDrawNew", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogDrawNew: public Ui_dialogDrawNew {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGDRAWNEW_H
