/********************************************************************************
** Form generated from reading UI file 'dialogCadOpen.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGCADOPEN_H
#define UI_DIALOGCADOPEN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_dialogCadOpen
{
public:
    QLabel *label;
    QLineEdit *fileEdit;
    QPushButton *browseButton;
    QLineEdit *northEdit;
    QLineEdit *eastEdit;
    QLineEdit *elevEdit;
    QRadioButton *relButton;
    QRadioButton *absButton;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *rotxEdit;
    QLineEdit *rotyEdit;
    QLineEdit *rotzEdit;
    QLabel *label_6;
    QFrame *line;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *label_12;
    QLineEdit *scaleEdit;
    QPushButton *loadButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *dialogCadOpen)
    {
        if (dialogCadOpen->objectName().isEmpty())
            dialogCadOpen->setObjectName(QString::fromUtf8("dialogCadOpen"));
        dialogCadOpen->resize(511, 300);
        label = new QLabel(dialogCadOpen);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 91, 16));
        fileEdit = new QLineEdit(dialogCadOpen);
        fileEdit->setObjectName(QString::fromUtf8("fileEdit"));
        fileEdit->setGeometry(QRect(120, 20, 291, 20));
        browseButton = new QPushButton(dialogCadOpen);
        browseButton->setObjectName(QString::fromUtf8("browseButton"));
        browseButton->setGeometry(QRect(430, 20, 75, 23));
        northEdit = new QLineEdit(dialogCadOpen);
        northEdit->setObjectName(QString::fromUtf8("northEdit"));
        northEdit->setGeometry(QRect(90, 90, 113, 20));
        eastEdit = new QLineEdit(dialogCadOpen);
        eastEdit->setObjectName(QString::fromUtf8("eastEdit"));
        eastEdit->setGeometry(QRect(90, 130, 113, 20));
        elevEdit = new QLineEdit(dialogCadOpen);
        elevEdit->setObjectName(QString::fromUtf8("elevEdit"));
        elevEdit->setGeometry(QRect(90, 210, 113, 20));
        relButton = new QRadioButton(dialogCadOpen);
        relButton->setObjectName(QString::fromUtf8("relButton"));
        relButton->setGeometry(QRect(40, 160, 221, 17));
        absButton = new QRadioButton(dialogCadOpen);
        absButton->setObjectName(QString::fromUtf8("absButton"));
        absButton->setGeometry(QRect(40, 180, 161, 17));
        label_2 = new QLabel(dialogCadOpen);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 50, 201, 16));
        label_3 = new QLabel(dialogCadOpen);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 90, 47, 13));
        label_4 = new QLabel(dialogCadOpen);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(40, 130, 47, 13));
        label_5 = new QLabel(dialogCadOpen);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(40, 210, 41, 16));
        rotxEdit = new QLineEdit(dialogCadOpen);
        rotxEdit->setObjectName(QString::fromUtf8("rotxEdit"));
        rotxEdit->setGeometry(QRect(390, 100, 113, 20));
        rotyEdit = new QLineEdit(dialogCadOpen);
        rotyEdit->setObjectName(QString::fromUtf8("rotyEdit"));
        rotyEdit->setGeometry(QRect(390, 130, 113, 20));
        rotzEdit = new QLineEdit(dialogCadOpen);
        rotzEdit->setObjectName(QString::fromUtf8("rotzEdit"));
        rotzEdit->setGeometry(QRect(390, 160, 113, 20));
        label_6 = new QLabel(dialogCadOpen);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(20, 70, 241, 16));
        line = new QFrame(dialogCadOpen);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(253, 50, 20, 181));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        label_7 = new QLabel(dialogCadOpen);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(320, 50, 151, 16));
        label_8 = new QLabel(dialogCadOpen);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(350, 100, 47, 13));
        label_9 = new QLabel(dialogCadOpen);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(350, 130, 47, 13));
        label_10 = new QLabel(dialogCadOpen);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(350, 160, 41, 20));
        label_11 = new QLabel(dialogCadOpen);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(320, 70, 171, 16));
        label_12 = new QLabel(dialogCadOpen);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(330, 190, 71, 16));
        scaleEdit = new QLineEdit(dialogCadOpen);
        scaleEdit->setObjectName(QString::fromUtf8("scaleEdit"));
        scaleEdit->setGeometry(QRect(390, 210, 113, 20));
        loadButton = new QPushButton(dialogCadOpen);
        loadButton->setObjectName(QString::fromUtf8("loadButton"));
        loadButton->setGeometry(QRect(320, 260, 75, 23));
        cancelButton = new QPushButton(dialogCadOpen);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(410, 260, 75, 23));

        retranslateUi(dialogCadOpen);
        QObject::connect(browseButton, SIGNAL(clicked()), dialogCadOpen, SLOT(doBrowse()));
        QObject::connect(loadButton, SIGNAL(clicked()), dialogCadOpen, SLOT(doLoad()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogCadOpen, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogCadOpen);
    } // setupUi

    void retranslateUi(QDialog *dialogCadOpen)
    {
        dialogCadOpen->setWindowTitle(QApplication::translate("dialogCadOpen", "dialogCadOpen", nullptr));
        label->setText(QApplication::translate("dialogCadOpen", "Open CAD file", nullptr));
        browseButton->setText(QApplication::translate("dialogCadOpen", "Browse", nullptr));
        relButton->setText(QApplication::translate("dialogCadOpen", "Elevation relative to local map elevation", nullptr));
        absButton->setText(QApplication::translate("dialogCadOpen", "Elevation is absolute", nullptr));
        label_2->setText(QApplication::translate("dialogCadOpen", "Location on map of the CAD model origini", nullptr));
        label_3->setText(QApplication::translate("dialogCadOpen", "Northing", nullptr));
        label_4->setText(QApplication::translate("dialogCadOpen", "Easting", nullptr));
        label_5->setText(QApplication::translate("dialogCadOpen", "Elev (m)", nullptr));
        label_6->setText(QApplication::translate("dialogCadOpen", "in map projection coordinate (m)", nullptr));
        label_7->setText(QApplication::translate("dialogCadOpen", "Model rotations", nullptr));
        label_8->setText(QApplication::translate("dialogCadOpen", "X (deg)", nullptr));
        label_9->setText(QApplication::translate("dialogCadOpen", "Y (deg)", nullptr));
        label_10->setText(QApplication::translate("dialogCadOpen", "Z (deg)", nullptr));
        label_11->setText(QApplication::translate("dialogCadOpen", "x is East, y is North, z is Up", nullptr));
        label_12->setText(QApplication::translate("dialogCadOpen", "Scale factor", nullptr));
        loadButton->setText(QApplication::translate("dialogCadOpen", "Load", nullptr));
        cancelButton->setText(QApplication::translate("dialogCadOpen", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogCadOpen: public Ui_dialogCadOpen {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGCADOPEN_H
