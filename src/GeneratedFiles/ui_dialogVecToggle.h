/********************************************************************************
** Form generated from reading UI file 'dialogVecToggle.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGVECTOGGLE_H
#define UI_DIALOGVECTOGGLE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_dialogVecToggle
{
public:
    QPushButton *applyButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *dialogVecToggle)
    {
        if (dialogVecToggle->objectName().isEmpty())
            dialogVecToggle->setObjectName(QString::fromUtf8("dialogVecToggle"));
        dialogVecToggle->resize(306, 567);
        applyButton = new QPushButton(dialogVecToggle);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setGeometry(QRect(60, 540, 141, 23));
        cancelButton = new QPushButton(dialogVecToggle);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(210, 540, 75, 23));

        retranslateUi(dialogVecToggle);
        QObject::connect(applyButton, SIGNAL(clicked()), dialogVecToggle, SLOT(doApply()));
        QObject::connect(cancelButton, SIGNAL(clicked()), dialogVecToggle, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogVecToggle);
    } // setupUi

    void retranslateUi(QDialog *dialogVecToggle)
    {
        dialogVecToggle->setWindowTitle(QApplication::translate("dialogVecToggle", "dialogVecToggle", nullptr));
        applyButton->setText(QApplication::translate("dialogVecToggle", "Apply changes", nullptr));
        cancelButton->setText(QApplication::translate("dialogVecToggle", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogVecToggle: public Ui_dialogVecToggle {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGVECTOGGLE_H
