/********************************************************************************
** Form generated from reading UI file 'dialogFileScreen.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGFILESCREEN_H
#define UI_DIALOGFILESCREEN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_dialogFileScreen
{
public:
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QFrame *line;
    QLabel *label_4;
    QLabel *label_5;
    QPushButton *oneBrowseButton;
    QPushButton *oneSnapButton;
    QLineEdit *oneBrowseEdit;
    QFrame *line_2;
    QFrame *line_3;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QPushButton *multBrowseButton;
    QLineEdit *multBrowseEdit;
    QPushButton *snapNextButton;
    QPushButton *cancelButton;
    QLineEdit *yEdit;
    QLineEdit *xEdit;

    void setupUi(QDialog *dialogFileScreen)
    {
        if (dialogFileScreen->objectName().isEmpty())
            dialogFileScreen->setObjectName(QString::fromUtf8("dialogFileScreen"));
        dialogFileScreen->resize(379, 351);
        label = new QLabel(dialogFileScreen);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 10, 331, 16));
        label_2 = new QLabel(dialogFileScreen);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 40, 121, 16));
        label_3 = new QLabel(dialogFileScreen);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(220, 40, 47, 13));
        line = new QFrame(dialogFileScreen);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(20, 60, 341, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label_4 = new QLabel(dialogFileScreen);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(150, 70, 71, 16));
        label_5 = new QLabel(dialogFileScreen);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(30, 100, 111, 16));
        oneBrowseButton = new QPushButton(dialogFileScreen);
        oneBrowseButton->setObjectName(QString::fromUtf8("oneBrowseButton"));
        oneBrowseButton->setGeometry(QRect(290, 100, 75, 23));
        oneSnapButton = new QPushButton(dialogFileScreen);
        oneSnapButton->setObjectName(QString::fromUtf8("oneSnapButton"));
        oneSnapButton->setGeometry(QRect(150, 130, 75, 23));
        oneBrowseEdit = new QLineEdit(dialogFileScreen);
        oneBrowseEdit->setObjectName(QString::fromUtf8("oneBrowseEdit"));
        oneBrowseEdit->setGeometry(QRect(120, 100, 161, 20));
        line_2 = new QFrame(dialogFileScreen);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(30, 160, 341, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(dialogFileScreen);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(20, 300, 341, 16));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        label_6 = new QLabel(dialogFileScreen);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(150, 170, 131, 16));
        label_7 = new QLabel(dialogFileScreen);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(30, 190, 321, 16));
        label_8 = new QLabel(dialogFileScreen);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(30, 210, 331, 16));
        label_9 = new QLabel(dialogFileScreen);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(30, 240, 71, 16));
        multBrowseButton = new QPushButton(dialogFileScreen);
        multBrowseButton->setObjectName(QString::fromUtf8("multBrowseButton"));
        multBrowseButton->setGeometry(QRect(290, 240, 75, 23));
        multBrowseEdit = new QLineEdit(dialogFileScreen);
        multBrowseEdit->setObjectName(QString::fromUtf8("multBrowseEdit"));
        multBrowseEdit->setGeometry(QRect(90, 240, 191, 20));
        snapNextButton = new QPushButton(dialogFileScreen);
        snapNextButton->setObjectName(QString::fromUtf8("snapNextButton"));
        snapNextButton->setGeometry(QRect(150, 270, 75, 23));
        cancelButton = new QPushButton(dialogFileScreen);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(290, 320, 75, 23));
        yEdit = new QLineEdit(dialogFileScreen);
        yEdit->setObjectName(QString::fromUtf8("yEdit"));
        yEdit->setGeometry(QRect(260, 40, 71, 20));
        xEdit = new QLineEdit(dialogFileScreen);
        xEdit->setObjectName(QString::fromUtf8("xEdit"));
        xEdit->setGeometry(QRect(150, 40, 51, 20));

        retranslateUi(dialogFileScreen);
        QObject::connect(oneBrowseButton, SIGNAL(clicked()), dialogFileScreen, SLOT(doOneBrowse()));
        QObject::connect(multBrowseButton, SIGNAL(clicked()), dialogFileScreen, SLOT(doMultBrowse()));
        QObject::connect(oneSnapButton, SIGNAL(clicked()), dialogFileScreen, SLOT(doOneSnap()));
        QObject::connect(snapNextButton, SIGNAL(clicked()), dialogFileScreen, SLOT(doMultSnap()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogFileScreen, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogFileScreen);
    } // setupUi

    void retranslateUi(QDialog *dialogFileScreen)
    {
        dialogFileScreen->setWindowTitle(QApplication::translate("dialogFileScreen", "dialogFileScreen", nullptr));
        label->setText(QApplication::translate("dialogFileScreen", "Save screen or sequence of screens to JPEG file(s)", nullptr));
        label_2->setText(QApplication::translate("dialogFileScreen", "Size of saved image in x", nullptr));
        label_3->setText(QApplication::translate("dialogFileScreen", "and y", nullptr));
        label_4->setText(QApplication::translate("dialogFileScreen", "Single image", nullptr));
        label_5->setText(QApplication::translate("dialogFileScreen", "output filename", nullptr));
        oneBrowseButton->setText(QApplication::translate("dialogFileScreen", "Browse", nullptr));
        oneSnapButton->setText(QApplication::translate("dialogFileScreen", "Snap Single", nullptr));
        label_6->setText(QApplication::translate("dialogFileScreen", "Multiple images", nullptr));
        label_7->setText(QApplication::translate("dialogFileScreen", "e.g. base name 'C:/FusionDd/bin/temp' would produce a sequence", nullptr));
        label_8->setText(QApplication::translate("dialogFileScreen", "'C:/Fusion3D/bin/temp0001.jpg', 'C:/Fusion3D/bin/temp.0002.jpg' ...", nullptr));
        label_9->setText(QApplication::translate("dialogFileScreen", "Base name", nullptr));
        multBrowseButton->setText(QApplication::translate("dialogFileScreen", "Browse", nullptr));
        snapNextButton->setText(QApplication::translate("dialogFileScreen", "Snap Next", nullptr));
        cancelButton->setText(QApplication::translate("dialogFileScreen", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogFileScreen: public Ui_dialogFileScreen {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGFILESCREEN_H
