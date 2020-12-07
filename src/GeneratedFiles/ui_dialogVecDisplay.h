/********************************************************************************
** Form generated from reading UI file 'dialogVecDisplay.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGVECDISPLAY_H
#define UI_DIALOGVECDISPLAY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_dialogVecDisplay
{
public:
    QLabel *label;
    QLineEdit *inputEdit;
    QPushButton *browseButton;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLineEdit *redEdit;
    QLineEdit *grnEdit;
    QLineEdit *bluEdit;
    QLineEdit *altEdit;
    QRadioButton *withButton;
    QRadioButton *overButton;
    QLabel *label_9;
    QLabel *label_10;
    QRadioButton *wireButton;
    QRadioButton *filledButton;
    QRadioButton *clampButton;
    QRadioButton *relButton;
    QRadioButton *absoluteButton;
    QRadioButton *radioButton;
    QPushButton *loadButton;
    QPushButton *cancelButton;
    QFrame *line;
    QFrame *line_2;

    void setupUi(QDialog *dialogVecDisplay)
    {
        if (dialogVecDisplay->objectName().isEmpty())
            dialogVecDisplay->setObjectName(QString::fromUtf8("dialogVecDisplay"));
        dialogVecDisplay->resize(498, 415);
        label = new QLabel(dialogVecDisplay);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 91, 16));
        inputEdit = new QLineEdit(dialogVecDisplay);
        inputEdit->setObjectName(QString::fromUtf8("inputEdit"));
        inputEdit->setGeometry(QRect(10, 30, 391, 20));
        browseButton = new QPushButton(dialogVecDisplay);
        browseButton->setObjectName(QString::fromUtf8("browseButton"));
        browseButton->setGeometry(QRect(410, 30, 75, 23));
        label_2 = new QLabel(dialogVecDisplay);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 70, 181, 21));
        label_3 = new QLabel(dialogVecDisplay);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(50, 100, 61, 16));
        label_4 = new QLabel(dialogVecDisplay);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(190, 100, 61, 16));
        label_5 = new QLabel(dialogVecDisplay);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(350, 100, 51, 20));
        label_6 = new QLabel(dialogVecDisplay);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(20, 130, 101, 16));
        label_7 = new QLabel(dialogVecDisplay);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(20, 170, 221, 16));
        label_8 = new QLabel(dialogVecDisplay);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(20, 240, 131, 16));
        redEdit = new QLineEdit(dialogVecDisplay);
        redEdit->setObjectName(QString::fromUtf8("redEdit"));
        redEdit->setGeometry(QRect(100, 100, 61, 20));
        grnEdit = new QLineEdit(dialogVecDisplay);
        grnEdit->setObjectName(QString::fromUtf8("grnEdit"));
        grnEdit->setGeometry(QRect(250, 100, 71, 20));
        bluEdit = new QLineEdit(dialogVecDisplay);
        bluEdit->setObjectName(QString::fromUtf8("bluEdit"));
        bluEdit->setGeometry(QRect(400, 100, 71, 20));
        altEdit = new QLineEdit(dialogVecDisplay);
        altEdit->setObjectName(QString::fromUtf8("altEdit"));
        altEdit->setGeometry(QRect(120, 130, 113, 20));
        withButton = new QRadioButton(dialogVecDisplay);
        withButton->setObjectName(QString::fromUtf8("withButton"));
        withButton->setGeometry(QRect(120, 190, 121, 17));
        overButton = new QRadioButton(dialogVecDisplay);
        overButton->setObjectName(QString::fromUtf8("overButton"));
        overButton->setGeometry(QRect(120, 210, 261, 17));
        label_9 = new QLabel(dialogVecDisplay);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(70, 260, 91, 16));
        label_10 = new QLabel(dialogVecDisplay);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(270, 260, 131, 16));
        wireButton = new QRadioButton(dialogVecDisplay);
        wireButton->setObjectName(QString::fromUtf8("wireButton"));
        wireButton->setGeometry(QRect(100, 280, 101, 17));
        filledButton = new QRadioButton(dialogVecDisplay);
        filledButton->setObjectName(QString::fromUtf8("filledButton"));
        filledButton->setGeometry(QRect(100, 300, 82, 17));
        clampButton = new QRadioButton(dialogVecDisplay);
        clampButton->setObjectName(QString::fromUtf8("clampButton"));
        clampButton->setGeometry(QRect(310, 280, 121, 17));
        relButton = new QRadioButton(dialogVecDisplay);
        relButton->setObjectName(QString::fromUtf8("relButton"));
        relButton->setGeometry(QRect(310, 300, 111, 17));
        absoluteButton = new QRadioButton(dialogVecDisplay);
        absoluteButton->setObjectName(QString::fromUtf8("absoluteButton"));
        absoluteButton->setGeometry(QRect(310, 320, 82, 17));
        radioButton = new QRadioButton(dialogVecDisplay);
        radioButton->setObjectName(QString::fromUtf8("radioButton"));
        radioButton->setGeometry(QRect(310, 340, 171, 17));
        loadButton = new QPushButton(dialogVecDisplay);
        loadButton->setObjectName(QString::fromUtf8("loadButton"));
        loadButton->setGeometry(QRect(260, 380, 75, 23));
        cancelButton = new QPushButton(dialogVecDisplay);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(370, 380, 75, 23));
        line = new QFrame(dialogVecDisplay);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(0, 230, 481, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line_2 = new QFrame(dialogVecDisplay);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(0, 360, 481, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        retranslateUi(dialogVecDisplay);
        QObject::connect(browseButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doBrowse()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(reject()));
        QObject::connect(loadButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doLoad()));
        QObject::connect(wireButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doWire()));
        QObject::connect(filledButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doFilled()));
        QObject::connect(clampButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doClamp()));
        QObject::connect(relButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doRel()));
        QObject::connect(absoluteButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doAbsolute()));
        QObject::connect(radioButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doAttr()));
        QObject::connect(withButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doWith()));
        QObject::connect(overButton, SIGNAL(clicked()), dialogVecDisplay, SLOT(doOver()));

        QMetaObject::connectSlotsByName(dialogVecDisplay);
    } // setupUi

    void retranslateUi(QDialog *dialogVecDisplay)
    {
        dialogVecDisplay->setWindowTitle(QApplication::translate("dialogVecDisplay", "Enter Vector Overlay Files", nullptr));
        label->setText(QApplication::translate("dialogVecDisplay", "Input file(s)", nullptr));
        browseButton->setText(QApplication::translate("dialogVecDisplay", "Browse", nullptr));
        label_2->setText(QApplication::translate("dialogVecDisplay", "Default color (if not specified in file)", nullptr));
        label_3->setText(QApplication::translate("dialogVecDisplay", "Red [0,1]", nullptr));
        label_4->setText(QApplication::translate("dialogVecDisplay", "Green [0,1]", nullptr));
        label_5->setText(QApplication::translate("dialogVecDisplay", "Blue [0,1]", nullptr));
        label_6->setText(QApplication::translate("dialogVecDisplay", "Altitude offset (m)", nullptr));
        label_7->setText(QApplication::translate("dialogVecDisplay", "Overwrite all map objects so always visible", nullptr));
        label_8->setText(QApplication::translate("dialogVecDisplay", "Shapefile parameters", nullptr));
        withButton->setText(QApplication::translate("dialogVecDisplay", "Render with map", nullptr));
        overButton->setText(QApplication::translate("dialogVecDisplay", "Overwrite map (filled polygons show artifacts", nullptr));
        label_9->setText(QApplication::translate("dialogVecDisplay", "Polygon fill type", nullptr));
        label_10->setText(QApplication::translate("dialogVecDisplay", "Altitude modes", nullptr));
        wireButton->setText(QApplication::translate("dialogVecDisplay", "Wireframe", nullptr));
        filledButton->setText(QApplication::translate("dialogVecDisplay", "Filled ", nullptr));
        clampButton->setText(QApplication::translate("dialogVecDisplay", "Clamp to ground", nullptr));
        relButton->setText(QApplication::translate("dialogVecDisplay", "Relative to ground", nullptr));
        absoluteButton->setText(QApplication::translate("dialogVecDisplay", "Absolute", nullptr));
        radioButton->setText(QApplication::translate("dialogVecDisplay", "Relative to attribute GrndHae", nullptr));
        loadButton->setText(QApplication::translate("dialogVecDisplay", "Load", nullptr));
        cancelButton->setText(QApplication::translate("dialogVecDisplay", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogVecDisplay: public Ui_dialogVecDisplay {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGVECDISPLAY_H
