// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlsensorgesture_p.h"
#include <QtSensors/qsensorgesture.h>
#include <QtSensors/qsensorgesturemanager.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype SensorGesture
    \instantiates QmlSensorGesture
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \brief Provides notifications when sensor-based gestures are detected.

    This type provides notification when sensor gestures are triggered.

    The following QML code creates a "shake" and "SecondCounter" SensorGesture QML type, and
    displays the detected gesture in a text type.

    QtSensors.shake gesture is available with the Qt Sensors API, but the QtSensors.SecondCounter
    sensor gesture is provided as example code for the \l {Qt Sensors - SensorGesture QML Type Example}

    \qml
    Item {
       SensorGesture {
           id: sensorGesture
           enabled: false
           gestures : ["QtSensors.shake", "QtSensors.SecondCounter"]
           onDetected:{
               detectedText.text = gesture
           }
       }
       Text {
           id: detectedText
           x:5
           y:160
       }
    }
    \endqml

    \l {Qt Sensor Gestures} contains a list of currently supported sensor gestures and their
    descriptions.


*/
QmlSensorGesture::QmlSensorGesture(QObject* parent)
    : QObject(parent)
    , isEnabled(false)
    , initDone(false)
    , sensorGesture(0)
    , sensorGestureManager(new QSensorGestureManager(this))
{
    connect(sensorGestureManager, SIGNAL(newSensorGestureAvailable()), SIGNAL(availableGesturesChanged()));
}

QmlSensorGesture::~QmlSensorGesture()
{
}

/*
  QQmlParserStatus interface implementation
*/
void QmlSensorGesture::classBegin()
{
}

void QmlSensorGesture::componentComplete()
{
    /*
      this is needed in the case the customer defines the type(s) and set it enabled = true
    */
    initDone = true;
    setEnabled(isEnabled);
}
/*
  End of QQmlParserStatus interface implementation
*/

/*!
    \qmlproperty stringlist SensorGesture::availableGestures
    This property can be used to determine all available gestures on the system.
*/
QStringList QmlSensorGesture::availableGestures()
{
    return sensorGestureManager->gestureIds();
}

/*!
    \qmlproperty stringlist SensorGesture::gestures
    Set this property to a list of the gestures that the application is interested in detecting.
    This property cannot be changed while the type is enabled.

    The properties validGestures and invalidGestures will be set as appropriate immediately.
    To determine all available getures on the system please use the
    \l {SensorGesture::availableGestures} {availableGestures} property.

    \sa {QtSensorGestures Plugins}
*/
QStringList QmlSensorGesture::gestures() const
{
    return gestureList;
}

void QmlSensorGesture::setGestures(const QStringList& value)
{
    if (gestureList == value)
        return;

    if (initDone && enabled()) {
        qWarning() << "Cannot change gestures while running.";
        return;
    }
    gestureList = value;
    createGesture();
    Q_EMIT gesturesChanged();
}


/*!
    \qmlproperty stringlist SensorGesture::validGestures
    This property holds the requested gestures that were found on the system.
*/
QStringList QmlSensorGesture::validGestures() const
{
    if (sensorGesture)
        return sensorGesture->validIds();
    return QStringList();
}

/*!
    \qmlproperty stringlist SensorGesture::invalidGestures
    This property holds the requested gestures that were not found on the system.
*/
QStringList QmlSensorGesture::invalidGestures() const
{
    if (sensorGesture)
        return sensorGesture->invalidIds();
    return QStringList();
}

/*!
    \qmlproperty bool SensorGesture::enabled
    This property can be used to activate or deactivate the sensor gesture.
    Default value is false;
    \sa {SensorGesture::detected}, {detected}
*/
bool QmlSensorGesture::enabled() const
{
    return isEnabled;
}

void QmlSensorGesture::setEnabled(bool value)
{
    bool hasChanged = false;
    if (isEnabled != value) {
        isEnabled = value;
        hasChanged = true;
    }
    if (!initDone)
        return;

    if (sensorGesture) {
        if (value) {
            sensorGesture->startDetection();
        } else {
            sensorGesture->stopDetection();
        }
    }
    if (hasChanged)
        Q_EMIT enabledChanged();
}

/*!
    \qmlsignal SensorGesture::detected(string gesture)
    This signal is emitted whenever a gesture is detected.
    The \a gesture parameter contains the gesture that was detected.

    The corresponding handler is \c onDetected.
*/

/*
  private function implementation
*/
void QmlSensorGesture::deleteGesture()
{
    if (sensorGesture) {
        bool emitInvalidChange = !invalidGestures().isEmpty();
        bool emitValidChange = !validGestures().isEmpty();

        if (sensorGesture->isActive()) {
            sensorGesture->stopDetection();
        }
        delete sensorGesture;
        sensorGesture = 0;

        if (emitInvalidChange) {
            Q_EMIT invalidGesturesChanged();
        }
        if (emitValidChange) {
            Q_EMIT validGesturesChanged();
        }
    }
}

void QmlSensorGesture::createGesture()
{
    deleteGesture();
    sensorGesture = new QSensorGesture(gestureList, this);
    if (!validGestures().isEmpty()) {
        QObject::connect(sensorGesture
                         , SIGNAL(detected(QString))
                         , this
                         , SIGNAL(detected(QString)));
        Q_EMIT validGesturesChanged();
    }
    if (!invalidGestures().isEmpty())
        Q_EMIT invalidGesturesChanged();
}

/*
  End of private function implementation
*/

QT_END_NAMESPACE
