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
    QLabel *label;
    QLabel *label_2;
    QDoubleSpinBox *A_doubleSpinBox;
    QLabel *label_5;
    QDoubleSpinBox *B_doubleSpinBox;
    QLabel *label_6;
    QDoubleSpinBox *J_doubleSpinBox;
    QLabel *label_7;
    QDoubleSpinBox *tau_g_doubleSpinBox;
    QLabel *label_9;
    QDoubleSpinBox *W_tau_doubleSpinBox;
    QDoubleSpinBox *W_theta_doubleSpinBox;
    QLabel *label_10;
    QDoubleSpinBox *Thor_doubleSpinBox;
    QLabel *label_11;
    QCustomPlot *plot_2;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *GUIClass)
    {
        if (GUIClass->objectName().isEmpty())
            GUIClass->setObjectName(QString::fromUtf8("GUIClass"));
        GUIClass->resize(601, 540);
        centralWidget = new QWidget(GUIClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        plot = new QCustomPlot(centralWidget);
        plot->setObjectName(QString::fromUtf8("plot"));
        plot->setGeometry(QRect(110, 20, 471, 221));
        btn_start = new QPushButton(centralWidget);
        btn_start->setObjectName(QString::fromUtf8("btn_start"));
        btn_start->setGeometry(QRect(20, 400, 75, 23));
        btn_stop = new QPushButton(centralWidget);
        btn_stop->setObjectName(QString::fromUtf8("btn_stop"));
        btn_stop->setGeometry(QRect(20, 430, 75, 23));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(60, 460, 41, 20));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 460, 41, 21));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 61, 16));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 40, 61, 16));
        A_doubleSpinBox = new QDoubleSpinBox(centralWidget);
        A_doubleSpinBox->setObjectName(QString::fromUtf8("A_doubleSpinBox"));
        A_doubleSpinBox->setGeometry(QRect(20, 60, 71, 22));
        A_doubleSpinBox->setSingleStep(0.100000000000000);
        A_doubleSpinBox->setValue(1.500000000000000);
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(20, 90, 61, 16));
        B_doubleSpinBox = new QDoubleSpinBox(centralWidget);
        B_doubleSpinBox->setObjectName(QString::fromUtf8("B_doubleSpinBox"));
        B_doubleSpinBox->setGeometry(QRect(20, 110, 71, 22));
        B_doubleSpinBox->setValue(0.800000000000000);
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(20, 140, 61, 16));
        J_doubleSpinBox = new QDoubleSpinBox(centralWidget);
        J_doubleSpinBox->setObjectName(QString::fromUtf8("J_doubleSpinBox"));
        J_doubleSpinBox->setGeometry(QRect(20, 160, 71, 22));
        J_doubleSpinBox->setValue(1.000000000000000);
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(20, 190, 61, 16));
        tau_g_doubleSpinBox = new QDoubleSpinBox(centralWidget);
        tau_g_doubleSpinBox->setObjectName(QString::fromUtf8("tau_g_doubleSpinBox"));
        tau_g_doubleSpinBox->setGeometry(QRect(20, 210, 71, 22));
        label_9 = new QLabel(centralWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(20, 240, 61, 16));
        W_tau_doubleSpinBox = new QDoubleSpinBox(centralWidget);
        W_tau_doubleSpinBox->setObjectName(QString::fromUtf8("W_tau_doubleSpinBox"));
        W_tau_doubleSpinBox->setGeometry(QRect(20, 310, 71, 22));
        W_tau_doubleSpinBox->setValue(1.000000000000000);
        W_theta_doubleSpinBox = new QDoubleSpinBox(centralWidget);
        W_theta_doubleSpinBox->setObjectName(QString::fromUtf8("W_theta_doubleSpinBox"));
        W_theta_doubleSpinBox->setGeometry(QRect(20, 260, 71, 22));
        W_theta_doubleSpinBox->setMaximum(100000.000000000000000);
        W_theta_doubleSpinBox->setValue(1000.000000000000000);
        label_10 = new QLabel(centralWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(20, 290, 61, 16));
        Thor_doubleSpinBox = new QDoubleSpinBox(centralWidget);
        Thor_doubleSpinBox->setObjectName(QString::fromUtf8("Thor_doubleSpinBox"));
        Thor_doubleSpinBox->setGeometry(QRect(20, 360, 71, 22));
        Thor_doubleSpinBox->setValue(0.200000000000000);
        label_11 = new QLabel(centralWidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(20, 340, 61, 16));
        plot_2 = new QCustomPlot(centralWidget);
        plot_2->setObjectName(QString::fromUtf8("plot_2"));
        plot_2->setGeometry(QRect(110, 260, 471, 221));
        GUIClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(GUIClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 601, 21));
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
        QObject::connect(A_doubleSpinBox, SIGNAL(valueChanged(double)), GUIClass, SLOT(on_A_changed()));

        QMetaObject::connectSlotsByName(GUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *GUIClass)
    {
        GUIClass->setWindowTitle(QApplication::translate("GUIClass", "GUI", nullptr));
        btn_start->setText(QApplication::translate("GUIClass", "Start MPC", nullptr));
        btn_stop->setText(QApplication::translate("GUIClass", "Stop MPC", nullptr));
        label_3->setText(QApplication::translate("GUIClass", "0.000", nullptr));
        label_4->setText(QApplication::translate("GUIClass", "Time", nullptr));
        label->setText(QApplication::translate("GUIClass", "Params", nullptr));
        label_2->setText(QApplication::translate("GUIClass", "A", nullptr));
        label_5->setText(QApplication::translate("GUIClass", "B", nullptr));
        label_6->setText(QApplication::translate("GUIClass", "J", nullptr));
        label_7->setText(QApplication::translate("GUIClass", "tau_g", nullptr));
        label_9->setText(QApplication::translate("GUIClass", "W_theta", nullptr));
        label_10->setText(QApplication::translate("GUIClass", "W_tau", nullptr));
        label_11->setText(QApplication::translate("GUIClass", "Thor", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GUIClass: public Ui_GUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUI_H
