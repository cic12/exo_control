/********************************************************************************
** Form generated from reading UI file 'GUI.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GUI_H
#define UI_GUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_GUIClass
{
public:
    QWidget *centralWidget;
    QCustomPlot *plot;
    QPushButton *btn_start;
    QPushButton *btn_stop;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_2;
    QDoubleSpinBox *A_box;
    QLabel *label_5;
    QDoubleSpinBox *B_box;
    QLabel *label_6;
    QDoubleSpinBox *J_box;
    QLabel *label_7;
    QDoubleSpinBox *tau_g_box;
    QLabel *label_9;
    QDoubleSpinBox *W_tau_box;
    QDoubleSpinBox *W_theta_box;
    QLabel *label_10;
    QDoubleSpinBox *Thor_box;
    QLabel *label_11;
    QPushButton *btn_set_params;
    QCustomPlot *plot2;
    QCustomPlot *plot3;
    QCustomPlot *plot1;
    QCustomPlot *plot4;
    QCustomPlot *plot5;
    QLabel *label_12;
    QDoubleSpinBox *sig_h_box;
    QLabel *label_8;
    QLabel *label_13;
    QLabel *label_14;
    QDoubleSpinBox *pR_box;
    QLabel *label_15;
    QDoubleSpinBox *b1_box;
    QDoubleSpinBox *pA_box;
    QDoubleSpinBox *b2_box;
    QLabel *label_16;
    QDoubleSpinBox *c_h_box;
    QLabel *label_17;
    QLabel *label_18;
    QDoubleSpinBox *sig_e_box;
    QLabel *label_19;
    QDoubleSpinBox *c_e_box;
    QLabel *label_20;
    QDoubleSpinBox *halt_lim_box;
    QDoubleSpinBox *b3_box;
    QLabel *label_21;
    QLabel *label_22;
    QLabel *label_23;
    QLabel *label_24;
    QLabel *label_25;
    QLabel *label_26;
    QPlainTextEdit *plainTextEdit;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *GUIClass)
    {
        if (GUIClass->objectName().isEmpty())
            GUIClass->setObjectName(QString::fromUtf8("GUIClass"));
        GUIClass->resize(712, 876);
        GUIClass->setFocusPolicy(Qt::NoFocus);
        centralWidget = new QWidget(GUIClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        plot = new QCustomPlot(centralWidget);
        plot->setObjectName(QString::fromUtf8("plot"));
        plot->setGeometry(QRect(200, 30, 501, 121));
        btn_start = new QPushButton(centralWidget);
        btn_start->setObjectName(QString::fromUtf8("btn_start"));
        btn_start->setGeometry(QRect(10, 800, 75, 23));
        btn_stop = new QPushButton(centralWidget);
        btn_stop->setObjectName(QString::fromUtf8("btn_stop"));
        btn_stop->setGeometry(QRect(110, 800, 75, 23));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(50, 770, 41, 20));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 770, 41, 21));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 10, 61, 16));
        A_box = new QDoubleSpinBox(centralWidget);
        A_box->setObjectName(QString::fromUtf8("A_box"));
        A_box->setGeometry(QRect(10, 30, 71, 22));
        A_box->setDecimals(4);
        A_box->setSingleStep(0.100000000000000);
        A_box->setValue(0.000000000000000);
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 60, 61, 16));
        B_box = new QDoubleSpinBox(centralWidget);
        B_box->setObjectName(QString::fromUtf8("B_box"));
        B_box->setGeometry(QRect(10, 80, 71, 22));
        B_box->setDecimals(4);
        B_box->setSingleStep(0.100000000000000);
        B_box->setValue(0.000000000000000);
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 110, 61, 16));
        J_box = new QDoubleSpinBox(centralWidget);
        J_box->setObjectName(QString::fromUtf8("J_box"));
        J_box->setGeometry(QRect(10, 130, 71, 22));
        J_box->setDecimals(4);
        J_box->setSingleStep(0.100000000000000);
        J_box->setValue(0.000000000000000);
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 160, 61, 16));
        tau_g_box = new QDoubleSpinBox(centralWidget);
        tau_g_box->setObjectName(QString::fromUtf8("tau_g_box"));
        tau_g_box->setGeometry(QRect(10, 180, 71, 22));
        tau_g_box->setDecimals(4);
        tau_g_box->setSingleStep(0.100000000000000);
        label_9 = new QLabel(centralWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(10, 210, 61, 16));
        W_tau_box = new QDoubleSpinBox(centralWidget);
        W_tau_box->setObjectName(QString::fromUtf8("W_tau_box"));
        W_tau_box->setGeometry(QRect(10, 280, 71, 22));
        W_tau_box->setDecimals(1);
        W_tau_box->setValue(0.000000000000000);
        W_theta_box = new QDoubleSpinBox(centralWidget);
        W_theta_box->setObjectName(QString::fromUtf8("W_theta_box"));
        W_theta_box->setGeometry(QRect(10, 230, 71, 22));
        W_theta_box->setDecimals(1);
        W_theta_box->setMaximum(100000.000000000000000);
        W_theta_box->setSingleStep(10000.000000000000000);
        W_theta_box->setValue(0.000000000000000);
        label_10 = new QLabel(centralWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(10, 260, 61, 16));
        Thor_box = new QDoubleSpinBox(centralWidget);
        Thor_box->setObjectName(QString::fromUtf8("Thor_box"));
        Thor_box->setGeometry(QRect(10, 330, 71, 22));
        Thor_box->setDecimals(3);
        Thor_box->setSingleStep(0.001000000000000);
        Thor_box->setValue(0.000000000000000);
        label_11 = new QLabel(centralWidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(10, 310, 61, 16));
        btn_set_params = new QPushButton(centralWidget);
        btn_set_params->setObjectName(QString::fromUtf8("btn_set_params"));
        btn_set_params->setGeometry(QRect(10, 380, 75, 23));
        plot2 = new QCustomPlot(centralWidget);
        plot2->setObjectName(QString::fromUtf8("plot2"));
        plot2->setEnabled(false);
        plot2->setGeometry(QRect(200, 330, 501, 101));
        plot3 = new QCustomPlot(centralWidget);
        plot3->setObjectName(QString::fromUtf8("plot3"));
        plot3->setEnabled(false);
        plot3->setGeometry(QRect(200, 460, 501, 101));
        plot1 = new QCustomPlot(centralWidget);
        plot1->setObjectName(QString::fromUtf8("plot1"));
        plot1->setEnabled(false);
        plot1->setGeometry(QRect(200, 180, 501, 121));
        plot4 = new QCustomPlot(centralWidget);
        plot4->setObjectName(QString::fromUtf8("plot4"));
        plot4->setEnabled(false);
        plot4->setGeometry(QRect(200, 590, 501, 101));
        plot5 = new QCustomPlot(centralWidget);
        plot5->setObjectName(QString::fromUtf8("plot5"));
        plot5->setEnabled(false);
        plot5->setGeometry(QRect(200, 720, 501, 101));
        label_12 = new QLabel(centralWidget);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(110, 260, 61, 16));
        sig_h_box = new QDoubleSpinBox(centralWidget);
        sig_h_box->setObjectName(QString::fromUtf8("sig_h_box"));
        sig_h_box->setGeometry(QRect(110, 280, 71, 22));
        sig_h_box->setDecimals(2);
        sig_h_box->setMinimum(-100.000000000000000);
        sig_h_box->setMaximum(100.000000000000000);
        sig_h_box->setSingleStep(0.010000000000000);
        sig_h_box->setValue(0.000000000000000);
        label_8 = new QLabel(centralWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(110, 10, 61, 16));
        label_13 = new QLabel(centralWidget);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(110, 210, 61, 16));
        label_14 = new QLabel(centralWidget);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(110, 310, 61, 16));
        pR_box = new QDoubleSpinBox(centralWidget);
        pR_box->setObjectName(QString::fromUtf8("pR_box"));
        pR_box->setGeometry(QRect(110, 230, 71, 22));
        pR_box->setDecimals(2);
        pR_box->setMaximum(1.000000000000000);
        pR_box->setSingleStep(0.010000000000000);
        pR_box->setValue(0.000000000000000);
        label_15 = new QLabel(centralWidget);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(110, 60, 61, 16));
        b1_box = new QDoubleSpinBox(centralWidget);
        b1_box->setObjectName(QString::fromUtf8("b1_box"));
        b1_box->setGeometry(QRect(110, 30, 71, 22));
        b1_box->setDecimals(2);
        b1_box->setMinimum(-5000.000000000000000);
        b1_box->setMaximum(5000.000000000000000);
        b1_box->setSingleStep(100.000000000000000);
        b1_box->setValue(0.000000000000000);
        pA_box = new QDoubleSpinBox(centralWidget);
        pA_box->setObjectName(QString::fromUtf8("pA_box"));
        pA_box->setGeometry(QRect(110, 180, 71, 22));
        pA_box->setDecimals(2);
        pA_box->setMinimum(0.000000000000000);
        pA_box->setMaximum(1.000000000000000);
        pA_box->setSingleStep(0.010000000000000);
        b2_box = new QDoubleSpinBox(centralWidget);
        b2_box->setObjectName(QString::fromUtf8("b2_box"));
        b2_box->setGeometry(QRect(110, 80, 71, 22));
        b2_box->setDecimals(2);
        b2_box->setMinimum(-5000.000000000000000);
        b2_box->setMaximum(5000.000000000000000);
        b2_box->setSingleStep(100.000000000000000);
        b2_box->setValue(0.000000000000000);
        label_16 = new QLabel(centralWidget);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setGeometry(QRect(110, 160, 61, 16));
        c_h_box = new QDoubleSpinBox(centralWidget);
        c_h_box->setObjectName(QString::fromUtf8("c_h_box"));
        c_h_box->setGeometry(QRect(110, 330, 71, 22));
        c_h_box->setDecimals(2);
        c_h_box->setMinimum(-100.000000000000000);
        c_h_box->setMaximum(100.000000000000000);
        c_h_box->setSingleStep(0.010000000000000);
        c_h_box->setValue(0.000000000000000);
        label_17 = new QLabel(centralWidget);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setGeometry(QRect(110, 110, 61, 16));
        label_18 = new QLabel(centralWidget);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setGeometry(QRect(110, 360, 61, 16));
        sig_e_box = new QDoubleSpinBox(centralWidget);
        sig_e_box->setObjectName(QString::fromUtf8("sig_e_box"));
        sig_e_box->setGeometry(QRect(110, 380, 71, 22));
        sig_e_box->setDecimals(2);
        sig_e_box->setMinimum(-100.000000000000000);
        sig_e_box->setMaximum(100.000000000000000);
        sig_e_box->setSingleStep(0.010000000000000);
        sig_e_box->setValue(0.000000000000000);
        label_19 = new QLabel(centralWidget);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setGeometry(QRect(110, 410, 61, 16));
        c_e_box = new QDoubleSpinBox(centralWidget);
        c_e_box->setObjectName(QString::fromUtf8("c_e_box"));
        c_e_box->setGeometry(QRect(110, 430, 71, 22));
        c_e_box->setDecimals(2);
        c_e_box->setMinimum(-100.000000000000000);
        c_e_box->setMaximum(100.000000000000000);
        c_e_box->setSingleStep(0.010000000000000);
        c_e_box->setValue(0.000000000000000);
        label_20 = new QLabel(centralWidget);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setGeometry(QRect(110, 460, 61, 16));
        halt_lim_box = new QDoubleSpinBox(centralWidget);
        halt_lim_box->setObjectName(QString::fromUtf8("halt_lim_box"));
        halt_lim_box->setGeometry(QRect(110, 480, 71, 22));
        halt_lim_box->setDecimals(2);
        halt_lim_box->setMinimum(0.000000000000000);
        halt_lim_box->setMaximum(1.000000000000000);
        halt_lim_box->setSingleStep(0.010000000000000);
        halt_lim_box->setValue(0.000000000000000);
        b3_box = new QDoubleSpinBox(centralWidget);
        b3_box->setObjectName(QString::fromUtf8("b3_box"));
        b3_box->setGeometry(QRect(110, 130, 71, 22));
        b3_box->setDecimals(2);
        b3_box->setMinimum(-5000.000000000000000);
        b3_box->setMaximum(5000.000000000000000);
        b3_box->setSingleStep(100.000000000000000);
        b3_box->setValue(0.000000000000000);
        label_21 = new QLabel(centralWidget);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setGeometry(QRect(210, 10, 61, 16));
        label_22 = new QLabel(centralWidget);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        label_22->setGeometry(QRect(210, 160, 61, 16));
        label_23 = new QLabel(centralWidget);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setGeometry(QRect(210, 310, 61, 16));
        label_24 = new QLabel(centralWidget);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        label_24->setGeometry(QRect(210, 440, 61, 16));
        label_25 = new QLabel(centralWidget);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setGeometry(QRect(210, 570, 61, 16));
        label_26 = new QLabel(centralWidget);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        label_26->setGeometry(QRect(210, 700, 61, 16));
        plainTextEdit = new QPlainTextEdit(centralWidget);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setGeometry(QRect(10, 520, 171, 241));
        plainTextEdit->setReadOnly(true);
        GUIClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(GUIClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 712, 21));
        GUIClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(GUIClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        GUIClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(GUIClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        GUIClass->setStatusBar(statusBar);

        retranslateUi(GUIClass);
        QObject::connect(btn_start, SIGNAL(clicked()), GUIClass, SLOT(on_btn_start_clicked()));
        QObject::connect(btn_stop, SIGNAL(clicked()), GUIClass, SLOT(on_btn_stop_clicked()));
        QObject::connect(btn_set_params, SIGNAL(clicked()), GUIClass, SLOT(on_btn_set_params_clicked()));

        QMetaObject::connectSlotsByName(GUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *GUIClass)
    {
        GUIClass->setWindowTitle(QApplication::translate("GUIClass", "GUI", nullptr));
        btn_start->setText(QApplication::translate("GUIClass", "Start MPC", nullptr));
        btn_stop->setText(QApplication::translate("GUIClass", "Stop MPC", nullptr));
        label_3->setText(QApplication::translate("GUIClass", "0.00", nullptr));
        label_4->setText(QApplication::translate("GUIClass", "Time", nullptr));
        label_2->setText(QApplication::translate("GUIClass", "A", nullptr));
        label_5->setText(QApplication::translate("GUIClass", "B", nullptr));
        label_6->setText(QApplication::translate("GUIClass", "J", nullptr));
        label_7->setText(QApplication::translate("GUIClass", "tau_g", nullptr));
        label_9->setText(QApplication::translate("GUIClass", "W_theta", nullptr));
        label_10->setText(QApplication::translate("GUIClass", "W_tau", nullptr));
        label_11->setText(QApplication::translate("GUIClass", "Thor", nullptr));
        btn_set_params->setText(QApplication::translate("GUIClass", "Set Params", nullptr));
        label_12->setText(QApplication::translate("GUIClass", "sig_h", nullptr));
        label_8->setText(QApplication::translate("GUIClass", "b1", nullptr));
        label_13->setText(QApplication::translate("GUIClass", "pR", nullptr));
        label_14->setText(QApplication::translate("GUIClass", "c_h", nullptr));
        label_15->setText(QApplication::translate("GUIClass", "b2", nullptr));
        label_16->setText(QApplication::translate("GUIClass", "pA", nullptr));
        label_17->setText(QApplication::translate("GUIClass", "b3", nullptr));
        label_18->setText(QApplication::translate("GUIClass", "sig_e", nullptr));
        label_19->setText(QApplication::translate("GUIClass", "c_e", nullptr));
        label_20->setText(QApplication::translate("GUIClass", "halt_lim", nullptr));
        label_21->setText(QApplication::translate("GUIClass", "Outputs", nullptr));
        label_22->setText(QApplication::translate("GUIClass", "Inputs", nullptr));
        label_23->setText(QApplication::translate("GUIClass", "EMG1", nullptr));
        label_24->setText(QApplication::translate("GUIClass", "EMG2", nullptr));
        label_25->setText(QApplication::translate("GUIClass", "lambdaA", nullptr));
        label_26->setText(QApplication::translate("GUIClass", "lambdaA", nullptr));
        plainTextEdit->setPlainText(QString());
    } // retranslateUi

};

namespace Ui {
    class GUIClass: public Ui_GUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUI_H
