/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet
  Created on: 2015-02-03

  (C) Copyright 2015 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


// system include
#include <iostream>

// Qt include
#include <QString>
#include <QtGui>

// cisst
#include <cisstMultiTask/mtsInterfaceRequired.h>

#include <sawNovintFalcon/mtsNovintHDLQtWidget.h>


CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsNovintHDLQtWidget, mtsComponent, std::string);

mtsNovintHDLQtWidget::mtsNovintHDLQtWidget(const std::string & componentName, double periodInSeconds):
    mtsComponent(componentName),
    TimerPeriodInMilliseconds(periodInSeconds)
{
    DirectControl = false;

    // Setup CISST Interface
    mtsInterfaceRequired * interfaceRequired = AddInterfaceRequired("Falcon");
    if (interfaceRequired) {
        interfaceRequired->AddFunction("GetPositionCartesian", Arm.GetPositionCartesian);
        interfaceRequired->AddFunction("GetPeriodStatistics", Arm.GetPeriodStatistics);
    }
    setupUi();
    startTimer(TimerPeriodInMilliseconds); // ms
}

void mtsNovintHDLQtWidget::Configure(const std::string &filename)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: " << filename << std::endl;
}

void mtsNovintHDLQtWidget::Startup(void)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "mtsNovintHDLQtWidget::Startup" << std::endl;
    if (!parent()) {
        show();
    }
}

void mtsNovintHDLQtWidget::Cleanup(void)
{
    this->hide();
    CMN_LOG_CLASS_INIT_VERBOSE << "mtsNovintHDLQtWidget::Cleanup" << std::endl;
}

void mtsNovintHDLQtWidget::closeEvent(QCloseEvent * event)
{
    int answer = QMessageBox::warning(this, tr("mtsNovintHDLQtWidget"),
                                      tr("Do you really want to quit this application?"),
                                      QMessageBox::No | QMessageBox::Yes);
    if (answer == QMessageBox::Yes) {
        event->accept();
        QCoreApplication::exit();
    } else {
        event->ignore();
    }
}

void mtsNovintHDLQtWidget::timerEvent(QTimerEvent * CMN_UNUSED(event))
{
    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }

    mtsExecutionResult executionResult;
    executionResult = Arm.GetPositionCartesian(Position);
    if (!executionResult) {
        CMN_LOG_CLASS_RUN_ERROR << "Manipulator.GetPositionCartesian failed, \""
                                << executionResult << "\"" << std::endl;
    }
    QFRPositionWidget->SetValue(Position.Position());

    Arm.GetPeriodStatistics(IntervalStatistics);
    QMIntervalStatistics->SetValue(IntervalStatistics);
}

void mtsNovintHDLQtWidget::setupUi(void)
{
    QVBoxLayout * mainLayout = new QVBoxLayout;

    QHBoxLayout * topLayout = new QHBoxLayout;
    mainLayout->addLayout(topLayout);

    // 3D position
    QFRPositionWidget = new vctQtWidgetFrameDoubleRead(vctQtWidgetRotationDoubleRead::OPENGL_WIDGET);
    topLayout->addWidget(QFRPositionWidget, 0, 0);

    // timing
    QVBoxLayout * timingLayout = new QVBoxLayout();
    QMIntervalStatistics = new mtsQtWidgetIntervalStatistics();
    timingLayout->addWidget(QMIntervalStatistics);
    timingLayout->addStretch();
    topLayout->addLayout(timingLayout);

    setLayout(mainLayout);
    setWindowTitle("Novint Falcon");
    resize(sizeHint());
}
