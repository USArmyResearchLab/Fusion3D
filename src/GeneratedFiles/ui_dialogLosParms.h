/********************************************************************************
** Form generated from reading UI file 'dialogLosParms.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGLOSPARMS_H
#define UI_DIALOGLOSPARMS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_dialogLosParms
{
public:
    QLabel *label;
    QLabel *label_2;
    QFrame *line;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QSlider *minRSlider;
    QSlider *hCenSlider;
    QSlider *hPerSlider;
    QSlider *minAngSlider;
    QSlider *maxAngSlider;
    QLineEdit *minREdit;
    QLineEdit *hCenEdit;
    QLineEdit *hPerEdit;
    QLineEdit *minAngEdit;
    QLineEdit *maxAngEdit;
    QFrame *line_2;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *label_12;
    QLineEdit *sensorLatEdit;
    QLineEdit *sensorLonEdit;
    QLineEdit *sensorElevEdit;
    QFrame *line_3;
    QLabel *label_13;
    QLabel *label_14;
    QLabel *label_15;
    QLineEdit *dateEdit;
    QLineEdit *timeEdit;
    QFrame *line_4;
    QLabel *label_16;
    QCheckBox *visBox;
    QCheckBox *shadowBox;
    QCheckBox *checkBox;
    QFrame *line_5;
    QPushButton *applyButton;
    QPushButton *CancelButton;
    QSlider *sizeSlider;
    QLineEdit *sizeEdit;
    QPushButton *recalcButton;

    void setupUi(QDialog *dialogLosParms)
    {
        if (dialogLosParms->objectName().isEmpty())
            dialogLosParms->setObjectName(QString::fromUtf8("dialogLosParms"));
        dialogLosParms->resize(400, 609);
        label = new QLabel(dialogLosParms);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 101, 16));
        label_2 = new QLabel(dialogLosParms);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 30, 151, 16));
        line = new QFrame(dialogLosParms);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(20, 60, 351, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label_3 = new QLabel(dialogLosParms);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 80, 111, 16));
        label_4 = new QLabel(dialogLosParms);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(110, 100, 111, 16));
        label_5 = new QLabel(dialogLosParms);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(60, 130, 111, 16));
        label_6 = new QLabel(dialogLosParms);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(50, 160, 141, 16));
        label_7 = new QLabel(dialogLosParms);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(40, 190, 131, 16));
        label_8 = new QLabel(dialogLosParms);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(40, 220, 131, 16));
        minRSlider = new QSlider(dialogLosParms);
        minRSlider->setObjectName(QString::fromUtf8("minRSlider"));
        minRSlider->setGeometry(QRect(170, 100, 160, 19));
        minRSlider->setOrientation(Qt::Horizontal);
        hCenSlider = new QSlider(dialogLosParms);
        hCenSlider->setObjectName(QString::fromUtf8("hCenSlider"));
        hCenSlider->setGeometry(QRect(170, 130, 160, 19));
        hCenSlider->setOrientation(Qt::Horizontal);
        hPerSlider = new QSlider(dialogLosParms);
        hPerSlider->setObjectName(QString::fromUtf8("hPerSlider"));
        hPerSlider->setGeometry(QRect(170, 160, 160, 19));
        hPerSlider->setOrientation(Qt::Horizontal);
        minAngSlider = new QSlider(dialogLosParms);
        minAngSlider->setObjectName(QString::fromUtf8("minAngSlider"));
        minAngSlider->setGeometry(QRect(170, 190, 160, 19));
        minAngSlider->setOrientation(Qt::Horizontal);
        maxAngSlider = new QSlider(dialogLosParms);
        maxAngSlider->setObjectName(QString::fromUtf8("maxAngSlider"));
        maxAngSlider->setGeometry(QRect(170, 220, 160, 19));
        maxAngSlider->setOrientation(Qt::Horizontal);
        minREdit = new QLineEdit(dialogLosParms);
        minREdit->setObjectName(QString::fromUtf8("minREdit"));
        minREdit->setGeometry(QRect(340, 100, 41, 20));
        hCenEdit = new QLineEdit(dialogLosParms);
        hCenEdit->setObjectName(QString::fromUtf8("hCenEdit"));
        hCenEdit->setGeometry(QRect(342, 130, 41, 20));
        hPerEdit = new QLineEdit(dialogLosParms);
        hPerEdit->setObjectName(QString::fromUtf8("hPerEdit"));
        hPerEdit->setGeometry(QRect(342, 160, 41, 20));
        minAngEdit = new QLineEdit(dialogLosParms);
        minAngEdit->setObjectName(QString::fromUtf8("minAngEdit"));
        minAngEdit->setGeometry(QRect(342, 190, 41, 20));
        maxAngEdit = new QLineEdit(dialogLosParms);
        maxAngEdit->setObjectName(QString::fromUtf8("maxAngEdit"));
        maxAngEdit->setGeometry(QRect(342, 220, 41, 20));
        line_2 = new QFrame(dialogLosParms);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(10, 250, 371, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        label_9 = new QLabel(dialogLosParms);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(10, 270, 221, 16));
        label_10 = new QLabel(dialogLosParms);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(100, 290, 111, 16));
        label_11 = new QLabel(dialogLosParms);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(100, 320, 121, 16));
        label_12 = new QLabel(dialogLosParms);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(100, 350, 111, 16));
        sensorLatEdit = new QLineEdit(dialogLosParms);
        sensorLatEdit->setObjectName(QString::fromUtf8("sensorLatEdit"));
        sensorLatEdit->setGeometry(QRect(240, 290, 141, 20));
        sensorLonEdit = new QLineEdit(dialogLosParms);
        sensorLonEdit->setObjectName(QString::fromUtf8("sensorLonEdit"));
        sensorLonEdit->setGeometry(QRect(240, 320, 141, 20));
        sensorElevEdit = new QLineEdit(dialogLosParms);
        sensorElevEdit->setObjectName(QString::fromUtf8("sensorElevEdit"));
        sensorElevEdit->setGeometry(QRect(240, 350, 141, 20));
        line_3 = new QFrame(dialogLosParms);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(10, 380, 361, 16));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        label_13 = new QLabel(dialogLosParms);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(10, 390, 111, 16));
        label_14 = new QLabel(dialogLosParms);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(170, 400, 111, 16));
        label_15 = new QLabel(dialogLosParms);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(180, 430, 91, 16));
        dateEdit = new QLineEdit(dialogLosParms);
        dateEdit->setObjectName(QString::fromUtf8("dateEdit"));
        dateEdit->setGeometry(QRect(270, 400, 113, 20));
        timeEdit = new QLineEdit(dialogLosParms);
        timeEdit->setObjectName(QString::fromUtf8("timeEdit"));
        timeEdit->setGeometry(QRect(270, 430, 113, 20));
        line_4 = new QFrame(dialogLosParms);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setGeometry(QRect(20, 450, 341, 16));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        label_16 = new QLabel(dialogLosParms);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setGeometry(QRect(20, 470, 211, 16));
        visBox = new QCheckBox(dialogLosParms);
        visBox->setObjectName(QString::fromUtf8("visBox"));
        visBox->setGeometry(QRect(40, 490, 131, 17));
        shadowBox = new QCheckBox(dialogLosParms);
        shadowBox->setObjectName(QString::fromUtf8("shadowBox"));
        shadowBox->setGeometry(QRect(180, 490, 151, 17));
        checkBox = new QCheckBox(dialogLosParms);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(40, 510, 241, 17));
        line_5 = new QFrame(dialogLosParms);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setGeometry(QRect(10, 530, 371, 16));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);
        applyButton = new QPushButton(dialogLosParms);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setGeometry(QRect(60, 550, 231, 23));
        CancelButton = new QPushButton(dialogLosParms);
        CancelButton->setObjectName(QString::fromUtf8("CancelButton"));
        CancelButton->setGeometry(QRect(310, 580, 75, 23));
        sizeSlider = new QSlider(dialogLosParms);
        sizeSlider->setObjectName(QString::fromUtf8("sizeSlider"));
        sizeSlider->setGeometry(QRect(170, 30, 160, 19));
        sizeSlider->setOrientation(Qt::Horizontal);
        sizeEdit = new QLineEdit(dialogLosParms);
        sizeEdit->setObjectName(QString::fromUtf8("sizeEdit"));
        sizeEdit->setGeometry(QRect(342, 30, 41, 20));
        recalcButton = new QPushButton(dialogLosParms);
        recalcButton->setObjectName(QString::fromUtf8("recalcButton"));
        recalcButton->setGeometry(QRect(60, 580, 231, 23));

        retranslateUi(dialogLosParms);
        QObject::connect(applyButton, SIGNAL(clicked()), dialogLosParms, SLOT(doApply()));
        QObject::connect(CancelButton, SIGNAL(clicked()), dialogLosParms, SLOT(reject()));
        QObject::connect(minRSlider, SIGNAL(sliderReleased()), dialogLosParms, SLOT(doMinRange()));
        QObject::connect(hCenSlider, SIGNAL(sliderReleased()), dialogLosParms, SLOT(doHCen()));
        QObject::connect(hPerSlider, SIGNAL(sliderReleased()), dialogLosParms, SLOT(doHPer()));
        QObject::connect(minAngSlider, SIGNAL(sliderReleased()), dialogLosParms, SLOT(doMinAngle()));
        QObject::connect(maxAngSlider, SIGNAL(sliderReleased()), dialogLosParms, SLOT(doMaxAngle()));
        QObject::connect(sizeSlider, SIGNAL(sliderReleased()), dialogLosParms, SLOT(doSize()));
        QObject::connect(recalcButton, SIGNAL(clicked()), dialogLosParms, SLOT(doRecalc()));

        QMetaObject::connectSlotsByName(dialogLosParms);
    } // setupUi

    void retranslateUi(QDialog *dialogLosParms)
    {
        dialogLosParms->setWindowTitle(QApplication::translate("dialogLosParms", "dialogLosParms", nullptr));
        label->setText(QApplication::translate("dialogLosParms", "All LOS calculations", nullptr));
        label_2->setText(QApplication::translate("dialogLosParms", "Height/width of calc area (m)", nullptr));
        label_3->setText(QApplication::translate("dialogLosParms", "Ground-to-Ground", nullptr));
        label_4->setText(QApplication::translate("dialogLosParms", "Min range", nullptr));
        label_5->setText(QApplication::translate("dialogLosParms", "Height at center loc", nullptr));
        label_6->setText(QApplication::translate("dialogLosParms", "Height at perimeter loc", nullptr));
        label_7->setText(QApplication::translate("dialogLosParms", "Min angle (deg from N cw)", nullptr));
        label_8->setText(QApplication::translate("dialogLosParms", "Max angle (deg from N cw)", nullptr));
        label_9->setText(QApplication::translate("dialogLosParms", "Fixed-position standoff sensor", nullptr));
        label_10->setText(QApplication::translate("dialogLosParms", "Sensor latitude (deg)", nullptr));
        label_11->setText(QApplication::translate("dialogLosParms", "Sensor longitude (deg)", nullptr));
        label_12->setText(QApplication::translate("dialogLosParms", "Sensor elevation (m)", nullptr));
        label_13->setText(QApplication::translate("dialogLosParms", "Sun shadowing", nullptr));
        label_14->setText(QApplication::translate("dialogLosParms", "Date (mm/dd/yyyy)", nullptr));
        label_15->setText(QApplication::translate("dialogLosParms", "Time UTC (GMT)", nullptr));
        label_16->setText(QApplication::translate("dialogLosParms", "Write shadow boundaries to shapefile", nullptr));
        visBox->setText(QApplication::translate("dialogLosParms", "Outline visible regions", nullptr));
        shadowBox->setText(QApplication::translate("dialogLosParms", "Outline shadowed regions", nullptr));
        checkBox->setText(QApplication::translate("dialogLosParms", "Include shadow regions that touch boundary", nullptr));
        applyButton->setText(QApplication::translate("dialogLosParms", "Apply changes only -- LOS Menu for Recalcs", nullptr));
        CancelButton->setText(QApplication::translate("dialogLosParms", "Cancel", nullptr));
        recalcButton->setText(QApplication::translate("dialogLosParms", "Recalc LOS Ground-to-Ground with Changes", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialogLosParms: public Ui_dialogLosParms {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGLOSPARMS_H
