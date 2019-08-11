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
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *GUIClass)
    {
        if (GUIClass->objectName().isEmpty())
            GUIClass->setObjectName(QString::fromUtf8("GUIClass"));
        GUIClass->resize(946, 879);
        GUIClass->setFocusPolicy(Qt::NoFocus);
        centralWidget = new QWidget(GUIClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        plot = new QCustomPlot(centralWidget);
        plot->setObjectName(QString::fromUtf8("plot"));
        plot->setGeometry(QRect(380, 20, 391, 161));
        btn_start = new QPushButton(centralWidget);
        btn_start->setObjectName(QString::fromUtf8("btn_start"));
        btn_start->setGeometry(QRect(20, 410, 75, 23));
        btn_stop = new QPushButton(centralWidget);
        btn_stop->setObjectName(QString::fromUtf8("btn_stop"));
        btn_stop->setGeometry(QRect(20, 440, 75, 23));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(60, 470, 41, 20));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 470, 41, 21));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 50, 61, 16));
        A_box = new QDoubleSpinBox(centralWidget);
        A_box->setObjectName(QString::fromUtf8("A_box"));
        A_box->setGeometry(QRect(20, 70, 71, 22));
        A_box->setDecimals(4);
        A_box->setSingleStep(0.100000000000000);
        A_box->setValue(0.000000000000000);
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(20, 100, 61, 16));
        B_box = new QDoubleSpinBox(centralWidget);
        B_box->setObjectName(QString::fromUtf8("B_box"));
        B_box->setGeometry(QRect(20, 120, 71, 22));
        B_box->setDecimals(4);
        B_box->setValue(0.000000000000000);
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(20, 150, 61, 16));
        J_box = new QDoubleSpinBox(centralWidget);
        J_box->setObjectName(QString::fromUtf8("J_box"));
        J_box->setGeometry(QRect(20, 170, 71, 22));
        J_box->setDecimals(4);
        J_box->setValue(0.000000000000000);
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(20, 200, 61, 16));
        tau_g_box = new QDoubleSpinBox(centralWidget);
        tau_g_box->setObjectName(QString::fromUtf8("tau_g_box"));
        tau_g_box->setGeometry(QRect(20, 220, 71, 22));
        tau_g_box->setDecimals(4);
        label_9 = new QLabel(centralWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(20, 250, 61, 16));
        W_tau_box = new QDoubleSpinBox(centralWidget);
        W_tau_box->setObjectName(QString::fromUtf8("W_tau_box"));
        W_tau_box->setGeometry(QRect(20, 320, 71, 22));
        W_tau_box->setDecimals(1);
        W_tau_box->setValue(0.000000000000000);
        W_theta_box = new QDoubleSpinBox(centralWidget);
        W_theta_box->setObjectName(QString::fromUtf8("W_theta_box"));
        W_theta_box->setGeometry(QRect(20, 270, 71, 22));
        W_theta_box->setDecimals(1);
        W_theta_box->setMaximum(100000.000000000000000);
        W_theta_box->setValue(0.000000000000000);
        label_10 = new QLabel(centralWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(20, 300, 61, 16));
        Thor_box = new QDoubleSpinBox(centralWidget);
        Thor_box->setObjectName(QString::fromUtf8("Thor_box"));
        Thor_box->setGeometry(QRect(20, 370, 71, 22));
        Thor_box->setDecimals(3);
        Thor_box->setValue(0.000000000000000);
        label_11 = new QLabel(centralWidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(20, 350, 61, 16));
        btn_set_params = new QPushButton(centralWidget);
        btn_set_params->setObjectName(QString::fromUtf8("btn_set_params"));
        btn_set_params->setGeometry(QRect(20, 20, 75, 23));
        plot2 = new QCustomPlot(centralWidget);
        plot2->setObjectName(QString::fromUtf8("plot2"));
        plot2->setEnabled(false);
        plot2->setGeometry(QRect(380, 380, 391, 161));
        plot3 = new QCustomPlot(centralWidget);
        plot3->setObjectName(QString::fromUtf8("plot3"));
        plot3->setEnabled(false);
        plot3->setGeometry(QRect(380, 560, 391, 161));
        plot1 = new QCustomPlot(centralWidget);
        plot1->setObjectName(QString::fromUtf8("plot1"));
        plot1->setEnabled(false);
        plot1->setGeometry(QRect(380, 200, 391, 161));
        GUIClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(GUIClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 946, 21));
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
        label_3->setText(QApplication::translate("GUIClass", "0.000", nullptr));
        label_4->setText(QApplication::translate("GUIClass", "Time", nullptr));
        label_2->setText(QApplication::translate("GUIClass", "A", nullptr));
        label_5->setText(QApplication::translate("GUIClass", "B", nullptr));
        label_6->setText(QApplication::translate("GUIClass", "J", nullptr));
        label_7->setText(QApplication::translate("GUIClass", "tau_g", nullptr));
        label_9->setText(QApplication::translate("GUIClass", "W_theta", nullptr));
        label_10->setText(QApplication::translate("GUIClass", "W_tau", nullptr));
        label_11->setText(QApplication::translate("GUIClass", "Thor", nullptr));
        btn_set_params->setText(QApplication::translate("GUIClass", "Set Params", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GUIClass: public Ui_GUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUI_H
